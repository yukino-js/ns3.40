
#include "three-gpp-spectrum-propagation-loss-model.h"

#include "spectrum-signal-parameters.h"
#include "three-gpp-channel-model.h"

#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/string.h"

#include <map>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("ThreeGppSpectrumPropagationLossModel");

NS_OBJECT_ENSURE_REGISTERED(ThreeGppSpectrumPropagationLossModel);

ThreeGppSpectrumPropagationLossModel::ThreeGppSpectrumPropagationLossModel() {
  NS_LOG_FUNCTION(this);
}

ThreeGppSpectrumPropagationLossModel::~ThreeGppSpectrumPropagationLossModel() {
  NS_LOG_FUNCTION(this);
}

void ThreeGppSpectrumPropagationLossModel::DoDispose() {
  m_longTermMap.clear();
  m_channelModel->Dispose();
  m_channelModel = nullptr;
}

TypeId ThreeGppSpectrumPropagationLossModel::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::ThreeGppSpectrumPropagationLossModel")
          .SetParent<PhasedArraySpectrumPropagationLossModel>()
          .SetGroupName("Spectrum")
          .AddConstructor<ThreeGppSpectrumPropagationLossModel>()
          .AddAttribute(
              "ChannelModel",
              "The channel model. It needs to implement the "
              "MatrixBasedChannelModel interface",
              StringValue("ns3::ThreeGppChannelModel"),
              MakePointerAccessor(
                  &ThreeGppSpectrumPropagationLossModel::SetChannelModel,
                  &ThreeGppSpectrumPropagationLossModel::GetChannelModel),
              MakePointerChecker<MatrixBasedChannelModel>());
  return tid;
}

void ThreeGppSpectrumPropagationLossModel::SetChannelModel(
    Ptr<MatrixBasedChannelModel> channel) {
  m_channelModel = channel;
}

Ptr<MatrixBasedChannelModel>
ThreeGppSpectrumPropagationLossModel::GetChannelModel() const {
  return m_channelModel;
}

double ThreeGppSpectrumPropagationLossModel::GetFrequency() const {
  DoubleValue freq;
  m_channelModel->GetAttribute("Frequency", freq);
  return freq.Get();
}

void ThreeGppSpectrumPropagationLossModel::SetChannelModelAttribute(
    const std::string &name, const AttributeValue &value) {
  m_channelModel->SetAttribute(name, value);
}

void ThreeGppSpectrumPropagationLossModel::GetChannelModelAttribute(
    const std::string &name, AttributeValue &value) const {
  m_channelModel->GetAttribute(name, value);
}

PhasedArrayModel::ComplexVector
ThreeGppSpectrumPropagationLossModel::CalcLongTerm(
    Ptr<const MatrixBasedChannelModel::ChannelMatrix> params,
    const PhasedArrayModel::ComplexVector &sW,
    const PhasedArrayModel::ComplexVector &uW) const {
  NS_LOG_FUNCTION(this);

  size_t uAntennaNum = uW.GetSize();
  size_t sAntennaNum = sW.GetSize();

  NS_ASSERT(uAntennaNum == params->m_channel.GetNumRows());
  NS_ASSERT(sAntennaNum == params->m_channel.GetNumCols());

  NS_LOG_DEBUG("CalcLongTerm with " << uAntennaNum << " u antenna elements and "
                                    << sAntennaNum << " s antenna elements.");
  return params->m_channel.MultiplyByLeftAndRightMatrix(uW.Transpose(), sW);
}

