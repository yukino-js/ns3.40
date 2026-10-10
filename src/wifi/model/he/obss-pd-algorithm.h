
#ifndef OBSS_PD_ALGORITHM_H
#define OBSS_PD_ALGORITHM_H

#include "he-configuration.h"

#include "ns3/object.h"
#include "ns3/traced-callback.h"

namespace ns3 {

struct HeSigAParameters;

class WifiNetDevice;

class ObssPdAlgorithm : public Object {
public:
  static TypeId GetTypeId();

  virtual void ConnectWifiNetDevice(const Ptr<WifiNetDevice> device);

  void ResetPhy(HeSigAParameters params);

  virtual void ReceiveHeSigA(HeSigAParameters params) = 0;

  typedef void (*ResetTracedCallback)(uint8_t bssColor, double rssiDbm,
                                      bool powerRestricted,
                                      double txPowerMaxDbmSiso,
                                      double txPowerMaxDbmMimo);

  void SetObssPdLevel(double level);
  double GetObssPdLevel() const;

protected:
  void DoDispose() override;

  Ptr<WifiNetDevice> m_device;

private:
  double m_obssPdLevel;
  double m_obssPdLevelMin;
  double m_obssPdLevelMax;
  double m_txPowerRefSiso;
  double m_txPowerRefMimo;

  TracedCallback<uint8_t, double, bool, double, double> m_resetEvent;
};

} // namespace ns3

#endif
