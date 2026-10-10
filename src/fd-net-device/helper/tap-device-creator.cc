
#include "creator-utils.h"

#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <linux/if_tun.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <sstream>
#include <stdint.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define TAP_MAGIC 95549

#define ASCII_DOT (0x2e)
#define ASCII_ZERO (0x30)
#define ASCII_a (0x41)
#define ASCII_z (0x5a)
#define ASCII_A (0x61)
#define ASCII_Z (0x7a)
#define ASCII_COLON (0x3a)
#define ASCII_ZERO (0x30)

using namespace ns3;

struct in6_ifreq {
  struct in6_addr ifr6_addr;
  uint32_t ifr6_prefixlen;
  int32_t ifr6_ifindex;
};

char AsciiToLowCase(char c) {
  if (c >= ASCII_a && c <= ASCII_z) {
    return c;
  } else if (c >= ASCII_A && c <= ASCII_Z) {
    return c + (ASCII_a - ASCII_A);
  } else {
    return c;
  }
}

void AsciiToMac48(const char *str, uint8_t addr[6]) {
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

void SetIpv4(const char *deviceName, const char *ip, const char *netmask) {
  struct ifreq ifr;
  struct sockaddr_in *sin;

  int sock = socket(AF_INET, SOCK_DGRAM, 0);

  memset(&ifr, 0, sizeof(struct ifreq));
  strncpy(ifr.ifr_name, deviceName, IFNAMSIZ - 1);

  sin = (struct sockaddr_in *)&ifr.ifr_addr;
  inet_pton(AF_INET, ip, &sin->sin_addr);
  ifr.ifr_addr.sa_family = AF_INET;

  ABORT_IF(ioctl(sock, SIOCSIFADDR, &ifr) == -1, "Could not set IP address",
           true);

  LOG("Set device IP address to " << ip);

  memset(&ifr, 0, sizeof(struct ifreq));
  strncpy(ifr.ifr_name, deviceName, IFNAMSIZ - 1);

  sin = (struct sockaddr_in *)&ifr.ifr_netmask;
  inet_pton(AF_INET, netmask, &sin->sin_addr);
  ifr.ifr_addr.sa_family = AF_INET;

  ABORT_IF(ioctl(sock, SIOCSIFNETMASK, &ifr) == -1, "Could not set net mask",
           true);

  LOG("Set device Net Mask to " << netmask);
  close(sock);
}

void SetIpv6(const char *deviceName, const char *ip, int netprefix) {
  struct ifreq ifr;
  struct sockaddr_in6 sin;
  struct in6_ifreq ifr6;

  int sock = socket(AF_INET6, SOCK_DGRAM, 0);
  memset(&ifr, 0, sizeof(struct ifreq));
  strncpy(ifr.ifr_name, deviceName, IFNAMSIZ - 1);

  ABORT_IF(ioctl(sock, SIOGIFINDEX, &ifr) == -1,
           "Could not get interface index", true);

  LOG("Set device IP v6 address to " << ip);

  memset(&sin, 0, sizeof(struct sockaddr_in6));
  sin.sin6_family = AF_INET6;
  inet_pton(AF_INET6, ip, (void *)&sin.sin6_addr);

  memset(&ifr6, 0, sizeof(in6_ifreq));
  memcpy((char *)&ifr6.ifr6_addr, (char *)&sin.sin6_addr,
         sizeof(struct in6_addr));

  ifr6.ifr6_ifindex = ifr.ifr_ifindex;
  ifr6.ifr6_prefixlen = netprefix;

  ABORT_IF(ioctl(sock, SIOCSIFADDR, &ifr6) == -1, "Could not set IP v6 address",
           true);

  LOG("Set device IP v6 address to " << ip);
  close(sock);
}

void SetMacAddress(int fd, const char *mac) {
  struct ifreq ifr;
  memset(&ifr, 0, sizeof(struct ifreq));

  ifr.ifr_hwaddr.sa_family = 1;
  AsciiToMac48(mac, (uint8_t *)ifr.ifr_hwaddr.sa_data);
  ABORT_IF(ioctl(fd, SIOCSIFHWADDR, &ifr) == -1, "Could not set MAC address",
           true);
  LOG("Set device MAC address to " << mac);
}

void SetUp(char *deviceName) {
  struct ifreq ifr;

  int sock = socket(AF_INET, SOCK_DGRAM, 0);

  memset(&ifr, 0, sizeof(struct ifreq));
  strncpy(ifr.ifr_name, deviceName, IFNAMSIZ - 1);

  ABORT_IF(ioctl(sock, SIOCGIFFLAGS, &ifr) == -1,
           "Could not get flags for interface", true);
  ifr.ifr_flags |= IFF_UP | IFF_RUNNING;

  ABORT_IF(ioctl(sock, SIOCSIFFLAGS, &ifr) == -1,
           "Could not bring interface up", true);

  LOG("Device is up");
  close(sock);
}

int CreateTap(char *deviceName, const char *mac, bool ifftap, bool iffpi,
              const char *ip4, const char *netmask, const char *ip6,
              const int netprefix) {
  int fd = open("/dev/net/tun", O_RDWR);
  ABORT_IF(fd == -1, "Could not open /dev/net/tun", true);

  struct ifreq ifr;

  memset(&ifr, 0, sizeof(struct ifreq));

  ifr.ifr_flags = (ifftap ? IFF_TAP : IFF_TUN);
  if (!iffpi) {
    ifr.ifr_flags |= IFF_NO_PI;
  }

  if (*deviceName) {
    strncpy(ifr.ifr_name, deviceName, IFNAMSIZ - 1);
  }

  ABORT_IF(ioctl(fd, TUNSETIFF, (void *)&ifr) == -1,
           "Could not allocate tap device", true);

  LOG("Allocated TAP device " << deviceName);

  if (ifftap) {
    SetMacAddress(fd, mac);
  }

  if (ip4) {
    SetIpv4(deviceName, ip4, netmask);
  }

  if (ip6) {
    SetIpv6(deviceName, ip6, netprefix);
  }

  SetUp(deviceName);

  return fd;
}

int main(int argc, char *argv[]) {
  int c;
  char *dev = nullptr;
  char *ip4 = nullptr;
  char *ip6 = nullptr;
  char *mac = nullptr;
  char *netmask = nullptr;
  char *path = nullptr;
  bool tap = false;
  bool pi = false;
  int prefix = -1;

  while ((c = getopt(argc, argv, "vd:i:m:n:I:P:thp:")) != -1) {
    switch (c) {
    case 'd':
      dev = optarg;
      break;
    case 'i':
      ip4 = optarg;
      break;
    case 'I':
      ip6 = optarg;
      break;
    case 'm':
      mac = optarg;
      break;
    case 'n':
      netmask = optarg;
      break;
    case 'P':
      prefix = atoi(optarg);
      break;
    case 't':
      tap = true;
      break;
    case 'h':
      pi = true;
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

  ABORT_IF(ip4 == nullptr && ip6 == nullptr,
           "IP Address is a required argument", 0);
  if (ip4) {
    ABORT_IF(netmask == nullptr, "Net mask is a required argument", 0);
    LOG("Provided IP v4 Address is \"" << ip4 << "\"");
    LOG("Provided IP v4 Net Mask is \"" << netmask << "\"");
  }
  if (ip6) {
    ABORT_IF(prefix == -1, "Prefix is a required argument", 0);
    LOG("Provided IP v6 Address is \"" << ip6 << "\"");
    LOG("Provided IP v6 Prefix is \"" << prefix << "\"");
  }

  ABORT_IF(mac == nullptr, "MAC Address is a required argument", 0);
  LOG("Provided MAC Address is \"" << mac << "\"");

  if (tap) {
    LOG("Provided device Mode is TAP");
  } else {
    LOG("Provided device Mode is TUN");
  }

  if (pi) {
    LOG("IFF_NO_PI flag set. Packet Information will be present in the "
        "traffic");
  }

  ABORT_IF(path == nullptr, "path is a required argument", 0);
  LOG("Provided path is \"" << path << "\"");

  LOG("Creating Tap");
  int sock = CreateTap(dev, mac, tap, pi, ip4, netmask, ip6, prefix);
  ABORT_IF(sock == -1, "main(): Unable to create tap socket", 1);

  SendSocket(path, sock, TAP_MAGIC);

  return 0;
}
