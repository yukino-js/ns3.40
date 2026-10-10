
#include "wifi-spectrum-value-helper.h"

#include "ns3/assert.h"
#include "ns3/fatal-error.h"
#include "ns3/log.h"

#include <cmath>
#include <map>
#include <sstream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("WifiSpectrumValueHelper");

struct WifiSpectrumModelId {
  uint32_t m_centerFrequency;
  uint16_t m_channelWidth;
  uint32_t m_carrierSpacing;
  uint16_t m_guardBandwidth;
};

bool operator<(const WifiSpectrumModelId &a, const WifiSpectrumModelId &b) {
  return ((a.m_centerFrequency < b.m_centerFrequency) ||
          ((a.m_centerFrequency == b.m_centerFrequency) &&
           (a.m_channelWidth < b.m_channelWidth)) ||
          ((a.m_centerFrequency == b.m_centerFrequency) &&
           (a.m_channelWidth == b.m_channelWidth) &&
           (a.m_carrierSpacing < b.m_carrierSpacing)) ||
          ((a.m_centerFrequency == b.m_centerFrequency) &&
           (a.m_channelWidth == b.m_channelWidth) &&
           (a.m_carrierSpacing == b.m_carrierSpacing) &&
           (a.m_guardBandwidth < b.m_guardBandwidth)));
}

static std::map<WifiSpectrumModelId, Ptr<SpectrumModel>> g_wifiSpectrumModelMap;

Ptr<SpectrumModel> WifiSpectrumValueHelper::GetSpectrumModel(
    uint32_t centerFrequency, uint16_t channelWidth, uint32_t carrierSpacing,
    uint16_t guardBandwidth) {
  NS_LOG_FUNCTION(centerFrequency << channelWidth << carrierSpacing
                                  << guardBandwidth);
  Ptr<SpectrumModel> ret;
  WifiSpectrumModelId key{centerFrequency, channelWidth, carrierSpacing,
                          guardBandwidth};
  auto it = g_wifiSpectrumModelMap.find(key);
  if (it != g_wifiSpectrumModelMap.end()) {
    ret = it->second;
  } else {
    Bands bands;
    double centerFrequencyHz = centerFrequency * 1e6;
    double bandwidth = (channelWidth + (2.0 * guardBandwidth)) * 1e6;
    auto numBands = static_cast<uint32_t>((bandwidth / carrierSpacing) + 0.5);
    NS_ASSERT(numBands > 0);
    if (numBands % 2 == 0) {
      numBands += 1;
    }
    NS_ASSERT_MSG(numBands % 2 == 1, "Number of bands should be odd");
    NS_LOG_DEBUG("Num bands " << numBands << " band bandwidth "
                              << carrierSpacing);
    double startingFrequencyHz = centerFrequencyHz -
                                 (numBands / 2 * carrierSpacing) -
                                 carrierSpacing / 2;
    for (size_t i = 0; i < numBands; i++) {
      BandInfo info;
      double f = startingFrequencyHz + (i * carrierSpacing);
      info.fl = f;
      f += carrierSpacing / 2;
      info.fc = f;
      f += carrierSpacing / 2;
      info.fh = f;
      NS_LOG_DEBUG("creating band " << i << " (" << info.fl << ":" << info.fc
                                    << ":" << info.fh << ")");
      bands.push_back(info);
    }
    ret = Create<SpectrumModel>(std::move(bands));
    g_wifiSpectrumModelMap.insert(
        std::pair<WifiSpectrumModelId, Ptr<SpectrumModel>>(key, ret));
  }
  NS_LOG_LOGIC("returning SpectrumModel::GetUid () == " << ret->GetUid());
  return ret;
}

