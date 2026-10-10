
#ifndef LTE_UE_PHY_SAP_H
#define LTE_UE_PHY_SAP_H

#include <ns3/packet.h>

namespace ns3 {

class LteControlMessage;

class LteUePhySapProvider {
public:
  virtual ~LteUePhySapProvider();

  virtual void SendMacPdu(Ptr<Packet> p) = 0;

  virtual void SendLteControlMessage(Ptr<LteControlMessage> msg) = 0;

  virtual void SendRachPreamble(uint32_t prachId, uint32_t raRnti) = 0;

  virtual void NotifyConnectionSuccessful() = 0;
};

class LteUePhySapUser {
public:
  virtual ~LteUePhySapUser();

  virtual void ReceivePhyPdu(Ptr<Packet> p) = 0;

  virtual void SubframeIndication(uint32_t frameNo, uint32_t subframeNo) = 0;

  virtual void ReceiveLteControlMessage(Ptr<LteControlMessage> msg) = 0;
};

} // namespace ns3

#endif
