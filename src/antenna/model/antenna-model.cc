
#include "antenna-model.h"

#include <ns3/log.h>

#include <cmath>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("AntennaModel");

NS_OBJECT_ENSURE_REGISTERED(AntennaModel);

AntennaModel::AntennaModel() {}

AntennaModel::~AntennaModel() {}

TypeId AntennaModel::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::AntennaModel").SetParent<Object>().SetGroupName("Antenna");
  return tid;
}

} // namespace ns3