Ptr<SpectrumValue> WifiSpectrumValueHelper::CreateDsssTxPowerSpectralDensity(
    uint32_t centerFrequency, double txPowerW, uint16_t guardBandwidth) {
  NS_LOG_FUNCTION(centerFrequency << txPowerW << +guardBandwidth);
  uint16_t channelWidth = 22;
  uint32_t carrierSpacing = 312500;
  Ptr<SpectrumValue> c = Create<SpectrumValue>(GetSpectrumModel(
      centerFrequency, channelWidth, carrierSpacing, guardBandwidth));
  auto vit = c->ValuesBegin();
  auto bit = c->ConstBandsBegin();
  auto nGuardBands = static_cast<uint32_t>(
      ((2 * guardBandwidth * 1e6) / carrierSpacing) + 0.5);
  auto nAllocatedBands =
      static_cast<uint32_t>(((channelWidth * 1e6) / carrierSpacing) + 0.5);
  NS_ASSERT(c->GetSpectrumModel()->GetNumBands() ==
            (nAllocatedBands + nGuardBands + 1));
  double txPowerPerBand = txPowerW / nAllocatedBands;
  for (size_t i = 0; i < c->GetSpectrumModel()->GetNumBands();
       i++, vit++, bit++) {
    if ((i >= (nGuardBands / 2)) &&
        (i <= ((nGuardBands / 2) + nAllocatedBands - 1))) {
      *vit = txPowerPerBand / (bit->fh - bit->fl);
    }
  }
  return c;
}

Ptr<SpectrumValue> WifiSpectrumValueHelper::CreateOfdmTxPowerSpectralDensity(
    uint32_t centerFrequency, uint16_t channelWidth, double txPowerW,
    uint16_t guardBandwidth, double minInnerBandDbr, double minOuterBandDbr,
    double lowestPointDbr) {
  NS_LOG_FUNCTION(centerFrequency << channelWidth << txPowerW << guardBandwidth
                                  << minInnerBandDbr << minOuterBandDbr
                                  << lowestPointDbr);
  uint32_t carrierSpacing = 0;
  uint32_t innerSlopeWidth = 0;
  switch (channelWidth) {
  case 20:
    carrierSpacing = 312500;
    innerSlopeWidth = static_cast<uint32_t>((2e6 / carrierSpacing) + 0.5);
    break;
  case 10:
    carrierSpacing = 156250;
    innerSlopeWidth = static_cast<uint32_t>((1e6 / carrierSpacing) + 0.5);
    break;
  case 5:
    carrierSpacing = 78125;
    innerSlopeWidth = static_cast<uint32_t>((5e5 / carrierSpacing) + 0.5);
    break;
  default:
    NS_FATAL_ERROR("Channel width " << channelWidth
                                    << " should be correctly set.");
    return nullptr;
  }

  Ptr<SpectrumValue> c = Create<SpectrumValue>(GetSpectrumModel(
      centerFrequency, channelWidth, carrierSpacing, guardBandwidth));
  auto nGuardBands = static_cast<uint32_t>(
      ((2 * guardBandwidth * 1e6) / carrierSpacing) + 0.5);
  auto nAllocatedBands =
      static_cast<uint32_t>(((channelWidth * 1e6) / carrierSpacing) + 0.5);
  NS_ASSERT_MSG(c->GetSpectrumModel()->GetNumBands() ==
                    (nAllocatedBands + nGuardBands + 1),
                "Unexpected number of bands "
                    << c->GetSpectrumModel()->GetNumBands());
  double txPowerPerBandW = txPowerW / 52;
  NS_LOG_DEBUG("Power per band " << txPowerPerBandW << "W");
  uint32_t start1 = (nGuardBands / 2) + 6;
  uint32_t stop1 = start1 + 26 - 1;
  uint32_t start2 = stop1 + 2;
  uint32_t stop2 = start2 + 26 - 1;

  std::vector<WifiSpectrumBandIndices> subBands{
      std::make_pair(start1, stop1),
      std::make_pair(start2, stop2),
  };
  WifiSpectrumBandIndices maskBand(0, nAllocatedBands + nGuardBands);
  CreateSpectrumMaskForOfdm(c, subBands, maskBand, txPowerPerBandW, nGuardBands,
                            innerSlopeWidth, minInnerBandDbr, minOuterBandDbr,
                            lowestPointDbr);
  NormalizeSpectrumMask(c, txPowerW);
  NS_ASSERT_MSG(std::abs(txPowerW - Integral(*c)) < 1e-6,
                "Power allocation failed");
  return c;
}

