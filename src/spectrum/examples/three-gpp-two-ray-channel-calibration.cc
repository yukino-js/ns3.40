
#include <ns3/command-line.h>
#include <ns3/core-module.h>
#include <ns3/double.h>
#include <ns3/isotropic-antenna-model.h>
#include <ns3/mobility-helper.h>
#include <ns3/node-container.h>
#include <ns3/object-factory.h>
#include <ns3/pointer.h>
#include <ns3/string.h>
#include <ns3/three-gpp-channel-model.h>
#include <ns3/three-gpp-propagation-loss-model.h>
#include <ns3/three-gpp-spectrum-propagation-loss-model.h>
#include <ns3/two-ray-spectrum-propagation-loss-model.h>
#include <ns3/uinteger.h>
#include <ns3/uniform-planar-array.h>

NS_LOG_COMPONENT_DEFINE("ThreeGppTwoRayChannelCalibration");

using namespace ns3;

constexpr double FC_STEP = 5e9;

constexpr double MIN_FC = 500e6;

constexpr double MAX_FC = 100e9;

constexpr double BW = 200e6;

constexpr double RB_WIDTH = 60e3;

const std::vector<std::string> LOS_CONDITIONS{
    "LOS",
    "NLOS",
};

const std::vector<std::string> THREE_GPP_SCENARIOS{
    "RMa", "UMa", "UMi-StreetCanyon", "InH-OfficeOpen", "InH-OfficeMixed",
};

const Ptr<OutputStreamWrapper> g_outStream = Create<OutputStreamWrapper>(
    "two-ray-to-three-gpp-calibration.csv", std::ios::out);

void LogEndToEndGain(std::string cond, std::string scen, double fc,
                     long int seed, double gain) {
  *g_outStream->GetStream() << cond << "\t" << scen << "\t" << fc << "\t"
                            << seed << "\t" << gain << "\n";
}

double ComputePowerSpectralDensityOverallPower(Ptr<const SpectrumValue> psd) {
  return Integral(*psd);
}

Ptr<SpectrumValue> CreateTxPowerSpectralDensity(double fc) {
  uint32_t numRbs = std::floor(BW / RB_WIDTH);
  double f = fc - (numRbs * RB_WIDTH / 2.0);
  double powerTx = 0.0;

  Bands rbs;
  std::vector<int> rbsId;
  rbsId.reserve(numRbs);

  for (uint32_t numrb = 0; numrb < numRbs; ++numrb) {
    BandInfo rb;
    rb.fl = f;
    f += RB_WIDTH / 2;
    rb.fc = f;
    f += RB_WIDTH / 2;
    rb.fh = f;

    rbs.push_back(rb);
    rbsId.push_back(numrb);
  }
  Ptr<SpectrumModel> model = Create<SpectrumModel>(rbs);
  Ptr<SpectrumValue> txPsd = Create<SpectrumValue>(model);

  double powerTxW = std::pow(10., (powerTx - 30) / 10);
  double txPowerDensity = powerTxW / BW;

  for (auto rbId : rbsId) {
    (*txPsd)[rbId] = txPowerDensity;
  }

  return txPsd;
}

