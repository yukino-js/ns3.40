
#include "wifi-radio-energy-model-helper.h"

#include "ns3/wifi-net-device.h"
#include "ns3/wifi-phy.h"
#include "ns3/wifi-tx-current-model.h"

namespace ns3 {

WifiRadioEnergyModelHelper::WifiRadioEnergyModelHelper() {
  m_radioEnergy.SetTypeId("ns3::WifiRadioEnergyModel");
  m_depletionCallback.Nullify();
  m_rechargedCallback.Nullify();
}

WifiRadioEnergyModelHelper::~WifiRadioEnergyModelHelper() {}

void WifiRadioEnergyModelHelper::Set(std::string name,
                                     const AttributeValue &v) {
  m_radioEnergy.Set(name, v);
}

void WifiRadioEnergyModelHelper::SetDepletionCallback(
    WifiRadioEnergyModel::WifiRadioEnergyDepletionCallback callback) {
  m_depletionCallback = callback;
}

void WifiRadioEnergyModelHelper::SetRechargedCallback(
    WifiRadioEnergyModel::WifiRadioEnergyRechargedCallback callback) {
  m_rechargedCallback = callback;
}

Ptr<DeviceEnergyModel>
WifiRadioEnergyModelHelper::DoInstall(Ptr<NetDevice> device,
                                      Ptr<EnergySource> source) const {
  NS_ASSERT(device);
  NS_ASSERT(source);
  std::string deviceName = device->GetInstanceTypeId().GetName();
  if (deviceName != "ns3::WifiNetDevice") {
    NS_FATAL_ERROR("NetDevice type is not WifiNetDevice!");
  }
  Ptr<Node> node = device->GetNode();
  Ptr<WifiRadioEnergyModel> model =
      m_radioEnergy.Create()->GetObject<WifiRadioEnergyModel>();
  NS_ASSERT(model);

  Ptr<WifiNetDevice> wifiDevice = DynamicCast<WifiNetDevice>(device);
  Ptr<WifiPhy> wifiPhy = wifiDevice->GetPhy();
  wifiPhy->SetWifiRadioEnergyModel(model);
  if (m_depletionCallback.IsNull()) {
    model->SetEnergyDepletionCallback(
        MakeCallback(&WifiPhy::SetOffMode, wifiPhy));
  } else {
    model->SetEnergyDepletionCallback(m_depletionCallback);
  }
  if (m_rechargedCallback.IsNull()) {
    model->SetEnergyRechargedCallback(
        MakeCallback(&WifiPhy::ResumeFromOff, wifiPhy));
  } else {
    model->SetEnergyRechargedCallback(m_rechargedCallback);
  }
  source->AppendDeviceEnergyModel(model);
  model->SetEnergySource(source);
  wifiPhy->RegisterListener(model->GetPhyListener());
  if (m_txCurrentModel.GetTypeId().GetUid()) {
    Ptr<WifiTxCurrentModel> txcurrent =
        m_txCurrentModel.Create<WifiTxCurrentModel>();
    model->SetTxCurrentModel(txcurrent);
  }
  return model;
}

} // namespace ns3
