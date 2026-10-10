
#include "emu-fd-net-device-helper.h"

#include "encode-decode.h"

#include "ns3/abort.h"
#include "ns3/config.h"
#include "ns3/fd-net-device.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/object-factory.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/trace-helper.h"

#include <arpa/inet.h>
#include <errno.h>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("EmuFdNetDeviceHelper");

#define EMU_MAGIC 65867

EmuFdNetDeviceHelper::EmuFdNetDeviceHelper() {
  m_deviceName = "undefined";
  m_hostQdiscBypass = false;
}

void EmuFdNetDeviceHelper::SetDeviceName(std::string deviceName) {
  m_deviceName = deviceName;
}

void EmuFdNetDeviceHelper::HostQdiscBypass(bool hostQdiscBypass) {
  m_hostQdiscBypass = hostQdiscBypass;
}

std::string EmuFdNetDeviceHelper::GetDeviceName() { return m_deviceName; }

Ptr<NetDevice> EmuFdNetDeviceHelper::InstallPriv(Ptr<Node> node) const {
  Ptr<NetDevice> d = FdNetDeviceHelper::InstallPriv(node);
  Ptr<FdNetDevice> device = d->GetObject<FdNetDevice>();
  SetFileDescriptor(device);
  return device;
}

void EmuFdNetDeviceHelper::SetFileDescriptor(Ptr<FdNetDevice> device) const {
  NS_LOG_LOGIC("Creating EMU socket");

  if (m_deviceName == "undefined") {
    NS_FATAL_ERROR(
        "EmuFdNetDeviceHelper::SetFileDescriptor (): m_deviceName is not set");
  }

  int fd = CreateFileDescriptor();
  device->SetFileDescriptor(fd);

  ifreq ifr;
  bzero(&ifr, sizeof(ifr));
  strncpy((char *)ifr.ifr_name, m_deviceName.c_str(), IFNAMSIZ - 1);

  NS_LOG_LOGIC("Getting interface index");
  int32_t rc = ioctl(fd, SIOCGIFINDEX, &ifr);
  if (rc == -1) {
    NS_FATAL_ERROR("EmuFdNetDeviceHelper::SetFileDescriptor (): Can't get "
                   "interface index");
  }

  struct sockaddr_ll ll;
  bzero(&ll, sizeof(ll));

  ll.sll_family = AF_PACKET;
  ll.sll_ifindex = ifr.ifr_ifindex;
  ll.sll_protocol = htons(ETH_P_ALL);

  NS_LOG_LOGIC("Binding socket to interface");

  rc = bind(fd, (struct sockaddr *)&ll, sizeof(ll));
  if (rc == -1) {
    NS_FATAL_ERROR("EmuFdNetDeviceHelper::SetFileDescriptor (): Can't bind to "
                   "specified interface");
  }

  rc = ioctl(fd, SIOCGIFFLAGS, &ifr);
  if (rc == -1) {
    NS_FATAL_ERROR("EmuFdNetDeviceHelper::SetFileDescriptor (): Can't get "
                   "interface flags");
  }

  if (m_hostQdiscBypass) {
#ifdef PACKET_QDISC_BYPASS
    static const int32_t sock_qdisc_bypass = 1;
    int32_t sock_qdisc_ret =
        setsockopt(fd, SOL_PACKET, PACKET_QDISC_BYPASS, &sock_qdisc_bypass,
                   sizeof(sock_qdisc_bypass));

    if (sock_qdisc_ret == -1) {
      NS_LOG_ERROR("Cannot use the qdisc bypass option");
    }
#else
    NS_LOG_ERROR(
        "PACKET_QDISC_BYPASS undefined; cannot use the qdisc bypass option");
#endif
  }

  if ((ifr.ifr_flags & IFF_PROMISC) == 0) {
    NS_FATAL_ERROR("EmuFdNetDeviceHelper::SetFileDescriptor (): "
                   << m_deviceName << " is not in promiscuous mode");
  }

  if ((ifr.ifr_flags & IFF_BROADCAST) != IFF_BROADCAST) {
    device->SetIsBroadcast(false);
  }

  if ((ifr.ifr_flags & IFF_MULTICAST) == IFF_MULTICAST) {
    device->SetIsMulticast(true);
  }

  ifreq ifr2;

  bzero(&ifr2, sizeof(ifr2));
  strcpy(ifr2.ifr_name, m_deviceName.c_str());

  int32_t mtufd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

  rc = ioctl(mtufd, SIOCGIFMTU, &ifr2);
  if (rc == -1) {
    NS_FATAL_ERROR("FdNetDevice::SetFileDescriptor (): Can't ioctl SIOCGIFMTU");
  }

  close(mtufd);
  device->SetMtu(ifr2.ifr_mtu);
}

