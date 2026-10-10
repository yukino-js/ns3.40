
#ifndef HT_PPDU_H
#define HT_PPDU_H

#include "ns3/ofdm-ppdu.h"

namespace ns3 {

class WifiPsdu;

class HtPpdu : public OfdmPpdu {
public:
  class HtSigHeader {
  public:
    HtSigHeader();

    void SetMcs(uint8_t mcs);
    uint8_t GetMcs() const;
    void SetChannelWidth(uint16_t channelWidth);
    uint16_t GetChannelWidth() const;
    void SetAggregation(bool aggregation);
    bool GetAggregation() const;
    void SetShortGuardInterval(bool sgi);
    bool GetShortGuardInterval() const;
    void SetHtLength(uint16_t length);
    uint16_t GetHtLength() const;

  private:
    uint8_t m_mcs;
    uint8_t m_cbw20_40;
    uint16_t m_htLength;
    uint8_t m_aggregation;
    uint8_t m_sgi;
  };

  HtPpdu(Ptr<const WifiPsdu> psdu, const WifiTxVector &txVector,
         const WifiPhyOperatingChannel &channel, Time ppduDuration,
         uint64_t uid);

  Time GetTxDuration() const override;
  Ptr<WifiPpdu> Copy() const override;

private:
  WifiTxVector DoGetTxVector() const override;

  void SetPhyHeaders(const WifiTxVector &txVector, Time ppduDuration,
                     std::size_t psduSize);

  virtual void SetLSigHeader(LSigHeader &lSig, Time ppduDuration) const;

  void SetHtSigHeader(HtSigHeader &htSig, const WifiTxVector &txVector,
                      std::size_t psduSize) const;

  void SetTxVectorFromPhyHeaders(WifiTxVector &txVector, const LSigHeader &lSig,
                                 const HtSigHeader &htSig) const;

  HtSigHeader m_htSig;
};

} // namespace ns3

#endif
