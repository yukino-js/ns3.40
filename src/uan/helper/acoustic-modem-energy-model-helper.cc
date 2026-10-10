
#include "acoustic-modem-energy-model-helper.h"

#include "ns3/basic-energy-source-helper.h"
#include "ns3/config.h"
#include "ns3/names.h"
#include "ns3/uan-net-device.h"
#include "ns3/uan-phy.h"

namespace ns3 {

AcousticModemEnergyModelHelper::AcousticModemEnergyModelHelper() {
  m_modemEnergy.SetTypeId("ns3::AcousticModemEnergyModel");
  m_depletionCallback.Nullify();
}

AcousticModemEnergyModelHelper::~AcousticModemEnergyModelHelper() {}

void AcousticModemEnergyModelHelper::Set(std::string name,
                                         const AttributeValue &v) {
  m_modemEnergy.Set(name, v);
}

void AcousticModemEnergyModelHelper::SetDepletionCallback(
    AcousticModemEnergyModel::AcousticModemEnergyDepletionCallback callback) {
  m_depletionCallback = callback;
}

Ptr<DeviceEnergyModel>
AcousticModemEnergyModelHelper::DoInstall(Ptr<NetDevice> device,
                                          Ptr<EnergySource> source) const {
  NS_ASSERT(device);
  NS_ASSERT(source);
  std::string deviceName = device->GetInstanceTypeId().GetName();
  if (deviceName != "ns3::UanNetDevice") {
    NS_FATAL_ERROR("NetDevice type is not UanNetDevice!");
  }
  Ptr<Node> node = device->GetNode();
  Ptr<AcousticModemEnergyModel> model =
      m_modemEnergy.Create<AcousticModemEnergyModel>();
  NS_ASSERT(model);
  model->SetNode(node);
  model->SetEnergySource(source);
  Ptr<UanNetDevice> uanDevice = DynamicCast<UanNetDevice>(device);
  Ptr<UanPhy> uanPhy = uanDevice->GetPhy();
  model->SetEnergyDepletionCallback(m_depletionCallback);
  source->AppendDeviceEnergyModel(model);
  source->SetNode(node);
  DeviceEnergyModel::ChangeStateCallback cb;
  cb = MakeCallback(&DeviceEnergyModel::ChangeState, model);
  uanPhy->SetEnergyModelCallback(cb);

  return model;
}

} // namespace ns3
