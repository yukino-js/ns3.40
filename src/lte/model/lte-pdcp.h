
#ifndef LTE_PDCP_H
#define LTE_PDCP_H

#include "lte-pdcp-sap.h"
#include "lte-rlc-sap.h"

#include "ns3/object.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/traced-value.h"

namespace ns3 {

class LtePdcp : public Object {
  friend class LtePdcpSpecificLteRlcSapUser;
  friend class LtePdcpSpecificLtePdcpSapProvider<LtePdcp>;

public:
  LtePdcp();
  ~LtePdcp() override;
  static TypeId GetTypeId();
  void DoDispose() override;

  void SetRnti(uint16_t rnti);

  void SetLcId(uint8_t lcId);

  void SetLtePdcpSapUser(LtePdcpSapUser *s);

  LtePdcpSapProvider *GetLtePdcpSapProvider();

  void SetLteRlcSapProvider(LteRlcSapProvider *s);

  LteRlcSapUser *GetLteRlcSapUser();

  static const uint16_t MAX_PDCP_SN = 4096;

  struct Status {
    uint16_t txSn;
    uint16_t rxSn;
  };

  Status GetStatus() const;

  void SetStatus(Status s);

  typedef void (*PduTxTracedCallback)(uint16_t rnti, uint8_t lcid,
                                      uint32_t size);

  typedef void (*PduRxTracedCallback)(const uint16_t rnti, const uint8_t lcid,
                                      const uint32_t size,
                                      const uint64_t delay);

protected:
  virtual void
  DoTransmitPdcpSdu(LtePdcpSapProvider::TransmitPdcpSduParameters params);

  LtePdcpSapUser *m_pdcpSapUser;
  LtePdcpSapProvider *m_pdcpSapProvider;

  virtual void DoReceivePdu(Ptr<Packet> p);

  LteRlcSapUser *m_rlcSapUser;
  LteRlcSapProvider *m_rlcSapProvider;

  uint16_t m_rnti;
  uint8_t m_lcid;

  TracedCallback<uint16_t, uint8_t, uint32_t> m_txPdu;
  TracedCallback<uint16_t, uint8_t, uint32_t, uint64_t> m_rxPdu;

private:
  uint16_t m_txSequenceNumber;
  uint16_t m_rxSequenceNumber;

  static const uint16_t m_maxPdcpSn = 4095;
};

} // namespace ns3

#endif
