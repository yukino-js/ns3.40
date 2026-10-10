
#ifndef ERP_OFDM_PPDU_H
#define ERP_OFDM_PPDU_H

#include "ofdm-ppdu.h"

namespace ns3 {

class WifiPsdu;

class ErpOfdmPpdu : public OfdmPpdu {
public:
  ErpOfdmPpdu(Ptr<const WifiPsdu> psdu, const WifiTxVector &txVector,
              const WifiPhyOperatingChannel &channel, uint64_t uid);

  Ptr<WifiPpdu> Copy() const override;

private:
  void SetTxVectorFromLSigHeader(WifiTxVector &txVector,
                                 const LSigHeader &lSig) const override;
};

} // namespace ns3

#endif