Ptr<SpectrumValue>
WifiSpectrumValueHelper::CreateDuplicated20MhzTxPowerSpectralDensity(
    uint32_t centerFrequency, uint16_t channelWidth, double txPowerW,
    uint16_t guardBandwidth, double minInnerBandDbr, double minOuterBandDbr,
    double lowestPointDbr, const std::vector<bool> &puncturedSubchannels) {
  NS_LOG_FUNCTION(centerFrequency << channelWidth << txPowerW << guardBandwidth
                                  << minInnerBandDbr << minOuterBandDbr
                                  << lowestPointDbr);
  uint32_t carrierSpacing = 312500;
  Ptr<SpectrumValue> c = Create<SpectrumValue>(GetSpectrumModel(
      centerFrequency, channelWidth, carrierSpacing, guardBandwidth));
  auto nGuardBands = static_cast<uint32_t>(
      ((2 * guardBandwidth * 1e6) / carrierSpacing) + 0.5);
  auto nAllocatedBands =
      static_cast<uint32_t>(((channelWidth * 1e6) / carrierSpacing) + 0.5);
  NS_ASSERT_MSG(c->GetSpectrumModel()->GetNumBands() ==
                    (nAllocatedBands + nGuardBands + 1),
                "Unexpected number of bands "
                    << c->GetSpectrumModel()->GetNumBands());
  std::size_t num20MhzBands = channelWidth / 20;
  std::size_t numAllocatedSubcarriersPer20MHz = 52;
  NS_ASSERT(puncturedSubchannels.empty() ||
            (puncturedSubchannels.size() == num20MhzBands));
  double txPowerPerBandW =
      (txPowerW / numAllocatedSubcarriersPer20MHz) / num20MhzBands;
  NS_LOG_DEBUG("Power per band " << txPowerPerBandW << "W");

  std::size_t numSubcarriersPer20MHz = (20 * 1e6) / carrierSpacing;
  std::size_t numUnallocatedSubcarriersPer20MHz =
      numSubcarriersPer20MHz - numAllocatedSubcarriersPer20MHz;
  std::vector<WifiSpectrumBandIndices> subBands;
  subBands.resize(num20MhzBands * 2);
  uint32_t start = (nGuardBands / 2) + (numUnallocatedSubcarriersPer20MHz / 2);
  uint32_t stop;
  uint8_t index = 0;
  for (auto it = subBands.begin(); it != subBands.end();) {
    if (!puncturedSubchannels.empty() && puncturedSubchannels.at(index++)) {
      NS_LOG_DEBUG("20 MHz subchannel " << +index << " is punctured");
      it += 2;
      continue;
    }
    stop = start + (numAllocatedSubcarriersPer20MHz / 2) - 1;
    *it = std::make_pair(start, stop);
    ++it;
    start = stop + 2;
    stop = start + (numAllocatedSubcarriersPer20MHz / 2) - 1;
    *it = std::make_pair(start, stop);
    ++it;
    start = stop + numUnallocatedSubcarriersPer20MHz;
  }

  auto innerSlopeWidth = static_cast<uint32_t>((2e6 / carrierSpacing) + 0.5);
  WifiSpectrumBandIndices maskBand(0, nAllocatedBands + nGuardBands);

  CreateSpectrumMaskForOfdm(c, subBands, maskBand, txPowerPerBandW, nGuardBands,
                            innerSlopeWidth, minInnerBandDbr, minOuterBandDbr,
                            lowestPointDbr);
  NormalizeSpectrumMask(c, txPowerW);
  NS_ASSERT_MSG(std::abs(txPowerW - Integral(*c)) < 1e-6,
                "Power allocation failed");
  return c;
}

