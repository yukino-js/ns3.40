
#include "energy-source.h"

#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("EnergySource");

NS_OBJECT_ENSURE_REGISTERED(EnergySource);

TypeId EnergySource::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::EnergySource").SetParent<Object>().SetGroupName("Energy");
  return tid;
}

EnergySource::EnergySource() { NS_LOG_FUNCTION(this); }

EnergySource::~EnergySource() { NS_LOG_FUNCTION(this); }

void EnergySource::SetNode(Ptr<Node> node) {
  NS_LOG_FUNCTION(this);
  NS_ASSERT(node);
  m_node = node;
}

Ptr<Node> EnergySource::GetNode() const { return m_node; }

void EnergySource::AppendDeviceEnergyModel(
    Ptr<DeviceEnergyModel> deviceEnergyModelPtr) {
  NS_LOG_FUNCTION(this << deviceEnergyModelPtr);
  NS_ASSERT(deviceEnergyModelPtr);
  m_models.Add(deviceEnergyModelPtr);
}

DeviceEnergyModelContainer EnergySource::FindDeviceEnergyModels(TypeId tid) {
  NS_LOG_FUNCTION(this << tid);
  DeviceEnergyModelContainer container;
  DeviceEnergyModelContainer::Iterator i;
  for (i = m_models.Begin(); i != m_models.End(); i++) {
    if ((*i)->GetInstanceTypeId() == tid) {
      container.Add(*i);
    }
  }
  return container;
}

DeviceEnergyModelContainer
EnergySource::FindDeviceEnergyModels(std::string name) {
  NS_LOG_FUNCTION(this << name);
  DeviceEnergyModelContainer container;
  DeviceEnergyModelContainer::Iterator i;
  for (i = m_models.Begin(); i != m_models.End(); i++) {
    if ((*i)->GetInstanceTypeId().GetName() == name) {
      container.Add(*i);
    }
  }
  return container;
}

void EnergySource::InitializeDeviceModels() {
  NS_LOG_FUNCTION(this);
  DeviceEnergyModelContainer::Iterator i;
  for (i = m_models.Begin(); i != m_models.End(); i++) {
    (*i)->Initialize();
  }
}

void EnergySource::DisposeDeviceModels() {
  NS_LOG_FUNCTION(this);
  DeviceEnergyModelContainer::Iterator i;
  for (i = m_models.Begin(); i != m_models.End(); i++) {
    (*i)->Dispose();
  }
}

void EnergySource::ConnectEnergyHarvester(
    Ptr<EnergyHarvester> energyHarvesterPtr) {
  NS_LOG_FUNCTION(this << energyHarvesterPtr);
  NS_ASSERT(energyHarvesterPtr);
  m_harvesters.push_back(energyHarvesterPtr);
}

void EnergySource::DoDispose() {
  NS_LOG_FUNCTION(this);
  BreakDeviceEnergyModelRefCycle();
}

double EnergySource::CalculateTotalCurrent() {
  NS_LOG_FUNCTION(this);
  double totalCurrentA = 0.0;
  DeviceEnergyModelContainer::Iterator i;
  for (i = m_models.Begin(); i != m_models.End(); i++) {
    totalCurrentA += (*i)->GetCurrentA();
  }

  if (!m_harvesters.empty()) {
    double totalHarvestedPower = 0.0;

    for (auto harvester = m_harvesters.begin(); harvester != m_harvesters.end();
         harvester++) {
      totalHarvestedPower += (*harvester)->GetPower();
    }

    double supplyVoltage = GetSupplyVoltage();

    if (supplyVoltage != 0) {
      double currentHarvestersA = totalHarvestedPower / supplyVoltage;
      NS_LOG_DEBUG(" Total harvested power: " << totalHarvestedPower
                                              << "| Current from harvesters: "
                                              << currentHarvestersA);
      totalCurrentA -= currentHarvestersA;
    }
  }

  return totalCurrentA;
}

void EnergySource::NotifyEnergyDrained() {
  NS_LOG_FUNCTION(this);
  DeviceEnergyModelContainer::Iterator i;
  for (i = m_models.Begin(); i != m_models.End(); i++) {
    (*i)->HandleEnergyDepletion();
  }
}

void EnergySource::NotifyEnergyRecharged() {
  NS_LOG_FUNCTION(this);
  DeviceEnergyModelContainer::Iterator i;
  for (i = m_models.Begin(); i != m_models.End(); i++) {
    (*i)->HandleEnergyRecharged();
  }
}

void EnergySource::NotifyEnergyChanged() {
  NS_LOG_FUNCTION(this);
  DeviceEnergyModelContainer::Iterator i;
  for (i = m_models.Begin(); i != m_models.End(); i++) {
    (*i)->HandleEnergyChanged();
  }
}

void EnergySource::BreakDeviceEnergyModelRefCycle() {
  NS_LOG_FUNCTION(this);
  m_models.Clear();
  m_harvesters.clear();
  m_node = nullptr;
}

} // namespace ns3
