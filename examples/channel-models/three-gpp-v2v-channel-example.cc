

#include "ns3/buildings-module.h"
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/spectrum-signal-parameters.h"
#include "ns3/three-gpp-channel-model.h"
#include "ns3/three-gpp-spectrum-propagation-loss-model.h"
#include "ns3/three-gpp-v2v-propagation-loss-model.h"
#include "ns3/uniform-planar-array.h"

#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ThreeGppV2vChannelExample");

static Ptr<ThreeGppPropagationLossModel> m_propagationLossModel;
static Ptr<ThreeGppSpectrumPropagationLossModel> m_spectrumLossModel;
static Ptr<ChannelConditionModel> m_condModel;

struct ComputeSnrParams {
  Ptr<MobilityModel> txMob;
  Ptr<MobilityModel> rxMob;
  Ptr<SpectrumSignalParameters> txParams;
  double noiseFigure;
  Ptr<PhasedArrayModel> txAntenna;
  Ptr<PhasedArrayModel> rxAntenna;
};

static void DoBeamforming(Ptr<NetDevice> thisDevice,
                          Ptr<PhasedArrayModel> thisAntenna,
                          Ptr<NetDevice> otherDevice) {
  PhasedArrayModel::ComplexVector antennaWeights;

  Vector aPos =
      thisDevice->GetNode()->GetObject<MobilityModel>()->GetPosition();
  Vector bPos =
      otherDevice->GetNode()->GetObject<MobilityModel>()->GetPosition();

  Angles completeAngle(bPos, aPos);

  PhasedArrayModel::ComplexVector bf =
      thisAntenna->GetBeamformingVector(completeAngle);
  thisAntenna->SetBeamformingVector(bf);
}

static void ComputeSnr(const ComputeSnrParams &params) {
  Ptr<ChannelCondition> cond =
      m_condModel->GetChannelCondition(params.txMob, params.rxMob);

  double propagationGainDb =
      m_propagationLossModel->CalcRxPower(0, params.txMob, params.rxMob);
  NS_LOG_DEBUG("Pathloss " << -propagationGainDb << " dB");
  double propagationGainLinear = std::pow(10.0, (propagationGainDb) / 10.0);
  *(params.txParams->psd) *= propagationGainLinear;

  Ptr<SpectrumValue> rxPsd = m_spectrumLossModel->CalcRxPowerSpectralDensity(
      params.txParams, params.txMob, params.rxMob, params.txAntenna,
      params.rxAntenna);
  NS_LOG_DEBUG("Average rx power " << 10 * log10(Sum(*rxPsd) * 180e3) << " dB");

  const double kT_dBm_Hz = -174.0;
  double kT_W_Hz = std::pow(10.0, (kT_dBm_Hz - 30) / 10.0);
  double noiseFigureLinear = std::pow(10.0, params.noiseFigure / 10.0);
  double noisePowerSpectralDensity = kT_W_Hz * noiseFigureLinear;
  Ptr<SpectrumValue> noisePsd =
      Create<SpectrumValue>(params.txParams->psd->GetSpectrumModel());
  (*noisePsd) = noisePowerSpectralDensity;

  NS_LOG_DEBUG("Average SNR " << 10 * log10(Sum(*rxPsd) / Sum(*noisePsd))
                              << " dB");

  std::ofstream f;
  f.open("example-output.txt", std::ios::out | std::ios::app);
  f << Simulator::Now().GetSeconds() << " " << params.txMob->GetPosition().x
    << " " << params.txMob->GetPosition().y << " "
    << params.rxMob->GetPosition().x << " " << params.rxMob->GetPosition().y
    << " " << cond->GetLosCondition() << " "
    << 10 * log10(Sum(*rxPsd) / Sum(*noisePsd)) << " " << -propagationGainDb
    << std::endl;
  f.close();
}

void PrintGnuplottableBuildingListToFile(std::string filename) {
  std::ofstream outFile;
  outFile.open(filename, std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open()) {
    NS_LOG_ERROR("Can't open file " << filename);
    return;
  }
  uint32_t index = 0;
  for (auto it = BuildingList::Begin(); it != BuildingList::End(); ++it) {
    ++index;
    Box box = (*it)->GetBoundaries();
    outFile << "set object " << index << " rect from " << box.xMin << ","
            << box.yMin << " to " << box.xMax << "," << box.yMax << std::endl;
  }
}