Ptr<SpectrumValue> WifiSpectrumValueHelper::CreateHtOfdmTxPowerSpectralDensity(
    uint32_t centerFrequency, uint16_t channelWidth, double txPowerW,
    uint16_t guardBandwidth, double minInnerBandDbr, double minOuterBandDbr,
    double lowestPointDbr) {
  NS_LOG_FUNCTION(centerFrequency << channelWidth << txPowerW << guardBandwidth
                                  << minInnerBandDbr << minOuterBandDbr
                                  << lowestPointDbr);
  uint32_t carrierSpacing = 312500;
  Ptr<SpectrumValue> c = Create<SpectrumValue>(GetSpectrumModel(
      centerFrequency, channelWidth, carrierSpacing, guardBandwidth));
  auto nGuardBands = static_cast<uint32_t>(
      ((2 * guardBandwidth * 1e6) / carrierSpacing) + 0.5);
  auto nAllocatedBands =
      static_cast<uint32_t>(((channelWidth * 1e6) / carrierSpacing) + 0.5);
  NS_ASSERT_MSG(c->GetSpectrumModel()->GetNumBands() ==
                    (nAllocatedBands + nGuardBands + 1),
                "Unexpected number of bands "
                    << c->GetSpectrumModel()->GetNumBands());
  std::size_t num20MhzBands = channelWidth / 20;
  std::size_t numAllocatedSubcarriersPer20MHz = 56;
  double txPowerPerBandW =
      (txPowerW / numAllocatedSubcarriersPer20MHz) / num20MhzBands;
  NS_LOG_DEBUG("Power per band " << txPowerPerBandW << "W");

  std::size_t numSubcarriersPer20MHz = (20 * 1e6) / carrierSpacing;
  std::size_t numUnallocatedSubcarriersPer20MHz =
      numSubcarriersPer20MHz - numAllocatedSubcarriersPer20MHz;
  std::vector<WifiSpectrumBandIndices> subBands;
  subBands.resize(num20MhzBands * 2);
  uint32_t start = (nGuardBands / 2) + (numUnallocatedSubcarriersPer20MHz / 2);
  uint32_t stop;
  for (auto it = subBands.begin(); it != subBands.end();) {
    stop = start + (numAllocatedSubcarriersPer20MHz / 2) - 1;
    *it = std::make_pair(start, stop);
    ++it;
    start = stop + 2;
    stop = start + (numAllocatedSubcarriersPer20MHz / 2) - 1;
    *it = std::make_pair(start, stop);
    ++it;
    start = stop + numUnallocatedSubcarriersPer20MHz;
  }

  auto innerSlopeWidth = static_cast<uint32_t>((2e6 / carrierSpacing) + 0.5);
  WifiSpectrumBandIndices maskBand(0, nAllocatedBands + nGuardBands);

  CreateSpectrumMaskForOfdm(c, subBands, maskBand, txPowerPerBandW, nGuardBands,
                            innerSlopeWidth, minInnerBandDbr, minOuterBandDbr,
                            lowestPointDbr);
  NormalizeSpectrumMask(c, txPowerW);
  NS_ASSERT_MSG(std::abs(txPowerW - Integral(*c)) < 1e-6,
                "Power allocation failed");
  return c;
}