Ptr<SpectrumValue> ThreeGppSpectrumPropagationLossModel::CalcBeamformingGain(
    Ptr<SpectrumValue> txPsd, PhasedArrayModel::ComplexVector longTerm,
    Ptr<const MatrixBasedChannelModel::ChannelMatrix> channelMatrix,
    Ptr<const MatrixBasedChannelModel::ChannelParams> channelParams,
    const ns3::Vector &sSpeed, const ns3::Vector &uSpeed) const {
  NS_LOG_FUNCTION(this);

  Ptr<SpectrumValue> tempPsd = Copy<SpectrumValue>(txPsd);

  uint16_t numCluster = channelMatrix->m_channel.GetNumPages();

  double slotTime = Simulator::Now().GetSeconds();
  double factor = 2 * M_PI * slotTime * GetFrequency() / 3e8;
  PhasedArrayModel::ComplexVector doppler(numCluster);

  NS_ASSERT(numCluster <= channelParams->m_alpha.size());
  NS_ASSERT(numCluster <= channelParams->m_D.size());
  NS_ASSERT(numCluster <=
            channelParams->m_angle[MatrixBasedChannelModel::ZOA_INDEX].size());
  NS_ASSERT(numCluster <=
            channelParams->m_angle[MatrixBasedChannelModel::ZOD_INDEX].size());
  NS_ASSERT(numCluster <=
            channelParams->m_angle[MatrixBasedChannelModel::AOA_INDEX].size());
  NS_ASSERT(numCluster <=
            channelParams->m_angle[MatrixBasedChannelModel::AOD_INDEX].size());
  NS_ASSERT(numCluster <= longTerm.GetSize());

  bool isSameDirection = (channelParams->m_nodeIds == channelMatrix->m_nodeIds);

  MatrixBasedChannelModel::DoubleVector zoa;
  MatrixBasedChannelModel::DoubleVector zod;
  MatrixBasedChannelModel::DoubleVector aoa;
  MatrixBasedChannelModel::DoubleVector aod;

  if (isSameDirection) {
    zoa = channelParams->m_angle[MatrixBasedChannelModel::ZOA_INDEX];
    zod = channelParams->m_angle[MatrixBasedChannelModel::ZOD_INDEX];
    aoa = channelParams->m_angle[MatrixBasedChannelModel::AOA_INDEX];
    aod = channelParams->m_angle[MatrixBasedChannelModel::AOD_INDEX];
  } else {
    zod = channelParams->m_angle[MatrixBasedChannelModel::ZOA_INDEX];
    zoa = channelParams->m_angle[MatrixBasedChannelModel::ZOD_INDEX];
    aod = channelParams->m_angle[MatrixBasedChannelModel::AOA_INDEX];
    aoa = channelParams->m_angle[MatrixBasedChannelModel::AOD_INDEX];
  }

  for (uint16_t cIndex = 0; cIndex < numCluster; cIndex++) {

    double alpha = channelParams->m_alpha[cIndex];
    double D = channelParams->m_D[cIndex];

    double tempDoppler =
        factor * ((sin(zoa[cIndex] * M_PI / 180) *
                       cos(aoa[cIndex] * M_PI / 180) * uSpeed.x +
                   sin(zoa[cIndex] * M_PI / 180) *
                       sin(aoa[cIndex] * M_PI / 180) * uSpeed.y +
                   cos(zoa[cIndex] * M_PI / 180) * uSpeed.z) +
                  (sin(zod[cIndex] * M_PI / 180) *
                       cos(aod[cIndex] * M_PI / 180) * sSpeed.x +
                   sin(zod[cIndex] * M_PI / 180) *
                       sin(aod[cIndex] * M_PI / 180) * sSpeed.y +
                   cos(zod[cIndex] * M_PI / 180) * sSpeed.z) +
                  2 * alpha * D);
    doppler[cIndex] = std::complex<double>(cos(tempDoppler), sin(tempDoppler));
  }

  NS_ASSERT(numCluster <= doppler.GetSize());

  auto vit = tempPsd->ValuesBegin();
  auto sbit = tempPsd->ConstBandsBegin();
  while (vit != tempPsd->ValuesEnd()) {
    if ((*vit) != 0.00) {
      std::complex<double> subsbandGain(0.0, 0.0);
      double fsb = (*sbit).fc;
      for (uint16_t cIndex = 0; cIndex < numCluster; cIndex++) {
        double delay = -2 * M_PI * fsb * (channelParams->m_delay[cIndex]);
        subsbandGain =
            subsbandGain + longTerm[cIndex] * doppler[cIndex] *
                               std::complex<double>(cos(delay), sin(delay));
      }
      *vit = (*vit) * (norm(subsbandGain));
    }
    vit++;
    sbit++;
  }
  return tempPsd;
}