double ComputeEndToEndGain(std::string cond, std::string scen, double fc,
                           Ptr<Node> a, Ptr<Node> b,
                           Ptr<PhasedArrayModel> aArray,
                           Ptr<PhasedArrayModel> bArray) {
  Ptr<ChannelConditionModel> channelConditionModel;
  if (cond == "LOS") {
    channelConditionModel = CreateObject<AlwaysLosChannelConditionModel>();
  } else if (cond == "NLOS") {
    channelConditionModel = CreateObject<NeverLosChannelConditionModel>();
  } else {
    NS_ABORT_MSG("Unsupported channel condition");
  }

  Ptr<ThreeGppSpectrumPropagationLossModel> threeGppSpectrumLossModel =
      CreateObject<ThreeGppSpectrumPropagationLossModel>();
  Ptr<ThreeGppChannelModel> threeGppChannelModel =
      CreateObject<ThreeGppChannelModel>();

  threeGppSpectrumLossModel->SetAttribute("ChannelModel",
                                          PointerValue(threeGppChannelModel));
  threeGppChannelModel->SetAttribute("ChannelConditionModel",
                                     PointerValue(channelConditionModel));

  Ptr<SpectrumValue> txPsd = CreateTxPowerSpectralDensity(fc);
  double txPower = ComputePowerSpectralDensityOverallPower(txPsd);

  Ptr<SpectrumSignalParameters> signalParams =
      Create<SpectrumSignalParameters>();
  signalParams->psd = txPsd;

  threeGppChannelModel->SetAttribute("Frequency", DoubleValue(fc));

  threeGppChannelModel->SetAttribute("Scenario", StringValue(scen));

  threeGppChannelModel->SetAttribute("Blockage", BooleanValue(false));

  Ptr<MobilityModel> aMob = a->GetObject<MobilityModel>();
  Ptr<MobilityModel> bMob = b->GetObject<MobilityModel>();
  Vector aPos = aMob->GetPosition();
  Vector bPos = bMob->GetPosition();

  Angles angleBtoA(bPos, aPos);
  Angles angleAtoB(aPos, bPos);

  aArray->SetBeamformingVector(aArray->GetBeamformingVector(angleBtoA));
  bArray->SetBeamformingVector(bArray->GetBeamformingVector(angleAtoB));

  auto rxPsd = threeGppSpectrumLossModel->DoCalcRxPowerSpectralDensity(
      signalParams, aMob, bMob, aArray, bArray);
  double rxPower = ComputePowerSpectralDensityOverallPower(rxPsd);

  return rxPower / txPower;
}

int main(int argc, char *argv[]) {
  uint32_t numRealizations = 5000;
  bool enableOutput = false;

  CommandLine cmd(__FILE__);
  cmd.AddValue("enableOutput", "Logs the results of the example", enableOutput);
  cmd.AddValue("numRealizations", "The number of different realizations",
               numRealizations);
  cmd.Parse(argc, argv);

  if (enableOutput) {
    *g_outStream->GetStream() << "cond\tscen\tfc\tseed\tgain\n";
  }

  NodeContainer nodes;
  nodes.Create(2);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();
  Vector aPos(0.0, 0.0, 0.0);
  Vector bPos(10.0, 0.0, 0.0);
  positionAlloc->Add(aPos);
  positionAlloc->Add(bPos);
  mobility.SetPositionAllocator(positionAlloc);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(nodes);

  Ptr<PhasedArrayModel> aPhasedArray =
      CreateObjectWithAttributes<UniformPlanarArray>(
          "NumColumns", UintegerValue(1), "NumRows", UintegerValue(1));
  aPhasedArray->SetAntennaElement(
      PointerValue(CreateObject<IsotropicAntennaModel>()));
  Ptr<PhasedArrayModel> bPhasedArray =
      CreateObjectWithAttributes<UniformPlanarArray>(
          "NumColumns", UintegerValue(1), "NumRows", UintegerValue(1));
  bPhasedArray->SetAntennaElement(
      PointerValue(CreateObject<IsotropicAntennaModel>()));

  for (const auto &cond : LOS_CONDITIONS) {
    for (const auto &scen : THREE_GPP_SCENARIOS) {
      for (double fc = MIN_FC; fc < MAX_FC; fc += FC_STEP) {
        for (uint32_t runIdx = 0; runIdx < numRealizations; runIdx++) {
          double gain =
              ComputeEndToEndGain(cond, scen, fc, nodes.Get(0), nodes.Get(1),
                                  aPhasedArray, bPhasedArray);
          if (enableOutput) {
            LogEndToEndGain(cond, scen, fc, runIdx, gain);
          }
        }
      }
    }
  }
}