Ptr<SpectrumValue> WifiSpectrumValueHelper::CreateHeOfdmTxPowerSpectralDensity(
    uint32_t centerFrequency, uint16_t channelWidth, double txPowerW,
    uint16_t guardBandwidth, double minInnerBandDbr, double minOuterBandDbr,
    double lowestPointDbr, const std::vector<bool> &puncturedSubchannels) {
  NS_LOG_FUNCTION(centerFrequency << channelWidth << txPowerW << guardBandwidth
                                  << minInnerBandDbr << minOuterBandDbr
                                  << lowestPointDbr);
  uint32_t carrierSpacing = 78125;
  Ptr<SpectrumValue> c = Create<SpectrumValue>(GetSpectrumModel(
      centerFrequency, channelWidth, carrierSpacing, guardBandwidth));
  auto nGuardBands = static_cast<uint32_t>(
      ((2 * guardBandwidth * 1e6) / carrierSpacing) + 0.5);
  auto nAllocatedBands =
      static_cast<uint32_t>(((channelWidth * 1e6) / carrierSpacing) + 0.5);
  NS_ASSERT_MSG(c->GetSpectrumModel()->GetNumBands() ==
                    (nAllocatedBands + nGuardBands + 1),
                "Unexpected number of bands "
                    << c->GetSpectrumModel()->GetNumBands());
  double txPowerPerBandW = 0.0;
  uint32_t start1;
  uint32_t stop1;
  uint32_t start2;
  uint32_t stop2;
  uint32_t start3;
  uint32_t stop3;
  uint32_t start4;
  uint32_t stop4;
  auto innerSlopeWidth = static_cast<uint32_t>((1e6 / carrierSpacing) + 0.5);
  std::vector<WifiSpectrumBandIndices> subBands;
  WifiSpectrumBandIndices maskBand(0, nAllocatedBands + nGuardBands);
  switch (channelWidth) {
  case 20:
    txPowerPerBandW = txPowerW / 242;
    innerSlopeWidth = static_cast<uint32_t>((5e5 / carrierSpacing) + 0.5);
    start1 = (nGuardBands / 2) + 6;
    stop1 = start1 + 121 - 1;
    start2 = stop1 + 4;
    stop2 = start2 + 121 - 1;
    subBands.emplace_back(start1, stop1);
    subBands.emplace_back(start2, stop2);
    break;
  case 40:
    txPowerPerBandW = txPowerW / 484;
    start1 = (nGuardBands / 2) + 12;
    stop1 = start1 + 242 - 1;
    start2 = stop1 + 6;
    stop2 = start2 + 242 - 1;
    subBands.emplace_back(start1, stop1);
    subBands.emplace_back(start2, stop2);
    break;
  case 80:
    txPowerPerBandW = txPowerW / 996;
    start1 = (nGuardBands / 2) + 12;
    stop1 = start1 + 498 - 1;
    start2 = stop1 + 6;
    stop2 = start2 + 498 - 1;
    subBands.emplace_back(start1, stop1);
    subBands.emplace_back(start2, stop2);
    break;
  case 160:
    txPowerPerBandW = txPowerW / (2 * 996);
    start1 = (nGuardBands / 2) + 12;
    stop1 = start1 + 498 - 1;
    start2 = stop1 + 6;
    stop2 = start2 + 498 - 1;
    start3 = stop2 + (2 * 12);
    stop3 = start3 + 498 - 1;
    start4 = stop3 + 6;
    stop4 = start4 + 498 - 1;
    subBands.emplace_back(start1, stop1);
    subBands.emplace_back(start2, stop2);
    subBands.emplace_back(start3, stop3);
    subBands.emplace_back(start4, stop4);
    break;
  default:
    NS_FATAL_ERROR("ChannelWidth " << channelWidth << " unsupported");
    break;
  }

  auto puncturedSlopeWidth =
      static_cast<uint32_t>((500e3 / carrierSpacing) + 0.5);
  std::vector<WifiSpectrumBandIndices> puncturedBands;
  std::size_t subcarriersPerSuband = (20 * 1e6 / carrierSpacing);
  uint32_t start = (nGuardBands / 2);
  uint32_t stop = start + subcarriersPerSuband - 1;
  for (auto puncturedSubchannel : puncturedSubchannels) {
    if (puncturedSubchannel) {
      puncturedBands.emplace_back(start, stop);
    }
    start = stop + 1;
    stop = start + subcarriersPerSuband - 1;
  }

  CreateSpectrumMaskForOfdm(c, subBands, maskBand, txPowerPerBandW, nGuardBands,
                            innerSlopeWidth, minInnerBandDbr, minOuterBandDbr,
                            lowestPointDbr, puncturedBands,
                            puncturedSlopeWidth);
  NormalizeSpectrumMask(c, txPowerW);
  NS_ASSERT_MSG(std::abs(txPowerW - Integral(*c)) < 1e-6,
                "Power allocation failed");
  return c;
}

Ptr<SpectrumValue>
WifiSpectrumValueHelper::CreateHeMuOfdmTxPowerSpectralDensity(
    uint32_t centerFrequency, uint16_t channelWidth, double txPowerW,
    uint16_t guardBandwidth, const WifiSpectrumBandIndices &ru) {
  NS_LOG_FUNCTION(centerFrequency << channelWidth << txPowerW << guardBandwidth
                                  << ru.first << ru.second);
  uint32_t carrierSpacing = 78125;
  Ptr<SpectrumValue> c = Create<SpectrumValue>(GetSpectrumModel(
      centerFrequency, channelWidth, carrierSpacing, guardBandwidth));

  auto vit = c->ValuesBegin();
  auto bit = c->ConstBandsBegin();
  double txPowerPerBandW = (txPowerW / (ru.second - ru.first + 1));
  uint32_t numBands = c->GetSpectrumModel()->GetNumBands();
  for (size_t i = 0; i < numBands; i++, vit++, bit++) {
    if (i < ru.first || i > ru.second) {
      *vit = 0.0;
    } else {
      *vit = (txPowerPerBandW / (bit->fh - bit->fl));
    }
  }

  return c;
}

