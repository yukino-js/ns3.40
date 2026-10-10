
#include "wifi-bandwidth-filter.h"

#include "spectrum-wifi-phy.h"
#include "wifi-psdu.h"
#include "wifi-spectrum-phy-interface.h"
#include "wifi-spectrum-signal-parameters.h"

#include <ns3/boolean.h>
#include <ns3/spectrum-transmit-filter.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("WifiBandwidthFilter");

NS_OBJECT_ENSURE_REGISTERED(WifiBandwidthFilter);

WifiBandwidthFilter::WifiBandwidthFilter() { NS_LOG_FUNCTION(this); }

TypeId WifiBandwidthFilter::GetTypeId() {
  static TypeId tid = TypeId("ns3::WifiBandwidthFilter")
                          .SetParent<SpectrumTransmitFilter>()
                          .SetGroupName("Wifi")
                          .AddConstructor<WifiBandwidthFilter>();
  return tid;
}

bool WifiBandwidthFilter::DoFilter(Ptr<const SpectrumSignalParameters> params,
                                   Ptr<const SpectrumPhy> receiverPhy) {
  NS_LOG_FUNCTION(this << params);

  auto wifiRxParams = DynamicCast<const WifiSpectrumSignalParameters>(params);
  if (!wifiRxParams) {
    NS_LOG_DEBUG("Received a non Wi-Fi signal: do not filter");
    return false;
  }

  auto interface = DynamicCast<const WifiSpectrumPhyInterface>(receiverPhy);
  if (!interface) {
    NS_LOG_DEBUG("Sending a Wi-Fi signal to a non Wi-Fi device; do not filter");
    return false;
  }

  auto wifiPhy = interface->GetSpectrumWifiPhy();
  NS_ASSERT_MSG(wifiPhy, "WifiPhy should be valid if "
                         "WifiSpectrumSignalParameters was found and sending "
                         "to a WifiSpectrumPhyInterface");

  BooleanValue trackSignalsInactiveInterfaces;
  wifiPhy->GetAttribute("TrackSignalsFromInactiveInterfaces",
                        trackSignalsInactiveInterfaces);

  NS_ASSERT_MSG(trackSignalsInactiveInterfaces.Get() ||
                    (interface == wifiPhy->GetCurrentInterface()),
                "DoFilter should not be called for an inactive interface if "
                "SpectrumWifiPhy::TrackSignalsFromInactiveInterfaces attribute "
                "is not enabled");

  NS_ASSERT((interface != wifiPhy->GetCurrentInterface()) ||
            (wifiPhy->GetOperatingChannel().GetFrequency() ==
             interface->GetCenterFrequency()));
  NS_ASSERT((interface != wifiPhy->GetCurrentInterface()) ||
            (wifiPhy->GetOperatingChannel().GetWidth() ==
             interface->GetChannelWidth()));

  const auto rxCenterFreq = wifiRxParams->ppdu->GetTxCenterFreq();
  const auto rxWidth = wifiRxParams->ppdu->GetTxVector().GetChannelWidth();
  const auto guardBandwidth = wifiPhy->GetGuardBandwidth(rxWidth);
  const auto operatingFrequency = interface->GetCenterFrequency();
  const auto operatingChannelWidth = interface->GetChannelWidth();

  const auto rxMinFreq = rxCenterFreq - rxWidth / 2 - guardBandwidth;
  const auto rxMaxFreq = rxCenterFreq + rxWidth / 2 + guardBandwidth;

  const auto channelMinFreq = operatingFrequency - operatingChannelWidth / 2;
  const auto channelMaxFreq = operatingFrequency + operatingChannelWidth / 2;

  auto filter = (rxMinFreq >= channelMaxFreq || rxMaxFreq <= channelMinFreq);
  NS_LOG_DEBUG("Returning " << filter);
  return filter;
}

} // namespace ns3
