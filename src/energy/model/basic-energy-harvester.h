
#ifndef BASIC_ENERGY_HARVESTER
#define BASIC_ENERGY_HARVESTER

#include "device-energy-model.h"
#include "energy-harvester.h"

#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"
#include "ns3/traced-value.h"

#include <iostream>

namespace ns3 {

class BasicEnergyHarvester : public EnergyHarvester {
public:
  static TypeId GetTypeId();

  BasicEnergyHarvester();

  BasicEnergyHarvester(Time updateInterval);

  ~BasicEnergyHarvester() override;

  void SetHarvestedPowerUpdateInterval(Time updateInterval);

  Time GetHarvestedPowerUpdateInterval() const;

  int64_t AssignStreams(int64_t stream);

private:
  void DoInitialize() override;

  void DoDispose() override;

  void CalculateHarvestedPower();

  double DoGetPower() const override;

  void UpdateHarvestedPower();

private:
  Ptr<RandomVariableStream> m_harvestablePower;

  TracedValue<double> m_harvestedPower;
  TracedValue<double> m_totalEnergyHarvestedJ;

  EventId m_energyHarvestingUpdateEvent;
  Time m_lastHarvestingUpdateTime;
  Time m_harvestedPowerUpdateInterval;
};

} // namespace ns3

#endif