Ptr<SpectrumValue> WifiSpectrumValueHelper::CreateNoisePowerSpectralDensity(
    uint32_t centerFrequency, uint16_t channelWidth, uint32_t carrierSpacing,
    double noiseFigure, uint16_t guardBandwidth) {
  Ptr<SpectrumModel> model = GetSpectrumModel(centerFrequency, channelWidth,
                                              carrierSpacing, guardBandwidth);
  return CreateNoisePowerSpectralDensity(noiseFigure, model);
}

Ptr<SpectrumValue> WifiSpectrumValueHelper::CreateNoisePowerSpectralDensity(
    double noiseFigureDb, Ptr<SpectrumModel> spectrumModel) {
  NS_LOG_FUNCTION(noiseFigureDb << spectrumModel);

  const double kT_dBm_Hz = -174.0;
  double kT_W_Hz = DbmToW(kT_dBm_Hz);
  double noiseFigureLinear = std::pow(10.0, noiseFigureDb / 10.0);
  double noisePowerSpectralDensity = kT_W_Hz * noiseFigureLinear;

  Ptr<SpectrumValue> noisePsd = Create<SpectrumValue>(spectrumModel);
  (*noisePsd) = noisePowerSpectralDensity;
  NS_LOG_INFO("NoisePowerSpectralDensity has integrated power of "
              << Integral(*noisePsd));
  return noisePsd;
}

