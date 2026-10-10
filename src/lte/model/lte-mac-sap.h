
#ifndef LTE_MAC_SAP_H
#define LTE_MAC_SAP_H

#include <ns3/packet.h>

namespace ns3 {

class LteMacSapProvider {
public:
  virtual ~LteMacSapProvider();

  struct TransmitPduParameters {
    Ptr<Packet> pdu;
    uint16_t rnti;
    uint8_t lcid;
    uint8_t layer;
    uint8_t harqProcessId;
    uint8_t componentCarrierId;
  };

  virtual void TransmitPdu(TransmitPduParameters params) = 0;

  struct ReportBufferStatusParameters {
    uint16_t rnti;
    uint8_t lcid;
    uint32_t txQueueSize;
    uint16_t txQueueHolDelay;
    uint32_t retxQueueSize;
    uint16_t retxQueueHolDelay;
    uint16_t statusPduSize;
  };

  virtual void ReportBufferStatus(ReportBufferStatusParameters params) = 0;
};

class LteMacSapUser {
public:
  virtual ~LteMacSapUser();

  struct TxOpportunityParameters {
    TxOpportunityParameters(uint32_t bytes, uint8_t layer, uint8_t harqId,
                            uint8_t ccId, uint16_t rnti, uint8_t lcId) {
      this->bytes = bytes;
      this->layer = layer;
      this->harqId = harqId;
      this->componentCarrierId = ccId;
      this->rnti = rnti;
      this->lcid = lcId;
    }

    TxOpportunityParameters() {}

    uint32_t bytes;
    uint8_t layer;
    uint8_t harqId;
    uint8_t componentCarrierId;
    uint16_t rnti;
    uint8_t lcid;
  };

  virtual void NotifyTxOpportunity(TxOpportunityParameters params) = 0;

  virtual void NotifyHarqDeliveryFailure() = 0;

  struct ReceivePduParameters {
    ReceivePduParameters() {}

    ReceivePduParameters(const Ptr<Packet> &p, uint16_t rnti, uint8_t lcid) {
      this->p = p;
      this->rnti = rnti;
      this->lcid = lcid;
    }

    Ptr<Packet> p;
    uint16_t rnti;
    uint8_t lcid;
  };

  virtual void ReceivePdu(ReceivePduParameters params) = 0;
};

template <class C>
class EnbMacMemberLteMacSapProvider : public LteMacSapProvider {
public:
  EnbMacMemberLteMacSapProvider(C *mac);

  void TransmitPdu(TransmitPduParameters params) override;
  void ReportBufferStatus(ReportBufferStatusParameters params) override;

private:
  C *m_mac;
};

template <class C>
EnbMacMemberLteMacSapProvider<C>::EnbMacMemberLteMacSapProvider(C *mac)
    : m_mac(mac) {}

template <class C>
void EnbMacMemberLteMacSapProvider<C>::TransmitPdu(
    TransmitPduParameters params) {
  m_mac->DoTransmitPdu(params);
}

template <class C>
void EnbMacMemberLteMacSapProvider<C>::ReportBufferStatus(
    ReportBufferStatusParameters params) {
  m_mac->DoReportBufferStatus(params);
}

} // namespace ns3

#endif