int main(int argc, char *argv[]) {
  double frequency = 28.0e9;
  double txPow_dbm = 30.0;
  double noiseFigure = 9.0;
  Time simTime = Seconds(40);
  Time timeRes = MilliSeconds(10);
  std::string scenario = "V2V-Urban";
  double vScatt = 0;
  double subCarrierSpacing = 60e3;
  uint32_t numRb = 275;

  CommandLine cmd(__FILE__);
  cmd.AddValue("frequency", "operating frequency in Hz", frequency);
  cmd.AddValue("txPow", "tx power in dBm", txPow_dbm);
  cmd.AddValue("noiseFigure", "noise figure in dB", noiseFigure);
  cmd.AddValue("scenario",
               "3GPP propagation scenario, V2V-Urban or V2V-Highway", scenario);
  cmd.Parse(argc, argv);

  NodeContainer nodes;
  nodes.Create(2);

  Ptr<SimpleNetDevice> txDev = CreateObject<SimpleNetDevice>();
  Ptr<SimpleNetDevice> rxDev = CreateObject<SimpleNetDevice>();

  nodes.Get(0)->AddDevice(txDev);
  txDev->SetNode(nodes.Get(0));
  nodes.Get(1)->AddDevice(rxDev);
  rxDev->SetNode(nodes.Get(1));

  Ptr<PhasedArrayModel> txAntenna =
      CreateObjectWithAttributes<UniformPlanarArray>(
          "NumColumns", UintegerValue(2), "NumRows", UintegerValue(2),
          "BearingAngle", DoubleValue(-M_PI / 2));
  Ptr<PhasedArrayModel> rxAntenna =
      CreateObjectWithAttributes<UniformPlanarArray>(
          "NumColumns", UintegerValue(2), "NumRows", UintegerValue(2),
          "BearingAngle", DoubleValue(M_PI / 2));

  Ptr<MobilityModel> txMob;
  Ptr<MobilityModel> rxMob;
  if (scenario == "V2V-Urban") {
    vScatt = 60 / 3.6;

    double buildingSizeX = 250 - 3.5 * 2 - 3;
    double buildingSizeY = 433 - 3.5 * 2 - 3;
    double streetWidth = 20;
    double buildingHeight = 10;
    uint32_t numBuildingsX = 2;
    uint32_t numBuildingsY = 2;
    double maxAxisX = (buildingSizeX + streetWidth) * numBuildingsX;
    double maxAxisY = (buildingSizeY + streetWidth) * numBuildingsY;

    std::vector<Ptr<Building>> buildingVector;
    for (uint32_t buildingIdX = 0; buildingIdX < numBuildingsX; ++buildingIdX) {
      for (uint32_t buildingIdY = 0; buildingIdY < numBuildingsY;
           ++buildingIdY) {
        Ptr<Building> building;
        building = CreateObject<Building>();

        building->SetBoundaries(
            Box(buildingIdX * (buildingSizeX + streetWidth),
                buildingIdX * (buildingSizeX + streetWidth) + buildingSizeX,
                buildingIdY * (buildingSizeY + streetWidth),
                buildingIdY * (buildingSizeY + streetWidth) + buildingSizeY,
                0.0, buildingHeight));
        building->SetNRoomsX(1);
        building->SetNRoomsY(1);
        building->SetNFloors(1);
        buildingVector.push_back(building);
      }
    }

    double vTx = vScatt;
    double vRx = vScatt / 2;
    txMob = CreateObject<WaypointMobilityModel>();
    rxMob = CreateObject<WaypointMobilityModel>();
    Time nextWaypoint = Seconds(0.0);
    txMob->GetObject<WaypointMobilityModel>()->AddWaypoint(Waypoint(
        nextWaypoint, Vector(maxAxisX / 2 - streetWidth / 2, 1.0, 1.5)));
    nextWaypoint += Seconds((maxAxisY - streetWidth) / 2 / vTx);
    txMob->GetObject<WaypointMobilityModel>()->AddWaypoint(
        Waypoint(nextWaypoint, Vector(maxAxisX / 2 - streetWidth / 2,
                                      maxAxisY / 2 - streetWidth / 2, 1.5)));
    nextWaypoint += Seconds((maxAxisX - streetWidth) / 2 / vTx);
    txMob->GetObject<WaypointMobilityModel>()->AddWaypoint(Waypoint(
        nextWaypoint, Vector(0.0, maxAxisY / 2 - streetWidth / 2, 1.5)));
    nextWaypoint = Seconds(0.0);
    rxMob->GetObject<WaypointMobilityModel>()->AddWaypoint(Waypoint(
        nextWaypoint, Vector(maxAxisX / 2 - streetWidth / 2, 0.0, 1.5)));
    nextWaypoint += Seconds(maxAxisY / vRx);
    rxMob->GetObject<WaypointMobilityModel>()->AddWaypoint(Waypoint(
        nextWaypoint, Vector(maxAxisX / 2 - streetWidth / 2, maxAxisY, 1.5)));

    nodes.Get(0)->AggregateObject(txMob);
    nodes.Get(1)->AggregateObject(rxMob);

    m_condModel = CreateObject<ThreeGppV2vUrbanChannelConditionModel>();

    m_propagationLossModel =
        CreateObject<ThreeGppV2vUrbanPropagationLossModel>();
  } else if (scenario == "V2V-Highway") {

    vScatt = 140 / 3.6;
    double vTx = vScatt;
    double vRx = vScatt / 2;

    txMob = CreateObject<ConstantVelocityMobilityModel>();
    rxMob = CreateObject<ConstantVelocityMobilityModel>();
    txMob->GetObject<ConstantVelocityMobilityModel>()->SetPosition(
        Vector(300.0, 20.0, 1.5));
    txMob->GetObject<ConstantVelocityMobilityModel>()->SetVelocity(
        Vector(0.0, vTx, 0.0));
    rxMob->GetObject<ConstantVelocityMobilityModel>()->SetPosition(
        Vector(300.0, 0.0, 1.5));
    rxMob->GetObject<ConstantVelocityMobilityModel>()->SetVelocity(
        Vector(0.0, vRx, 0.0));

    nodes.Get(0)->AggregateObject(txMob);
    nodes.Get(1)->AggregateObject(rxMob);

    m_condModel = CreateObject<ThreeGppV2vHighwayChannelConditionModel>();

    m_propagationLossModel =
        CreateObject<ThreeGppV2vHighwayPropagationLossModel>();
  } else {
    NS_FATAL_ERROR("Unknown scenario");
  }

  m_condModel->SetAttribute("UpdatePeriod", TimeValue(MilliSeconds(100)));

  m_propagationLossModel->SetAttribute("Frequency", DoubleValue(frequency));
  m_propagationLossModel->SetAttribute("ShadowingEnabled", BooleanValue(false));
  m_propagationLossModel->SetAttribute("ChannelConditionModel",
                                       PointerValue(m_condModel));

  Ptr<ThreeGppChannelModel> channelModel = CreateObject<ThreeGppChannelModel>();
  channelModel->SetAttribute("Scenario", StringValue(scenario));
  channelModel->SetAttribute("Frequency", DoubleValue(frequency));
  channelModel->SetAttribute("ChannelConditionModel",
                             PointerValue(m_condModel));
  channelModel->SetAttribute("vScatt", DoubleValue(vScatt));

  m_spectrumLossModel =
      CreateObjectWithAttributes<ThreeGppSpectrumPropagationLossModel>(
          "ChannelModel", PointerValue(channelModel));

  BuildingsHelper::Install(nodes);

  DoBeamforming(txDev, txAntenna, rxDev);
  DoBeamforming(rxDev, rxAntenna, txDev);

  Bands rbs;
  double freqSubBand = frequency;
  for (uint32_t n = 0; n < numRb; ++n) {
    BandInfo rb;
    rb.fl = freqSubBand;
    freqSubBand += subCarrierSpacing / 2;
    rb.fc = freqSubBand;
    freqSubBand += subCarrierSpacing / 2;
    rb.fh = freqSubBand;
    rbs.push_back(rb);
  }
  Ptr<SpectrumModel> spectrumModel = Create<SpectrumModel>(rbs);
  Ptr<SpectrumValue> txPsd = Create<SpectrumValue>(spectrumModel);
  Ptr<SpectrumSignalParameters> txParams = Create<SpectrumSignalParameters>();
  double txPow_w = std::pow(10., (txPow_dbm - 30) / 10);
  double txPowDens = (txPow_w / (numRb * subCarrierSpacing));
  (*txPsd) = txPowDens;
  txParams->psd = txPsd->Copy();

  for (int i = 0; i < simTime / timeRes; i++) {
    ComputeSnrParams params{txMob,       rxMob,     txParams,
                            noiseFigure, txAntenna, rxAntenna};
    Simulator::Schedule(timeRes * i, &ComputeSnr, params);
  }

  std::ofstream f;
  f.open("example-output.txt", std::ios::out);
  f << "Time[s] TxPosX[m] TxPosY[m] RxPosX[m] RxPosY[m] ChannelState SNR[dB] "
       "Pathloss[dB]"
    << std::endl;
  f.close();

  PrintGnuplottableBuildingListToFile("buildings.txt");

  Simulator::Run();
  Simulator::Destroy();
  return 0;
}
