
#ifndef WIFI_TX_CURRENT_MODEL_H
#define WIFI_TX_CURRENT_MODEL_H

#include "ns3/object.h"

namespace ns3 {

class WifiTxCurrentModel : public Object {
public:
  static TypeId GetTypeId();

  WifiTxCurrentModel();
  ~WifiTxCurrentModel() override;

  virtual double CalcTxCurrent(double txPowerDbm) const = 0;
};

class LinearWifiTxCurrentModel : public WifiTxCurrentModel {
public:
  static TypeId GetTypeId();

  LinearWifiTxCurrentModel();
  ~LinearWifiTxCurrentModel() override;

  double CalcTxCurrent(double txPowerDbm) const override;

private:
  double m_eta;
  double m_voltage;
  double m_idleCurrent;
};

} // namespace ns3

#endif
