
#include "tap-fd-net-device-helper.h"

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

NS_LOG_COMPONENT_DEFINE("TapFdNetDeviceHelper");

#define TAP_MAGIC 95549

TapFdNetDeviceHelper::TapFdNetDeviceHelper() {
  m_deviceName = "";
  m_modePi = false;
  m_tapIp4 = Ipv4Address::GetZero();
  m_tapMask4 = Ipv4Mask::GetZero();
  m_tapIp6 = Ipv6Address::GetZero();
  m_tapPrefix6 = 64;
  m_tapMac = Mac48Address::Allocate();
}

void TapFdNetDeviceHelper::SetModePi(bool modePi) { m_modePi = modePi; }

void TapFdNetDeviceHelper::SetTapIpv4Address(Ipv4Address address) {
  m_tapIp4 = address;
}

void TapFdNetDeviceHelper::SetTapIpv4Mask(Ipv4Mask mask) { m_tapMask4 = mask; }

void TapFdNetDeviceHelper::SetTapIpv6Address(Ipv6Address address) {
  m_tapIp6 = address;
}

void TapFdNetDeviceHelper::SetTapIpv6Prefix(int prefix) {
  m_tapPrefix6 = prefix;
}

void TapFdNetDeviceHelper::SetTapMacAddress(Mac48Address mac) {
  m_tapMac = mac;
}

Ptr<NetDevice> TapFdNetDeviceHelper::InstallPriv(Ptr<Node> node) const {
  Ptr<NetDevice> d = FdNetDeviceHelper::InstallPriv(node);
  Ptr<FdNetDevice> device = d->GetObject<FdNetDevice>();

  if (m_modePi) {
    device->SetEncapsulationMode(FdNetDevice::DIXPI);
  }

  SetFileDescriptor(device);
  return device;
}

void TapFdNetDeviceHelper::SetFileDescriptor(Ptr<FdNetDevice> device) const {
  NS_LOG_LOGIC("Creating TAP device");

  int fd = CreateFileDescriptor();
  device->SetFileDescriptor(fd);
}

int TapFdNetDeviceHelper::CreateFileDescriptor() const {
  NS_LOG_FUNCTION(this);

  int sock = socket(PF_UNIX, SOCK_DGRAM, 0);
  NS_ABORT_MSG_IF(sock == -1, "TapFdNetDeviceHelper::CreateFileDescriptor(): "
                              "Unix socket creation error, errno = "
                                  << strerror(errno));

  struct sockaddr_un un;
  memset(&un, 0, sizeof(un));
  un.sun_family = AF_UNIX;
  int status = bind(sock, (struct sockaddr *)&un, sizeof(sa_family_t));
  NS_ABORT_MSG_IF(
      status == -1,
      "TapFdNetDeviceHelper::CreateFileDescriptor(): Could not bind(): errno = "
          << strerror(errno));
  NS_LOG_INFO("Created Unix socket");
  NS_LOG_INFO("sun_family = " << un.sun_family);
  NS_LOG_INFO("sun_path = " << un.sun_path);

  socklen_t len = sizeof(un);
  status = getsockname(sock, (struct sockaddr *)&un, &len);
  NS_ABORT_MSG_IF(status == -1, "TapFdNetDeviceHelper::CreateFileDescriptor(): "
                                "Could not getsockname(): errno = "
                                    << strerror(errno));

  std::string path = BufferToString((uint8_t *)&un, len);
  NS_LOG_INFO("Encoded Unix socket as \"" << path << "\"");

  pid_t pid = ::fork();
  if (pid == 0) {
    NS_LOG_DEBUG("Child process");

    std::ostringstream ossDeviceName;
    if (!m_deviceName.empty()) {
      ossDeviceName << "-d" << m_deviceName;
    }

    std::ostringstream ossMac;
    ossMac << "-m" << m_tapMac;

    std::ostringstream ossIp4;
    if (m_tapIp4 != Ipv4Address::GetZero()) {
      ossIp4 << "-i" << m_tapIp4;
    }

    std::ostringstream ossIp6;
    if (m_tapIp6 != Ipv6Address::GetZero()) {
      ossIp6 << "-I" << m_tapIp6;
    }

    std::ostringstream ossNetmask4;
    if (m_tapMask4 != Ipv4Mask::GetZero()) {
      ossNetmask4 << "-n" << m_tapMask4;
    }

    std::ostringstream ossPrefix6;
    ossPrefix6 << "-P" << m_tapPrefix6;

    std::ostringstream ossMode;
    ossMode << "-t";

    std::ostringstream ossPI;
    if (m_modePi) {
      ossPI << "-h";
    }

    std::ostringstream ossPath;
    ossPath << "-p" << path;

    status =
        ::execlp(TAP_DEV_CREATOR, TAP_DEV_CREATOR, ossDeviceName.str().c_str(),
                 ossMac.str().c_str(), ossIp4.str().c_str(),
                 ossIp6.str().c_str(), ossNetmask4.str().c_str(),
                 ossPrefix6.str().c_str(), ossMode.str().c_str(),
                 ossPI.str().c_str(), ossPath.str().c_str(), (char *)nullptr);

    NS_FATAL_ERROR("TapFdNetDeviceHelper::CreateFileDescriptor(): Back from "
                   "execlp(), status = "
                   << status << ", errno = " << ::strerror(errno));
  } else {
    NS_LOG_DEBUG("Parent process");
    int st;
    pid_t waited = waitpid(pid, &st, 0);
    NS_ABORT_MSG_IF(
        waited == -1,
        "TapFdNetDeviceHelper::CreateFileDescriptor(): waitpid() fails, errno "
        "= " << strerror(errno));
    NS_ASSERT_MSG(pid == waited,
                  "TapFdNetDeviceHelper::CreateFileDescriptor(): pid mismatch");

    if (WIFEXITED(st)) {
      int exitStatus = WEXITSTATUS(st);
      NS_ABORT_MSG_IF(
          exitStatus != 0,
          "TapFdNetDeviceHelper::CreateFileDescriptor(): socket creator exited "
          "normally with status "
              << exitStatus);
    } else {
      NS_FATAL_ERROR("TapFdNetDeviceHelper::CreateFileDescriptor(): socket "
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
    NS_ABORT_MSG_IF(bytesRead != sizeof(int),
                    "TapFdNetDeviceHelper::CreateFileDescriptor(): Wrong byte "
                    "count from socket creator");

    struct cmsghdr *cmsg;
    for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != nullptr;
         cmsg = CMSG_NXTHDR(&msg, cmsg)) {
      if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS) {
        if (magic == TAP_MAGIC) {
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
