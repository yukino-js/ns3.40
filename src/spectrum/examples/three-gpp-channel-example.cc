

#include "ns3/channel-condition-model.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/core-module.h"
#include "ns3/lte-spectrum-value-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/net-device.h"
#include "ns3/node-container.h"
#include "ns3/node.h"
#include "ns3/simple-net-device.h"
#include "ns3/spectrum-signal-parameters.h"
#include "ns3/three-gpp-channel-model.h"
#include "ns3/three-gpp-propagation-loss-model.h"
#include "ns3/three-gpp-spectrum-propagation-loss-model.h"
#include "ns3/uniform-planar-array.h"

#include <fstream>

NS_LOG_COMPONENT_DEFINE("ThreeGppChannelExample");

using namespace ns3;

static Ptr<ThreeGppPropagationLossModel> m_propagationLossModel;
static Ptr<ThreeGppSpectrumPropagationLossModel> m_spectrumLossModel;

struct ComputeSnrParams {
  Ptr<MobilityModel> txMob;
  Ptr<MobilityModel> rxMob;
  double txPow;
  double noiseFigure;
  Ptr<PhasedArrayModel> txAntenna;
  Ptr<PhasedArrayModel> rxAntenna;
};

static void DoBeamforming(Ptr<NetDevice> thisDevice,
                          Ptr<PhasedArrayModel> thisAntenna,
                          Ptr<NetDevice> otherDevice) {
  Vector aPos =
      thisDevice->GetNode()->GetObject<MobilityModel>()->GetPosition();
  Vector bPos =
      otherDevice->GetNode()->GetObject<MobilityModel>()->GetPosition();

  Angles completeAngle(bPos, aPos);
  double hAngleRadian = completeAngle.GetAzimuth();

  double vAngleRadian = completeAngle.GetInclination();

  uint64_t totNoArrayElements = thisAntenna->GetNumberOfElements();
  PhasedArrayModel::ComplexVector antennaWeights(totNoArrayElements);

  double power = 1.0 / sqrt(totNoArrayElements);

  const double sinVAngleRadian = sin(vAngleRadian);
  const double cosVAngleRadian = cos(vAngleRadian);
  const double sinHAngleRadian = sin(hAngleRadian);
  const double cosHAngleRadian = cos(hAngleRadian);

  for (uint64_t ind = 0; ind < totNoArrayElements; ind++) {
    Vector loc = thisAntenna->GetElementLocation(ind);
    double phase =
        -2 * M_PI *
        (sinVAngleRadian * cosHAngleRadian * loc.x +
         sinVAngleRadian * sinHAngleRadian * loc.y + cosVAngleRadian * loc.z);
    antennaWeights[ind] = exp(std::complex<double>(0, phase)) * power;
  }

  thisAntenna->SetBeamformingVector(antennaWeights);
}

static void ComputeSnr(const ComputeSnrParams &params) {
  std::vector<int> activeRbs0(100);
  for (int i = 0; i < 100; i++) {
    activeRbs0[i] = i;
  }
  Ptr<SpectrumValue> txPsd =
      LteSpectrumValueHelper::CreateTxPowerSpectralDensity(
          2100, 100, params.txPow, activeRbs0);
  Ptr<SpectrumSignalParameters> txParams = Create<SpectrumSignalParameters>();
  txParams->psd = txPsd->Copy();
  NS_LOG_DEBUG("Average tx power " << 10 * log10(Sum(*txPsd) * 180e3) << " dB");

  Ptr<SpectrumValue> noisePsd =
      LteSpectrumValueHelper::CreateNoisePowerSpectralDensity(
          2100, 100, params.noiseFigure);
  NS_LOG_DEBUG("Average noise power " << 10 * log10(Sum(*noisePsd) * 180e3)
                                      << " dB");

  double propagationGainDb =
      m_propagationLossModel->CalcRxPower(0, params.txMob, params.rxMob);
  NS_LOG_DEBUG("Pathloss " << -propagationGainDb << " dB");
  double propagationGainLinear = std::pow(10.0, (propagationGainDb) / 10.0);
  *(txParams->psd) *= propagationGainLinear;

  NS_ASSERT_MSG(params.txAntenna, "params.txAntenna is nullptr!");
  NS_ASSERT_MSG(params.rxAntenna, "params.rxAntenna is nullptr!");

  Ptr<SpectrumValue> rxPsd = m_spectrumLossModel->CalcRxPowerSpectralDensity(
      txParams, params.txMob, params.rxMob, params.txAntenna, params.rxAntenna);
  NS_LOG_DEBUG("Average rx power " << 10 * log10(Sum(*rxPsd) * 180e3) << " dB");

  NS_LOG_DEBUG("Average SNR " << 10 * log10(Sum(*rxPsd) / Sum(*noisePsd))
                              << " dB");

  std::ofstream f;
  f.open("snr-trace.txt", std::ios::out | std::ios::app);
  f << Simulator::Now().GetSeconds() << " "
    << 10 * log10(Sum(*rxPsd) / Sum(*noisePsd)) << " " << propagationGainDb
    << std::endl;
  f.close();
}

