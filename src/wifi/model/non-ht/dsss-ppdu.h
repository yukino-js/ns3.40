
#ifndef DSSS_PPDU_H
#define DSSS_PPDU_H

#include "ns3/wifi-ppdu.h"

namespace ns3 {

class WifiPsdu;

class DsssPpdu : public WifiPpdu {
public:
  class DsssSigHeader {
  public:
    DsssSigHeader();

    void SetRate(uint64_t rate);
    uint64_t GetRate() const;
    void SetLength(uint16_t length);
    uint16_t GetLength() const;

  private:
    uint8_t m_rate;
    uint16_t m_length;
  };

  DsssPpdu(Ptr<const WifiPsdu> psdu, const WifiTxVector &txVector,
           const WifiPhyOperatingChannel &channel, Time ppduDuration,
           uint64_t uid);

  Time GetTxDuration() const override;
  Ptr<WifiPpdu> Copy() const override;

private:
  WifiTxVector DoGetTxVector() const override;

  void SetPhyHeaders(const WifiTxVector &txVector, Time ppduDuration);

  void SetDsssHeader(DsssSigHeader &dsssSig, const WifiTxVector &txVector,
                     Time ppduDuration) const;

  virtual void SetTxVectorFromDsssHeader(WifiTxVector &txVector,
                                         const DsssSigHeader &dsssSig) const;

  DsssSigHeader m_dsssSig;
};

} // namespace ns3

#endif
