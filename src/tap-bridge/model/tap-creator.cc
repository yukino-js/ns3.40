
#include "tap-encode-decode.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <linux/if_tun.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <sstream>
#include <stdint.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define TAP_MAGIC 95549

static bool gVerbose = false;

#define LOG(msg)                                                               \
  if (gVerbose) {                                                              \
    std::cout << __FUNCTION__ << "(): " << msg << std::endl;                   \
  }

#define ABORT(msg, printErrno)                                                 \
  std::cout << __FILE__ << ": fatal error at line " << __LINE__ << ": "        \
            << __FUNCTION__ << "(): " << msg << std::endl;                     \
  if (printErrno) {                                                            \
    std::cout << "    errno = " << errno << " (" << std::strerror(errno)       \
              << ")" << std::endl;                                             \
  }                                                                            \
  std::exit(-1);

#define ABORT_IF(cond, msg, printErrno)                                        \
  if (cond) {                                                                  \
    ABORT(msg, printErrno);                                                    \
  }

#define ASCII_DOT (0x2e)
#define ASCII_ZERO (0x30)
#define ASCII_a (0x41)
#define ASCII_z (0x5a)
#define ASCII_A (0x61)
#define ASCII_Z (0x7a)
#define ASCII_COLON (0x3a)

static char AsciiToLowCase(char c) {
  if (c >= ASCII_a && c <= ASCII_z) {
    return c;
  } else if (c >= ASCII_A && c <= ASCII_Z) {
    return c + (ASCII_a - ASCII_A);
  } else {
    return c;
  }
}

static uint32_t AsciiToIpv4(const char *address) {
  uint32_t host = 0;
  while (true) {
    uint8_t byte = 0;
    while (*address != ASCII_DOT && *address != 0) {
      byte *= 10;
      byte += *address - ASCII_ZERO;
      address++;
    }
    host <<= 8;
    host |= byte;
    if (*address == 0) {
      break;
    }
    address++;
  }
  return host;
}

static void AsciiToMac48(const char *str, uint8_t addr[6]) {
  int i = 0;
  while (*str != 0 && i < 6) {
    uint8_t byte = 0;
    while (*str != ASCII_COLON && *str != 0) {
      byte <<= 4;
      char low = AsciiToLowCase(*str);
      if (low >= ASCII_a) {
        byte |= low - ASCII_a + 10;
      } else {
        byte |= low - ASCII_ZERO;
      }
      str++;
    }
    addr[i] = byte;
    i++;
    if (*str == 0) {
      break;
    }
    str++;
  }
}

static sockaddr CreateInetAddress(uint32_t networkOrder) {
  union {
    struct sockaddr any_socket;
    struct sockaddr_in si;
  } s;

  s.si.sin_family = AF_INET;
  s.si.sin_port = 0;
  s.si.sin_addr.s_addr = htonl(networkOrder);
  return s.any_socket;
}

static void SendSocket(const char *path, int fd) {
  LOG("Create Unix socket");
  int sock = socket(PF_UNIX, SOCK_DGRAM, 0);
  ABORT_IF(sock == -1, "Unable to open socket", 1);

  socklen_t clientAddrLen;
  struct sockaddr_un clientAddr;

  LOG("Decode address " << path);
  bool rc =
      ns3::TapStringToBuffer(path, (uint8_t *)&clientAddr, &clientAddrLen);
  ABORT_IF(rc == false, "Unable to decode path", 0);

  LOG("Connect");
  int status = connect(sock, (struct sockaddr *)&clientAddr, clientAddrLen);
  ABORT_IF(status == -1, "Unable to connect to tap bridge", 1);

  LOG("Connected");

  struct iovec iov;
  uint32_t magic = TAP_MAGIC;
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

  struct cmsghdr *cmsg;
  cmsg = CMSG_FIRSTHDR(&msg);
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SCM_RIGHTS;
  cmsg->cmsg_len = CMSG_LEN(msg_size);
  msg.msg_controllen = cmsg->cmsg_len;

  int *fdptr = (int *)(CMSG_DATA(cmsg));
  *fdptr = fd;

  ssize_t len = sendmsg(sock, &msg, 0);
  ABORT_IF(len == -1, "Could not send socket back to tap bridge", 1);

  LOG("sendmsg complete");
}