void WifiSpectrumValueHelper::CreateSpectrumMaskForOfdm(
    Ptr<SpectrumValue> c,
    const std::vector<WifiSpectrumBandIndices> &allocatedSubBands,
    const WifiSpectrumBandIndices &maskBand, double txPowerPerBandW,
    uint32_t nGuardBands, uint32_t innerSlopeWidth, double minInnerBandDbr,
    double minOuterBandDbr, double lowestPointDbr,
    const std::vector<WifiSpectrumBandIndices> &puncturedBands,
    uint32_t puncturedSlopeWidth) {
  NS_LOG_FUNCTION(c << allocatedSubBands.front().first
                    << allocatedSubBands.back().second << maskBand.first
                    << maskBand.second << txPowerPerBandW << nGuardBands
                    << innerSlopeWidth << minInnerBandDbr << minOuterBandDbr
                    << lowestPointDbr << puncturedSlopeWidth);
  uint32_t numSubBands = allocatedSubBands.size();
  uint32_t numBands = c->GetSpectrumModel()->GetNumBands();
  uint32_t numMaskBands = maskBand.second - maskBand.first + 1;
  NS_ASSERT(numSubBands && numBands && numMaskBands);
  NS_LOG_LOGIC("Power per band " << txPowerPerBandW << "W");

  double txPowerRefDbm = (10.0 * std::log10(txPowerPerBandW * 1000.0));
  double txPowerInnerBandMinDbm = txPowerRefDbm + minInnerBandDbr;
  double txPowerMiddleBandMinDbm = txPowerRefDbm + minOuterBandDbr;
  double txPowerOuterBandMinDbm = txPowerRefDbm + lowestPointDbr;

  uint32_t outerSlopeWidth = nGuardBands / 4;
  uint32_t middleSlopeWidth = outerSlopeWidth - (innerSlopeWidth / 2);
  WifiSpectrumBandIndices outerBandLeft(maskBand.first,
                                        maskBand.first + outerSlopeWidth - 1);
  WifiSpectrumBandIndices middleBandLeft(
      outerBandLeft.second + 1, outerBandLeft.second + middleSlopeWidth);
  WifiSpectrumBandIndices innerBandLeft(allocatedSubBands.front().first -
                                            innerSlopeWidth,
                                        allocatedSubBands.front().first - 1);
  WifiSpectrumBandIndices flatJunctionLeft(middleBandLeft.second + 1,
                                           innerBandLeft.first - 1);
  WifiSpectrumBandIndices outerBandRight(maskBand.second - outerSlopeWidth + 1,
                                         maskBand.second);
  WifiSpectrumBandIndices middleBandRight(
      outerBandRight.first - middleSlopeWidth, outerBandRight.first - 1);
  WifiSpectrumBandIndices innerBandRight(allocatedSubBands.back().second + 1,
                                         allocatedSubBands.back().second +
                                             innerSlopeWidth);
  WifiSpectrumBandIndices flatJunctionRight(innerBandRight.second + 1,
                                            middleBandRight.first - 1);
  std::ostringstream ss;
  ss << "outerBandLeft=[" << outerBandLeft.first << ";" << outerBandLeft.second
     << "] "
     << "middleBandLeft=[" << middleBandLeft.first << ";"
     << middleBandLeft.second << "] "
     << "flatJunctionLeft=[" << flatJunctionLeft.first << ";"
     << flatJunctionLeft.second << "] "
     << "innerBandLeft=[" << innerBandLeft.first << ";" << innerBandLeft.second
     << "] "
     << "subBands=[" << allocatedSubBands.front().first << ";"
     << allocatedSubBands.back().second << "] ";
  if (!puncturedBands.empty()) {
    ss << "puncturedBands=[" << puncturedBands.front().first << ";"
       << puncturedBands.back().second << "] ";
  }
  ss << "innerBandRight=[" << innerBandRight.first << ";"
     << innerBandRight.second << "] "
     << "flatJunctionRight=[" << flatJunctionRight.first << ";"
     << flatJunctionRight.second << "] "
     << "middleBandRight=[" << middleBandRight.first << ";"
     << middleBandRight.second << "] "
     << "outerBandRight=[" << outerBandRight.first << ";"
     << outerBandRight.second << "] ";
  NS_LOG_DEBUG(ss.str());
  NS_ASSERT(
      numMaskBands ==
      ((allocatedSubBands.back().second - allocatedSubBands.front().first + 1) +
       2 * (innerSlopeWidth + middleSlopeWidth + outerSlopeWidth) +
       (flatJunctionLeft.second - flatJunctionLeft.first + 1) +
       (flatJunctionRight.second - flatJunctionRight.first + 1)));

  double innerSlope = (-1 * minInnerBandDbr) / innerSlopeWidth;
  double middleSlope =
      (-1 * (minOuterBandDbr - minInnerBandDbr)) / middleSlopeWidth;
  double outerSlope =
      (txPowerMiddleBandMinDbm - txPowerOuterBandMinDbm) / outerSlopeWidth;
  double puncturedSlope = (-1 * minInnerBandDbr) / puncturedSlopeWidth;

  auto vit = c->ValuesBegin();
  auto bit = c->ConstBandsBegin();
  double txPowerW = 0.0;
  double previousTxPowerW = 0.0;
  for (size_t i = 0; i < numBands; i++, vit++, bit++) {
    if (i < maskBand.first || i > maskBand.second) {
      txPowerW = 0.0;
    } else if (i <= outerBandLeft.second && i >= outerBandLeft.first) {
      txPowerW = DbmToW(txPowerOuterBandMinDbm +
                        ((i - outerBandLeft.first) * outerSlope));
    } else if (i <= middleBandLeft.second && i >= middleBandLeft.first) {
      txPowerW = DbmToW(txPowerMiddleBandMinDbm +
                        ((i - middleBandLeft.first) * middleSlope));
    } else if (i <= flatJunctionLeft.second && i >= flatJunctionLeft.first) {
      txPowerW = DbmToW(txPowerInnerBandMinDbm);
    } else if (i <= innerBandLeft.second && i >= innerBandLeft.first) {
      txPowerW = (!puncturedBands.empty() && (puncturedBands.front().first <=
                                              allocatedSubBands.front().first))
                     ? DbmToW(txPowerInnerBandMinDbm)
                     : DbmToW(txPowerInnerBandMinDbm +
                              ((i - innerBandLeft.first) * innerSlope));
    } else if (i <= allocatedSubBands.back().second &&
               i >= allocatedSubBands.front().first) {
      bool insideSubBand = false;
      for (uint32_t j = 0; !insideSubBand && j < numSubBands; j++) {
        insideSubBand = (i <= allocatedSubBands[j].second) &&
                        (i >= allocatedSubBands[j].first);
      }
      if (insideSubBand) {
        bool insidePuncturedSubBand = false;
        uint32_t j = 0;
        for (; !insidePuncturedSubBand && j < puncturedBands.size(); j++) {
          insidePuncturedSubBand =
              (i <= puncturedBands[j].second) && (i >= puncturedBands[j].first);
        }
        if (insidePuncturedSubBand) {
          uint32_t startPuncturedSlope =
              (puncturedBands[puncturedBands.size() - 1].second -
               puncturedSlopeWidth);
          if (i >= startPuncturedSlope) {
            txPowerW = DbmToW(txPowerInnerBandMinDbm +
                              ((i - startPuncturedSlope) * puncturedSlope));
          } else {
            txPowerW =
                std::max(DbmToW(txPowerInnerBandMinDbm),
                         DbmToW(txPowerRefDbm - ((i - puncturedBands[0].first) *
                                                 puncturedSlope)));
          }
        } else {
          txPowerW = txPowerPerBandW;
        }
      } else {
        txPowerW = DbmToW(txPowerInnerBandMinDbm);
      }
    } else if (i <= innerBandRight.second && i >= innerBandRight.first) {
      txPowerW =
          std::min(previousTxPowerW,
                   DbmToW(txPowerRefDbm -
                          ((i - innerBandRight.first + 1) * innerSlope)));
    } else if (i <= flatJunctionRight.second && i >= flatJunctionRight.first) {
      txPowerW = DbmToW(txPowerInnerBandMinDbm);
    } else if (i <= middleBandRight.second && i >= middleBandRight.first) {
      txPowerW = DbmToW(txPowerInnerBandMinDbm -
                        ((i - middleBandRight.first + 1) * middleSlope));
    } else if (i <= outerBandRight.second && i >= outerBandRight.first) {
      txPowerW = DbmToW(txPowerMiddleBandMinDbm -
                        ((i - outerBandRight.first + 1) * outerSlope));
    } else {
      NS_FATAL_ERROR("Should have handled all cases");
    }
    double txPowerDbr = 10 * std::log10(txPowerW / txPowerPerBandW);
    NS_LOG_LOGIC(uint32_t(i) << " -> " << txPowerDbr);
    *vit = txPowerW / (bit->fh - bit->fl);
    previousTxPowerW = txPowerW;
  }
  NS_LOG_INFO("Added signal power to subbands "
              << allocatedSubBands.front().first << "-"
              << allocatedSubBands.back().second);
}

