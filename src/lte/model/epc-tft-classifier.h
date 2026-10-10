
#ifndef EPC_TFT_CLASSIFIER_H
#define EPC_TFT_CLASSIFIER_H

#include "epc-tft.h"

#include "ns3/ptr.h"
#include "ns3/simple-ref-count.h"

#include <map>

namespace ns3 {

class EpcTft;
class Packet;

class EpcTftClassifier : public SimpleRefCount<EpcTftClassifier> {
public:
  EpcTftClassifier();

  void Add(Ptr<EpcTft> tft, uint32_t id);

  void Delete(uint32_t id);

  uint32_t Classify(Ptr<Packet> p, EpcTft::Direction direction,
                    uint16_t protocolNumber);

protected:
  std::map<uint32_t, Ptr<EpcTft>> m_tftMap;

  std::map<std::tuple<uint32_t, uint32_t, uint8_t, uint16_t>,
           std::pair<uint32_t, uint32_t>>
      m_classifiedIpv4Fragments;
};

} // namespace ns3

#endif