static int CreateTap(const char *dev, const char *gw, const char *ip,
                     const char *mac, const char *mode, const char *netmask) {
  int tap = open("/dev/net/tun", O_RDWR);
  ABORT_IF(tap == -1, "Could not open /dev/net/tun", true);

  struct ifreq ifr;
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
  strcpy(ifr.ifr_name, dev);
  int status = ioctl(tap, TUNSETIFF, (void *)&ifr);
  ABORT_IF(status == -1, "Could not allocate tap device", true);

  std::string tapDeviceName = (char *)ifr.ifr_name;
  LOG("Allocated TAP device " << tapDeviceName);

  if (std::string(mode) == "2" || std::string(mode) == "3") {
    LOG("Returning precreated tap ");
    return tap;
  }

  ifr.ifr_hwaddr.sa_family = 1;
  AsciiToMac48(mac, (uint8_t *)ifr.ifr_hwaddr.sa_data);
  status = ioctl(tap, SIOCSIFHWADDR, &ifr);
  ABORT_IF(status == -1, "Could not set MAC address", true);
  LOG("Set device MAC address to " << mac);

  int fd = socket(AF_INET, SOCK_DGRAM, 0);

  status = ioctl(fd, SIOCGIFFLAGS, &ifr);
  ABORT_IF(status == -1, "Could not get flags for interface", true);
  ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
  status = ioctl(fd, SIOCSIFFLAGS, &ifr);
  ABORT_IF(status == -1, "Could not bring interface up", true);
  LOG("Device is up");

  ifr.ifr_addr = CreateInetAddress(AsciiToIpv4(ip));
  status = ioctl(fd, SIOCSIFADDR, &ifr);
  ABORT_IF(status == -1, "Could not set IP address", true);
  LOG("Set device IP address to " << ip);

  ifr.ifr_netmask = CreateInetAddress(AsciiToIpv4(netmask));
  status = ioctl(fd, SIOCSIFNETMASK, &ifr);
  ABORT_IF(status == -1, "Could not set net mask", true);
  LOG("Set device Net Mask to " << netmask);

  return tap;
}

int main(int argc, char *argv[]) {
  int c;
  char *dev = (char *)"";
  char *gw = nullptr;
  char *ip = nullptr;
  char *mac = nullptr;
  char *netmask = nullptr;
  char *operatingMode = nullptr;
  char *path = nullptr;

  opterr = 0;

  while ((c = getopt(argc, argv, "vd:g:i:m:n:o:p:")) != -1) {
    switch (c) {
    case 'd':
      dev = optarg;
      break;
    case 'g':
      gw = optarg;
      break;
    case 'i':
      ip = optarg;
      break;
    case 'm':
      mac = optarg;
      break;
    case 'n':
      netmask = optarg;
      break;
    case 'o':
      operatingMode = optarg;
      break;
    case 'p':
      path = optarg;
      break;
    case 'v':
      gVerbose = true;
      break;
    }
  }

  LOG("Provided Device Name is \"" << dev << "\"");

  ABORT_IF(gw == nullptr, "Gateway Address is a required argument", 0);
  LOG("Provided Gateway Address is \"" << gw << "\"");

  ABORT_IF(ip == nullptr, "IP Address is a required argument", 0);
  LOG("Provided IP Address is \"" << ip << "\"");

  ABORT_IF(mac == nullptr, "MAC Address is a required argument", 0);
  LOG("Provided MAC Address is \"" << mac << "\"");

  ABORT_IF(netmask == nullptr, "Net Mask is a required argument", 0);
  LOG("Provided Net Mask is \"" << netmask << "\"");

  ABORT_IF(operatingMode == nullptr, "Operating Mode is a required argument",
           0);
  LOG("Provided Operating Mode is \"" << operatingMode << "\"");

  ABORT_IF(path == nullptr, "path is a required argument", 0);
  LOG("Provided path is \"" << path << "\"");

  LOG("Creating Tap");
  int sock = CreateTap(dev, gw, ip, mac, operatingMode, netmask);
  ABORT_IF(sock == -1, "main(): Unable to create tap socket", 1);

  SendSocket(path, sock);

  return 0;
}
