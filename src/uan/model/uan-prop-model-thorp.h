
#ifndef UAN_PROP_MODEL_THORP_H
#define UAN_PROP_MODEL_THORP_H

#include "uan-prop-model.h"

namespace ns3 {

class UanTxMode;

class UanPropModelThorp : public UanPropModel {
public:
  UanPropModelThorp();
  ~UanPropModelThorp() override;

  static TypeId GetTypeId();

  double GetPathLossDb(Ptr<MobilityModel> a, Ptr<MobilityModel> b,
                       UanTxMode mode) override;
  UanPdp GetPdp(Ptr<MobilityModel> a, Ptr<MobilityModel> b,
                UanTxMode mode) override;
  Time GetDelay(Ptr<MobilityModel> a, Ptr<MobilityModel> b,
                UanTxMode mode) override;

private:
  double GetAttenDbKyd(double freqKhz);
  double GetAttenDbKm(double freqKhz);

  double m_SpreadCoef;
};

} // namespace ns3

#endif
