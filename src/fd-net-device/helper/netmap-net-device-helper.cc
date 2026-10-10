
#include "netmap-net-device-helper.h"

#include "encode-decode.h"

#include "ns3/abort.h"
#include "ns3/config.h"
#include "ns3/fd-net-device.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/netmap-net-device.h"
#include "ns3/object-factory.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/trace-helper.h"
#include "ns3/uinteger.h"

#include <arpa/inet.h>
#include <errno.h>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <net/ethernet.h>
#include <net/if.h>
#include <net/netmap_user.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("NetmapNetDeviceHelper");

#define EMU_MAGIC 65867

NetmapNetDeviceHelper::NetmapNetDeviceHelper() {
  m_deviceName = "undefined";
  SetTypeId("ns3::NetmapNetDevice");
}

std::string NetmapNetDeviceHelper::GetDeviceName() { return m_deviceName; }

void NetmapNetDeviceHelper::SetDeviceName(std::string deviceName) {
  m_deviceName = deviceName;
}

Ptr<NetDevice> NetmapNetDeviceHelper::InstallPriv(Ptr<Node> node) const {
  Ptr<NetDevice> d = FdNetDeviceHelper::InstallPriv(node);
  Ptr<FdNetDevice> device = d->GetObject<FdNetDevice>();

  SetDeviceAttributes(device);

  int fd = CreateFileDescriptor();
  Ptr<NetmapNetDevice> netmapDevice = DynamicCast<NetmapNetDevice>(device);
  SwitchInNetmapMode(fd, netmapDevice);

  Ptr<NetDeviceQueueInterface> ndqi =
      CreateObjectWithAttributes<NetDeviceQueueInterface>(
          "TxQueuesType", TypeIdValue(NetDeviceQueueLock::GetTypeId()),
          "NTxQueues", UintegerValue(1));

  device->AggregateObject(ndqi);
  netmapDevice->SetNetDeviceQueue(ndqi->GetTxQueue(0));

  return device;
}

void NetmapNetDeviceHelper::SetDeviceAttributes(Ptr<FdNetDevice> device) const {
  if (m_deviceName == "undefined") {
    NS_FATAL_ERROR(
        "NetmapNetDeviceHelper::SetFileDescriptor (): m_deviceName is not set");
  }

  int fd = socket(PF_INET, SOCK_DGRAM, 0);

  struct ifreq ifr;
  bzero(&ifr, sizeof(ifr));
  strncpy((char *)ifr.ifr_name, m_deviceName.c_str(), IFNAMSIZ - 1);

  NS_LOG_LOGIC("Getting interface index");
  int32_t rc = ioctl(fd, SIOCGIFINDEX, &ifr);
  if (rc == -1) {
    NS_FATAL_ERROR("NetmapNetDeviceHelper::SetFileDescriptor (): Can't get "
                   "interface index");
  }

  rc = ioctl(fd, SIOCGIFFLAGS, &ifr);
  if (rc == -1) {
    NS_FATAL_ERROR("NetmapNetDeviceHelper::SetFileDescriptor (): Can't get "
                   "interface flags");
  }

  if ((ifr.ifr_flags & IFF_PROMISC) == 0) {
    NS_FATAL_ERROR("NetmapNetDeviceHelper::SetFileDescriptor (): "
                   << m_deviceName
                   << " is not in promiscuous mode. Please config the "
                      "interface in promiscuous "
                      "mode before to run the simulation.");
  }

  if ((ifr.ifr_flags & IFF_BROADCAST) != IFF_BROADCAST) {
    device->SetIsBroadcast(false);
  }

  if ((ifr.ifr_flags & IFF_MULTICAST) == IFF_MULTICAST) {
    device->SetIsMulticast(true);
  }

  rc = ioctl(fd, SIOCGIFMTU, &ifr);
  if (rc == -1) {
    NS_FATAL_ERROR("FdNetDevice::SetFileDescriptor (): Can't ioctl SIOCGIFMTU");
  }

  NS_LOG_DEBUG("Device MTU " << ifr.ifr_mtu);
  device->SetMtu(ifr.ifr_mtu);

  close(fd);
}

