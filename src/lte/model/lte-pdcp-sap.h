
#ifndef LTE_PDCP_SAP_H
#define LTE_PDCP_SAP_H

#include "ns3/packet.h"

namespace ns3 {

class LtePdcpSapProvider {
public:
  virtual ~LtePdcpSapProvider();

  struct TransmitPdcpSduParameters {
    Ptr<Packet> pdcpSdu;
    uint16_t rnti;
    uint8_t lcid;
  };

  virtual void TransmitPdcpSdu(TransmitPdcpSduParameters params) = 0;
};

class LtePdcpSapUser {
public:
  virtual ~LtePdcpSapUser();

  struct ReceivePdcpSduParameters {
    Ptr<Packet> pdcpSdu;
    uint16_t rnti;
    uint8_t lcid;
  };

  virtual void ReceivePdcpSdu(ReceivePdcpSduParameters params) = 0;
};

template <class C>
class LtePdcpSpecificLtePdcpSapProvider : public LtePdcpSapProvider {
public:
  LtePdcpSpecificLtePdcpSapProvider(C *pdcp);

  LtePdcpSpecificLtePdcpSapProvider() = delete;

  void TransmitPdcpSdu(TransmitPdcpSduParameters params) override;

private:
  C *m_pdcp;
};

template <class C>
LtePdcpSpecificLtePdcpSapProvider<C>::LtePdcpSpecificLtePdcpSapProvider(C *pdcp)
    : m_pdcp(pdcp) {}

template <class C>
void LtePdcpSpecificLtePdcpSapProvider<C>::TransmitPdcpSdu(
    TransmitPdcpSduParameters params) {
  m_pdcp->DoTransmitPdcpSdu(params);
}

template <class C> class LtePdcpSpecificLtePdcpSapUser : public LtePdcpSapUser {
public:
  LtePdcpSpecificLtePdcpSapUser(C *rrc);

  LtePdcpSpecificLtePdcpSapUser() = delete;

  void ReceivePdcpSdu(ReceivePdcpSduParameters params) override;

private:
  C *m_rrc;
};

template <class C>
LtePdcpSpecificLtePdcpSapUser<C>::LtePdcpSpecificLtePdcpSapUser(C *rrc)
    : m_rrc(rrc) {}

template <class C>
void LtePdcpSpecificLtePdcpSapUser<C>::ReceivePdcpSdu(
    ReceivePdcpSduParameters params) {
  m_rrc->DoReceivePdcpSdu(params);
}

} // namespace ns3

#endif
