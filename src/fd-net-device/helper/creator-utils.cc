
#include "creator-utils.h"

#include "encode-decode.h"

#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <iomanip>
#include <iostream>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace ns3 {

bool gVerbose = false;

void SendSocket(const char *path, int fd, const int magic_number) {
  LOG("Create Unix socket");
  int sock = socket(PF_UNIX, SOCK_DGRAM, 0);
  ABORT_IF(sock == -1, "Unable to open socket", 1);

  socklen_t clientAddrLen;
  struct sockaddr_un clientAddr;

  LOG("Decode address " << path);
  bool rc = ns3::StringToBuffer(path, (uint8_t *)&clientAddr, &clientAddrLen);
  ABORT_IF(rc == false, "Unable to decode path", 0);

  LOG("Connect");
  int status = connect(sock, (struct sockaddr *)&clientAddr, clientAddrLen);
  ABORT_IF(status == -1, "Unable to connect to emu device", 1);

  LOG("Connected");

  struct iovec iov;
  uint32_t magic = magic_number;
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
  ABORT_IF(len == -1, "Could not send socket back to emu net device", 1);

  LOG("sendmsg complete");
}

} // namespace ns3