int main(int argc, char *argv[]) {
  double frequency = 2125.0e6;
  double txPow = 49.0;
  double noiseFigure = 9.0;
  double distance = 10.0;
  uint32_t simTime = 1000;
  uint32_t timeRes = 10;
  std::string scenario = "UMa";

  Config::SetDefault("ns3::ThreeGppChannelModel::UpdatePeriod",
                     TimeValue(MilliSeconds(1)));
  Config::SetDefault("ns3::ThreeGppChannelConditionModel::UpdatePeriod",
                     TimeValue(MilliSeconds(0.0)));

  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(1);

  ObjectFactory propagationLossModelFactory;
  ObjectFactory channelConditionModelFactory;
  if (scenario == "RMa") {
    propagationLossModelFactory.SetTypeId(
        ThreeGppRmaPropagationLossModel::GetTypeId());
    channelConditionModelFactory.SetTypeId(
        ThreeGppRmaChannelConditionModel::GetTypeId());
  } else if (scenario == "UMa") {
    propagationLossModelFactory.SetTypeId(
        ThreeGppUmaPropagationLossModel::GetTypeId());
    channelConditionModelFactory.SetTypeId(
        ThreeGppUmaChannelConditionModel::GetTypeId());
  } else if (scenario == "UMi-StreetCanyon") {
    propagationLossModelFactory.SetTypeId(
        ThreeGppUmiStreetCanyonPropagationLossModel::GetTypeId());
    channelConditionModelFactory.SetTypeId(
        ThreeGppUmiStreetCanyonChannelConditionModel::GetTypeId());
  } else if (scenario == "InH-OfficeOpen") {
    propagationLossModelFactory.SetTypeId(
        ThreeGppIndoorOfficePropagationLossModel::GetTypeId());
    channelConditionModelFactory.SetTypeId(
        ThreeGppIndoorOpenOfficeChannelConditionModel::GetTypeId());
  } else if (scenario == "InH-OfficeMixed") {
    propagationLossModelFactory.SetTypeId(
        ThreeGppIndoorOfficePropagationLossModel::GetTypeId());
    channelConditionModelFactory.SetTypeId(
        ThreeGppIndoorMixedOfficeChannelConditionModel::GetTypeId());
  } else {
    NS_FATAL_ERROR("Unknown scenario");
  }

  m_propagationLossModel =
      propagationLossModelFactory.Create<ThreeGppPropagationLossModel>();
  m_propagationLossModel->SetAttribute("Frequency", DoubleValue(frequency));
  m_propagationLossModel->SetAttribute("ShadowingEnabled", BooleanValue(false));

  m_spectrumLossModel = CreateObject<ThreeGppSpectrumPropagationLossModel>();
  m_spectrumLossModel->SetChannelModelAttribute("Frequency",
                                                DoubleValue(frequency));
  m_spectrumLossModel->SetChannelModelAttribute("Scenario",
                                                StringValue(scenario));

  Ptr<ChannelConditionModel> condModel =
      channelConditionModelFactory.Create<ThreeGppChannelConditionModel>();
  m_spectrumLossModel->SetChannelModelAttribute("ChannelConditionModel",
                                                PointerValue(condModel));
  m_propagationLossModel->SetChannelConditionModel(condModel);

  NodeContainer nodes;
  nodes.Create(2);

  Ptr<SimpleNetDevice> txDev = CreateObject<SimpleNetDevice>();
  Ptr<SimpleNetDevice> rxDev = CreateObject<SimpleNetDevice>();

  nodes.Get(0)->AddDevice(txDev);
  txDev->SetNode(nodes.Get(0));
  nodes.Get(1)->AddDevice(rxDev);
  rxDev->SetNode(nodes.Get(1));

  Ptr<MobilityModel> txMob = CreateObject<ConstantPositionMobilityModel>();
  txMob->SetPosition(Vector(0.0, 0.0, 10.0));
  Ptr<MobilityModel> rxMob = CreateObject<ConstantPositionMobilityModel>();
  rxMob->SetPosition(Vector(distance, 0.0, 1.6));

  nodes.Get(0)->AggregateObject(txMob);
  nodes.Get(1)->AggregateObject(rxMob);

  Ptr<PhasedArrayModel> txAntenna =
      CreateObjectWithAttributes<UniformPlanarArray>(
          "NumColumns", UintegerValue(2), "NumRows", UintegerValue(2));
  Ptr<PhasedArrayModel> rxAntenna =
      CreateObjectWithAttributes<UniformPlanarArray>(
          "NumColumns", UintegerValue(2), "NumRows", UintegerValue(2));

  DoBeamforming(txDev, txAntenna, rxDev);
  DoBeamforming(rxDev, rxAntenna, txDev);

  for (int i = 0; i < floor(simTime / timeRes); i++) {
    ComputeSnrParams params{txMob,       rxMob,     txPow,
                            noiseFigure, txAntenna, rxAntenna};
    Simulator::Schedule(MilliSeconds(timeRes * i), &ComputeSnr, params);
  }

  Simulator::Run();
  Simulator::Destroy();
  return 0;
}