void WifiSpectrumValueHelper::NormalizeSpectrumMask(Ptr<SpectrumValue> c,
                                                    double txPowerW) {
  NS_LOG_FUNCTION(c << txPowerW);
  double currentTxPowerW = Integral(*c);
  double normalizationRatio = currentTxPowerW / txPowerW;
  NS_LOG_LOGIC("Current power: " << currentTxPowerW
                                 << "W vs expected power: " << txPowerW << "W"
                                 << " -> ratio (C/E) = " << normalizationRatio);
  auto vit = c->ValuesBegin();
  for (size_t i = 0; i < c->GetSpectrumModel()->GetNumBands(); i++, vit++) {
    *vit = (*vit) / normalizationRatio;
  }
}

double WifiSpectrumValueHelper::DbmToW(double dBm) {
  return std::pow(10.0, 0.1 * (dBm - 30.0));
}

double
WifiSpectrumValueHelper::GetBandPowerW(Ptr<SpectrumValue> psd,
                                       const WifiSpectrumBandIndices &band) {
  double powerWattPerHertz = 0.0;
  auto valueIt = psd->ConstValuesBegin() + band.first;
  auto end = psd->ConstValuesBegin() + band.second;
  auto bandIt = psd->ConstBandsBegin() + band.first;
  while (valueIt <= end) {
    powerWattPerHertz += *valueIt;
    ++valueIt;
  }
  return powerWattPerHertz * (bandIt->fh - bandIt->fl);
}

bool operator<(const FrequencyRange &left, const FrequencyRange &right) {
  return left.minFrequency < right.minFrequency;
}

bool operator==(const FrequencyRange &left, const FrequencyRange &right) {
  return (left.minFrequency == right.minFrequency) &&
         (left.maxFrequency == right.maxFrequency);
}

bool operator!=(const FrequencyRange &left, const FrequencyRange &right) {
  return !(left == right);
}

std::ostream &operator<<(std::ostream &os, const FrequencyRange &freqRange) {
  os << "[" << freqRange.minFrequency << " MHz - " << freqRange.maxFrequency
     << " MHz]";
  return os;
}

} // namespace ns3
