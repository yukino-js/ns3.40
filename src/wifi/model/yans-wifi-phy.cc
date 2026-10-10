
#include "yans-wifi-phy.h"

#include "interference-helper.h"
#include "yans-wifi-channel.h"

#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("YansWifiPhy");

NS_OBJECT_ENSURE_REGISTERED(YansWifiPhy);

TypeId YansWifiPhy::GetTypeId() {
  static TypeId tid = TypeId("ns3::YansWifiPhy")
                          .SetParent<WifiPhy>()
                          .SetGroupName("Wifi")
                          .AddConstructor<YansWifiPhy>();
  return tid;
}

YansWifiPhy::YansWifiPhy() { NS_LOG_FUNCTION(this); }

void YansWifiPhy::SetInterferenceHelper(const Ptr<InterferenceHelper> helper) {
  WifiPhy::SetInterferenceHelper(helper);
  m_interference->AddBand({{0, 0}, {0, 0}});
}

YansWifiPhy::~YansWifiPhy() { NS_LOG_FUNCTION(this); }

void YansWifiPhy::DoDispose() {
  NS_LOG_FUNCTION(this);
  m_channel = nullptr;
  WifiPhy::DoDispose();
}

Ptr<Channel> YansWifiPhy::GetChannel() const { return m_channel; }

void YansWifiPhy::SetChannel(const Ptr<YansWifiChannel> channel) {
  NS_LOG_FUNCTION(this << channel);
  m_channel = channel;
  m_channel->Add(this);
}

void YansWifiPhy::StartTx(Ptr<const WifiPpdu> ppdu) {
  NS_LOG_FUNCTION(this << ppdu);
  NS_LOG_DEBUG("Start transmission: signal power before antenna gain="
               << GetPowerDbm(ppdu->GetTxVector().GetTxPowerLevel()) << "dBm");
  m_channel->Send(this, ppdu, GetTxPowerForTransmission(ppdu) + GetTxGain());
}

uint16_t YansWifiPhy::GetGuardBandwidth(uint16_t currentChannelWidth) const {
  NS_ABORT_MSG("Guard bandwidth not relevant for Yans");
  return 0;
}

std::tuple<double, double, double>
YansWifiPhy::GetTxMaskRejectionParams() const {
  NS_ABORT_MSG("Tx mask rejection params not relevant for Yans");
  return std::make_tuple(0.0, 0.0, 0.0);
}

WifiSpectrumBandInfo YansWifiPhy::GetBand(uint16_t, uint8_t) {
  return {{0, 0}, {0, 0}};
}

FrequencyRange YansWifiPhy::GetCurrentFrequencyRange() const {
  return WHOLE_WIFI_SPECTRUM;
}

WifiSpectrumBandFrequencies YansWifiPhy::ConvertIndicesToFrequencies(
    const WifiSpectrumBandIndices &) const {
  return {0, 0};
}

} // namespace ns3
