
#ifndef UAN_PROP_MODEL_IDEAL_H
#define UAN_PROP_MODEL_IDEAL_H

#include "uan-prop-model.h"

#include "ns3/mobility-model.h"
#include "ns3/nstime.h"

namespace ns3 {

class UanPropModelIdeal : public UanPropModel {
public:
  UanPropModelIdeal();
  ~UanPropModelIdeal() override;

  static TypeId GetTypeId();

  double GetPathLossDb(Ptr<MobilityModel> a, Ptr<MobilityModel> b,
                       UanTxMode mode) override;
  UanPdp GetPdp(Ptr<MobilityModel> a, Ptr<MobilityModel> b,
                UanTxMode mode) override;
  Time GetDelay(Ptr<MobilityModel> a, Ptr<MobilityModel> b,
                UanTxMode mode) override;
};

} // namespace ns3

#endif
