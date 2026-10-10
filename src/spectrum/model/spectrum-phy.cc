
#include "spectrum-phy.h"

#include "spectrum-channel.h"
#include "spectrum-value.h"

#include <ns3/log.h>
#include <ns3/mobility-model.h>
#include <ns3/net-device.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SpectrumPhy");

NS_OBJECT_ENSURE_REGISTERED(SpectrumPhy);

TypeId SpectrumPhy::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::SpectrumPhy").SetParent<Object>().SetGroupName("Spectrum");
  return tid;
}

SpectrumPhy::SpectrumPhy() { NS_LOG_FUNCTION(this); }

SpectrumPhy::~SpectrumPhy() { NS_LOG_FUNCTION(this); }
} // namespace ns3