int EmuFdNetDeviceHelper::CreateFileDescriptor() const {
  NS_LOG_FUNCTION(this);

  int sock = socket(PF_UNIX, SOCK_DGRAM, 0);
  if (sock == -1) {
    NS_FATAL_ERROR("EmuFdNetDeviceHelper::CreateFileDescriptor(): Unix socket "
                   "creation error, errno = "
                   << strerror(errno));
  }

  struct sockaddr_un un;
  memset(&un, 0, sizeof(un));
  un.sun_family = AF_UNIX;
  int status = bind(sock, (struct sockaddr *)&un, sizeof(sa_family_t));
  if (status == -1) {
    NS_FATAL_ERROR("EmuFdNetDeviceHelper::CreateFileDescriptor(): Could not "
                   "bind(): errno = "
                   << strerror(errno));
  }

  NS_LOG_INFO("Created Unix socket");
  NS_LOG_INFO("sun_family = " << un.sun_family);
  NS_LOG_INFO("sun_path = " << un.sun_path);

  socklen_t len = sizeof(un);
  status = getsockname(sock, (struct sockaddr *)&un, &len);
  if (status == -1) {
    NS_FATAL_ERROR("EmuFdNetDeviceHelper::CreateFileDescriptor(): Could not "
                   "getsockname(): errno = "
                   << strerror(errno));
  }

  std::string path = BufferToString((uint8_t *)&un, len);
  NS_LOG_INFO("Encoded Unix socket as \"" << path << "\"");
  pid_t pid = ::fork();
  if (pid == 0) {
    NS_LOG_DEBUG("Child process");

    std::ostringstream oss;
    oss << "-p" << path;
    NS_LOG_INFO("Parameters set to \"" << oss.str() << "\"");

    status = ::execlp(RAW_SOCK_CREATOR, RAW_SOCK_CREATOR, oss.str().c_str(),
                      (char *)nullptr);

    NS_FATAL_ERROR("EmuFdNetDeviceHelper::CreateFileDescriptor(): Back from "
                   "execlp(), status = "
                   << status << ", errno = " << ::strerror(errno));
  } else {
    NS_LOG_DEBUG("Parent process");
    int st;
    pid_t waited = waitpid(pid, &st, 0);
    if (waited == -1) {
      NS_FATAL_ERROR("EmuFdNetDeviceHelper::CreateFileDescriptor(): waitpid() "
                     "fails, errno = "
                     << strerror(errno));
    }
    NS_ASSERT_MSG(pid == waited,
                  "EmuFdNetDeviceHelper::CreateFileDescriptor(): pid mismatch");

    if (WIFEXITED(st)) {
      int exitStatus = WEXITSTATUS(st);
      if (exitStatus != 0) {
        NS_FATAL_ERROR(
            "EmuFdNetDeviceHelper::CreateFileDescriptor(): socket creator "
            "exited normally with status "
            << exitStatus);
      }
    } else {
      NS_FATAL_ERROR("EmuFdNetDeviceHelper::CreateFileDescriptor(): socket "
                     "creator exited abnormally");
    }

    iovec iov;
    uint32_t magic;
    iov.iov_base = &magic;
    iov.iov_len = sizeof(magic);

    size_t msg_size = sizeof(int);
    char control[CMSG_SPACE(msg_size)];

    msghdr msg;
    msg.msg_name = nullptr;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = control;
    msg.msg_controllen = sizeof(control);
    msg.msg_flags = 0;

    ssize_t bytesRead = recvmsg(sock, &msg, 0);
    if (bytesRead != sizeof(int)) {
      NS_FATAL_ERROR(
          "EmuFdNetDeviceHelper::CreateFileDescriptor(): Wrong byte count from "
          "socket creator");
    }

    struct cmsghdr *cmsg;
    for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != nullptr;
         cmsg = CMSG_NXTHDR(&msg, cmsg)) {
      if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS) {
        if (magic == EMU_MAGIC) {
          NS_LOG_INFO("Got SCM_RIGHTS with correct magic " << magic);
          int *rawSocket = (int *)CMSG_DATA(cmsg);
          NS_LOG_INFO(
              "Got the socket from the socket creator = " << *rawSocket);
          return *rawSocket;
        } else {
          NS_LOG_INFO("Got SCM_RIGHTS, but with bad magic " << magic);
        }
      }
    }
    NS_FATAL_ERROR("Did not get the raw socket from the socket creator");
  }
  NS_FATAL_ERROR("Should be unreachable");
  return 0;
}

} // namespace ns3
