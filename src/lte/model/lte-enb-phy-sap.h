
#ifndef LTE_ENB_PHY_SAP_H
#define LTE_ENB_PHY_SAP_H

#include "ff-mac-common.h"
#include "ff-mac-sched-sap.h"

#include <ns3/packet.h>

namespace ns3 {

class LteControlMessage;

class LteEnbPhySapProvider {
public:
  virtual ~LteEnbPhySapProvider();

  virtual void SendMacPdu(Ptr<Packet> p) = 0;

  virtual void SendLteControlMessage(Ptr<LteControlMessage> msg) = 0;

  virtual uint8_t GetMacChTtiDelay() = 0;
};

class LteEnbPhySapUser {
public:
  virtual ~LteEnbPhySapUser();

  virtual void ReceivePhyPdu(Ptr<Packet> p) = 0;

  virtual void SubframeIndication(uint32_t frameNo, uint32_t subframeNo) = 0;

  virtual void ReceiveLteControlMessage(Ptr<LteControlMessage> msg) = 0;

  virtual void ReceiveRachPreamble(uint32_t prachId) = 0;

  virtual void
  UlCqiReport(FfMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi) = 0;

  virtual void UlInfoListElementHarqFeedback(UlInfoListElement_s params) = 0;

  virtual void DlInfoListElementHarqFeedback(DlInfoListElement_s params) = 0;
};

} // namespace ns3

#endif
