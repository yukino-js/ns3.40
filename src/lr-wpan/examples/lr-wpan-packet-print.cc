#include <ns3/core-module.h>
#include <ns3/lr-wpan-module.h>

#include <iostream>

using namespace ns3;

int main(int argc, char *argv[]) {
  Packet::EnablePrinting();
  Packet::EnableChecking();
  LrWpanMacHeader macHdr(LrWpanMacHeader::LRWPAN_MAC_BEACON, 0);
  macHdr.SetSrcAddrMode(2);
  macHdr.SetDstAddrMode(0);

  uint16_t srcPanId = 100;
  Mac16Address srcWpanAddr("00:11");

  macHdr.SetSrcAddrFields(srcPanId, srcWpanAddr);

  LrWpanMacTrailer macTrailer;

  Ptr<Packet> p = Create<Packet>(20);

  p->AddHeader(macHdr);
  p->AddTrailer(macTrailer);

  p->Print(std::cout);
  std::cout << std::endl;
  return 0;
}
