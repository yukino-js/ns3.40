
#ifndef VHT_PPDU_H
#define VHT_PPDU_H

#include "ns3/ofdm-ppdu.h"
#include "ns3/wifi-phy-operating-channel.h"

namespace ns3 {

class WifiPsdu;

class VhtPpdu : public OfdmPpdu {
public:
  class VhtSigHeader {
  public:
    VhtSigHeader();

    void SetMuFlag(bool mu);

    void SetChannelWidth(uint16_t channelWidth);
    uint16_t GetChannelWidth() const;
    void SetNStreams(uint8_t nStreams);
    uint8_t GetNStreams() const;

    void SetShortGuardInterval(bool sgi);
    bool GetShortGuardInterval() const;
    void SetShortGuardIntervalDisambiguation(bool disambiguation);
    bool GetShortGuardIntervalDisambiguation() const;
    void SetSuMcs(uint8_t mcs);
    uint8_t GetSuMcs() const;

  private:
    uint8_t m_bw;
    uint8_t m_nsts;

    uint8_t m_sgi;
    uint8_t m_sgi_disambiguation;
    uint8_t m_suMcs;

    bool m_mu;
  };

  VhtPpdu(Ptr<const WifiPsdu> psdu, const WifiTxVector &txVector,
          const WifiPhyOperatingChannel &channel, Time ppduDuration,
          uint64_t uid);

  Time GetTxDuration() const override;
  Ptr<WifiPpdu> Copy() const override;
  WifiPpduType GetType() const override;

private:
  WifiTxVector DoGetTxVector() const override;

  virtual void SetPhyHeaders(const WifiTxVector &txVector, Time ppduDuration);

  virtual void SetLSigHeader(LSigHeader &lSig, Time ppduDuration) const;

  void SetVhtSigHeader(VhtSigHeader &vhtSig, const WifiTxVector &txVector,
                       Time ppduDuration) const;

  void SetTxVectorFromPhyHeaders(WifiTxVector &txVector, const LSigHeader &lSig,
                                 const VhtSigHeader &vhtSig) const;

  VhtSigHeader m_vhtSig;
};

} // namespace ns3

#endif
