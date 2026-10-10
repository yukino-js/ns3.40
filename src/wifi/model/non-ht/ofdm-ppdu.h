
#ifndef OFDM_PPDU_H
#define OFDM_PPDU_H

#include "ns3/wifi-phy-band.h"
#include "ns3/wifi-ppdu.h"

namespace ns3 {

class WifiPsdu;

class OfdmPpdu : public WifiPpdu {
public:
  class LSigHeader {
  public:
    LSigHeader();

    void SetRate(uint64_t rate, uint16_t channelWidth = 20);
    uint64_t GetRate(uint16_t channelWidth = 20) const;
    void SetLength(uint16_t length);
    uint16_t GetLength() const;

  private:
    uint8_t m_rate;
    uint16_t m_length;
  };

  OfdmPpdu(Ptr<const WifiPsdu> psdu, const WifiTxVector &txVector,
           const WifiPhyOperatingChannel &channel, uint64_t uid,
           bool instantiateLSig = true);

  Time GetTxDuration() const override;
  Ptr<WifiPpdu> Copy() const override;

protected:
  LSigHeader m_lSig;

private:
  WifiTxVector DoGetTxVector() const override;

  void SetPhyHeaders(const WifiTxVector &txVector, std::size_t psduSize);

  void SetLSigHeader(LSigHeader &lSig, const WifiTxVector &txVector,
                     std::size_t psduSize) const;

  virtual void SetTxVectorFromLSigHeader(WifiTxVector &txVector,
                                         const LSigHeader &lSig) const;

  uint16_t m_channelWidth;
};

} // namespace ns3

#endif
