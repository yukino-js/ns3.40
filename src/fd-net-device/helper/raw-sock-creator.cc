
#include "creator-utils.h"

#include <arpa/inet.h>
#include <errno.h>
#include <iomanip>
#include <iostream>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define EMU_MAGIC 65867

using namespace ns3;

int main(int argc, char *argv[]) {
  int c;
  char *path = nullptr;

  opterr = 0;

  while ((c = getopt(argc, argv, "vp:")) != -1) {
    switch (c) {
    case 'v':
      gVerbose = true;
      break;
    case 'p':
      path = optarg;
      break;
    }
  }

  ABORT_IF(path == nullptr, "path is a required argument", 0);
  LOG("Provided path is \"" << path << "\"");
  LOG("Creating raw socket");
  int sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  ABORT_IF(sock == -1, "CreateSocket(): Unable to open raw socket", 1);

  SendSocket(path, sock, EMU_MAGIC);

  return 0;
}