PhasedArrayModel::ComplexVector
ThreeGppSpectrumPropagationLossModel::GetLongTerm(
    Ptr<const MatrixBasedChannelModel::ChannelMatrix> channelMatrix,
    Ptr<const PhasedArrayModel> aPhasedArrayModel,
    Ptr<const PhasedArrayModel> bPhasedArrayModel) const {
  PhasedArrayModel::ComplexVector longTerm;

  PhasedArrayModel::ComplexVector sW;
  PhasedArrayModel::ComplexVector uW;
  if (!channelMatrix->IsReverse(aPhasedArrayModel->GetId(),
                                bPhasedArrayModel->GetId())) {
    sW = aPhasedArrayModel->GetBeamformingVector();
    uW = bPhasedArrayModel->GetBeamformingVector();
  } else {
    sW = bPhasedArrayModel->GetBeamformingVector();
    uW = aPhasedArrayModel->GetBeamformingVector();
  }

  bool update = false;
  bool notFound = false;

  uint64_t longTermId = MatrixBasedChannelModel::GetKey(
      aPhasedArrayModel->GetId(), bPhasedArrayModel->GetId());

  if (m_longTermMap.find(longTermId) != m_longTermMap.end()) {
    NS_LOG_DEBUG("found the long term component in the map");
    longTerm = m_longTermMap[longTermId]->m_longTerm;

    update = (m_longTermMap[longTermId]->m_channel->m_generatedTime !=
                  channelMatrix->m_generatedTime ||
              m_longTermMap[longTermId]->m_sW != sW ||
              m_longTermMap[longTermId]->m_uW != uW);
  } else {
    NS_LOG_DEBUG("long term component NOT found");
    notFound = true;
  }

  if (update || notFound) {
    NS_LOG_DEBUG("compute the long term");
    longTerm = CalcLongTerm(channelMatrix, sW, uW);

    Ptr<LongTerm> longTermItem = Create<LongTerm>();
    longTermItem->m_longTerm = longTerm;
    longTermItem->m_channel = channelMatrix;
    longTermItem->m_sW = sW;
    longTermItem->m_uW = uW;

    m_longTermMap[longTermId] = longTermItem;
  }

  return longTerm;
}

Ptr<SpectrumValue>
ThreeGppSpectrumPropagationLossModel::DoCalcRxPowerSpectralDensity(
    Ptr<const SpectrumSignalParameters> params, Ptr<const MobilityModel> a,
    Ptr<const MobilityModel> b, Ptr<const PhasedArrayModel> aPhasedArrayModel,
    Ptr<const PhasedArrayModel> bPhasedArrayModel) const {
  NS_LOG_FUNCTION(this);
  uint32_t aId = a->GetObject<Node>()->GetId();
  uint32_t bId = b->GetObject<Node>()->GetId();

  NS_ASSERT(aId != bId);
  NS_ASSERT_MSG(a->GetDistanceFrom(b) > 0.0,
                "The position of a and b devices cannot be the same");

  Ptr<SpectrumValue> rxPsd = Copy<SpectrumValue>(params->psd);

  NS_ASSERT_MSG(aPhasedArrayModel, "Antenna not found for node " << aId);
  NS_LOG_DEBUG("a node " << a->GetObject<Node>() << " antenna "
                         << aPhasedArrayModel);

  NS_ASSERT_MSG(bPhasedArrayModel, "Antenna not found for device " << bId);
  NS_LOG_DEBUG("b node " << bId << " antenna " << bPhasedArrayModel);

  Ptr<const MatrixBasedChannelModel::ChannelMatrix> channelMatrix =
      m_channelModel->GetChannel(a, b, aPhasedArrayModel, bPhasedArrayModel);
  Ptr<const MatrixBasedChannelModel::ChannelParams> channelParams =
      m_channelModel->GetParams(a, b);

  PhasedArrayModel::ComplexVector longTerm =
      GetLongTerm(channelMatrix, aPhasedArrayModel, bPhasedArrayModel);

  rxPsd = CalcBeamformingGain(rxPsd, longTerm, channelMatrix, channelParams,
                              a->GetVelocity(), b->GetVelocity());

  return rxPsd;
}

} // namespace ns3