int NetmapNetDeviceHelper::CreateFileDescriptor() const {
  NS_LOG_FUNCTION(this);

  int sock = socket(PF_UNIX, SOCK_DGRAM, 0);
  if (sock == -1) {
    NS_FATAL_ERROR("NetmapNetDeviceHelper::CreateFileDescriptor(): Unix socket "
                   "creation error, errno = "
                   << strerror(errno));
  }

  struct sockaddr_un un;
  memset(&un, 0, sizeof(un));
  un.sun_family = AF_UNIX;
  int status = bind(sock, (struct sockaddr *)&un, sizeof(sa_family_t));
  if (status == -1) {
    NS_FATAL_ERROR("NetmapNetDeviceHelper::CreateFileDescriptor(): Could not "
                   "bind(): errno = "
                   << strerror(errno));
  }

  NS_LOG_INFO("Created Unix socket");
  NS_LOG_INFO("sun_family = " << un.sun_family);
  NS_LOG_INFO("sun_path = " << un.sun_path);

  socklen_t len = sizeof(un);
  status = getsockname(sock, (struct sockaddr *)&un, &len);
  if (status == -1) {
    NS_FATAL_ERROR("NetmapNetDeviceHelper::CreateFileDescriptor(): Could not "
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

    status = ::execlp(NETMAP_DEV_CREATOR, NETMAP_DEV_CREATOR, oss.str().c_str(),
                      nullptr);

    NS_FATAL_ERROR("NetmapNetDeviceHelper::CreateFileDescriptor(): Back from "
                   "execlp(), status = "
                   << status << ", errno = " << ::strerror(errno));
  } else {
    NS_LOG_DEBUG("Parent process");
    int st;
    pid_t waited = waitpid(pid, &st, 0);
    if (waited == -1) {
      NS_FATAL_ERROR("NetmapNetDeviceHelper::CreateFileDescriptor(): waitpid() "
                     "fails, errno = "
                     << strerror(errno));
    }
    NS_ASSERT_MSG(
        pid == waited,
        "NetmapNetDeviceHelper::CreateFileDescriptor(): pid mismatch");

    if (WIFEXITED(st)) {
      int exitStatus = WEXITSTATUS(st);
      if (exitStatus != 0) {
        NS_FATAL_ERROR(
            "NetmapNetDeviceHelper::CreateFileDescriptor(): socket creator "
            "exited normally with status "
            << exitStatus);
      }
    } else {
      NS_FATAL_ERROR("NetmapNetDeviceHelper::CreateFileDescriptor(): socket "
                     "creator exited abnormally");
    }

    struct iovec iov;
    uint32_t magic;
    iov.iov_base = &magic;
    iov.iov_len = sizeof(magic);

    size_t msg_size = sizeof(int);
    char control[CMSG_SPACE(msg_size)];

    struct msghdr msg;
    msg.msg_name = nullptr;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = control;
    msg.msg_controllen = sizeof(control);
    msg.msg_flags = 0;

    ssize_t bytesRead = recvmsg(sock, &msg, 0);
    if (bytesRead != sizeof(int)) {
      NS_FATAL_ERROR("NetmapNetDeviceHelper::CreateFileDescriptor(): Wrong "
                     "byte count from "
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
}

void NetmapNetDeviceHelper::SwitchInNetmapMode(
    int fd, Ptr<NetmapNetDevice> device) const {
  NS_LOG_FUNCTION(this << fd << device);
  NS_ASSERT(device);

  if (m_deviceName == "undefined") {
    NS_FATAL_ERROR("NetmapNetDevice: m_deviceName is not set");
  }

  if (fd == -1) {
    NS_FATAL_ERROR("NetmapNetDevice: fd is not set");
  }

  struct nmreq nmr;
  memset(&nmr, 0, sizeof(nmr));

  nmr.nr_version = NETMAP_API;

  strncpy(nmr.nr_name, m_deviceName.c_str(), m_deviceName.length());

  int code = ioctl(fd, NIOCREGIF, &nmr);
  if (code == -1) {
    NS_FATAL_ERROR("Switching failed");
  }

  uint8_t *memory = (uint8_t *)mmap(0, nmr.nr_memsize, PROT_WRITE | PROT_READ,
                                    MAP_SHARED, fd, 0);

  if (memory == MAP_FAILED) {
    NS_FATAL_ERROR("Memory mapping failed");
  }

  struct netmap_if *nifp = NETMAP_IF(memory, nmr.nr_offset);

  if (!nifp) {
    NS_FATAL_ERROR(
        "Failed getting the base struct of the interface in netmap mode");
  }

  device->SetNetmapInterfaceRepresentation(nifp);
  device->SetTxRingsInfo(nifp->ni_tx_rings, nmr.nr_tx_slots);
  device->SetRxRingsInfo(nifp->ni_rx_rings, nmr.nr_rx_slots);

  device->SetFileDescriptor(fd);
}

} // namespace ns3
