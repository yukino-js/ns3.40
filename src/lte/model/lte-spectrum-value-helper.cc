
#include "lte-spectrum-value-helper.h"

#include <ns3/fatal-error.h>
#include <ns3/log.h>

#include <cmath>
#include <map>

namespace std {

ostream &operator<<(ostream &os, const vector<int> &v) {
  auto it = v.begin();
  while (it != v.end()) {
    os << *it << " ";
    ++it;
  }
  os << endl;
  return os;
}

} // namespace std

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("LteSpectrumValueHelper");

struct EutraChannelNumbers {
  uint8_t band;
  double fDlLow;
  uint32_t nOffsDl;
  uint32_t rangeNdl1;
  uint32_t rangeNdl2;
  double fUlLow;
  uint32_t nOffsUl;
  uint32_t rangeNul1;
  uint32_t rangeNul2;
};

static const EutraChannelNumbers g_eutraChannelNumbers[]{
    {1, 2110, 0, 0, 599, 1920, 18000, 18000, 18599},
    {2, 1930, 600, 600, 1199, 1850, 18600, 18600, 19199},
    {3, 1805, 1200, 1200, 1949, 1710, 19200, 19200, 19949},
    {4, 2110, 1950, 1950, 2399, 1710, 19950, 19950, 20399},
    {5, 869, 2400, 2400, 2649, 824, 20400, 20400, 20649},
    {6, 875, 2650, 2650, 2749, 830, 20650, 20650, 20749},
    {7, 2620, 2750, 2750, 3449, 2500, 20750, 20750, 21449},
    {8, 925, 3450, 3450, 3799, 880, 21450, 21450, 21799},
    {9, 1844.9, 3800, 3800, 4149, 1749.9, 21800, 21800, 22149},
    {10, 2110, 4150, 4150, 4749, 1710, 22150, 22150, 22749},
    {11, 1475.9, 4750, 4750, 4949, 1427.9, 22750, 22750, 22949},
    {12, 728, 5000, 5000, 5179, 698, 23000, 23000, 23179},
    {13, 746, 5180, 5180, 5279, 777, 23180, 23180, 23279},
    {14, 758, 5280, 5280, 5379, 788, 23280, 23280, 23379},
    {17, 734, 5730, 5730, 5849, 704, 23730, 23730, 23849},
    {18, 860, 5850, 5850, 5999, 815, 23850, 23850, 23999},
    {19, 875, 6000, 6000, 6149, 830, 24000, 24000, 24149},
    {20, 791, 6150, 6150, 6449, 832, 24150, 24150, 24449},
    {21, 1495.9, 6450, 6450, 6599, 1447.9, 24450, 24450, 24599},
    {33, 1900, 36000, 36000, 36199, 1900, 36000, 36000, 36199},
    {34, 2010, 36200, 36200, 36349, 2010, 36200, 36200, 36349},
    {35, 1850, 36350, 36350, 36949, 1850, 36350, 36350, 36949},
    {36, 1930, 36950, 36950, 37549, 1930, 36950, 36950, 37549},
    {37, 1910, 37550, 37550, 37749, 1910, 37550, 37550, 37749},
    {38, 2570, 37750, 37750, 38249, 2570, 37750, 37750, 38249},
    {39, 1880, 38250, 38250, 38649, 1880, 38250, 38250, 38649},
    {40, 2300, 38650, 38650, 39649, 2300, 38650, 38650, 39649},
};

#define NUM_EUTRA_BANDS                                                        \
  (sizeof(g_eutraChannelNumbers) / sizeof(EutraChannelNumbers))

double LteSpectrumValueHelper::GetCarrierFrequency(uint32_t earfcn) {
  NS_LOG_FUNCTION(earfcn);
  if (earfcn < 7000) {
    return GetDownlinkCarrierFrequency(earfcn);
  } else {
    return GetUplinkCarrierFrequency(earfcn);
  }
}

uint16_t LteSpectrumValueHelper::GetDownlinkCarrierBand(uint32_t nDl) {
  NS_LOG_FUNCTION(nDl);
  for (uint32_t i = 0; i < NUM_EUTRA_BANDS; ++i) {
    if (g_eutraChannelNumbers[i].rangeNdl1 <= nDl &&
        g_eutraChannelNumbers[i].rangeNdl2 >= nDl) {
      NS_LOG_LOGIC("entry " << i
                            << " fDlLow=" << g_eutraChannelNumbers[i].fDlLow);
      return i;
    }
  }
  NS_LOG_ERROR("invalid EARFCN " << nDl);
  return NUM_EUTRA_BANDS;
}

uint16_t LteSpectrumValueHelper::GetUplinkCarrierBand(uint32_t nUl) {
  NS_LOG_FUNCTION(nUl);
  for (uint32_t i = 0; i < NUM_EUTRA_BANDS; ++i) {
    if (g_eutraChannelNumbers[i].rangeNul1 <= nUl &&
        g_eutraChannelNumbers[i].rangeNul2 >= nUl) {
      NS_LOG_LOGIC("entry " << i
                            << " fUlLow=" << g_eutraChannelNumbers[i].fUlLow);
      return i;
    }
  }
  NS_LOG_ERROR("invalid EARFCN " << nUl);
  return NUM_EUTRA_BANDS;
}

double LteSpectrumValueHelper::GetDownlinkCarrierFrequency(uint32_t nDl) {
  NS_LOG_FUNCTION(nDl);
  uint16_t i = GetDownlinkCarrierBand(nDl);
  if (i == NUM_EUTRA_BANDS) {
    return 0.0;
  }
  return 1.0e6 * (g_eutraChannelNumbers[i].fDlLow +
                  0.1 * (nDl - g_eutraChannelNumbers[i].nOffsDl));
}

double LteSpectrumValueHelper::GetUplinkCarrierFrequency(uint32_t nUl) {
  NS_LOG_FUNCTION(nUl);
  uint16_t i = GetUplinkCarrierBand(nUl);
  if (i == NUM_EUTRA_BANDS) {
    return 0.0;
  }
  return 1.0e6 * (g_eutraChannelNumbers[i].fUlLow +
                  0.1 * (nUl - g_eutraChannelNumbers[i].nOffsUl));
}

double
LteSpectrumValueHelper::GetChannelBandwidth(uint16_t transmissionBandwidth) {
  NS_LOG_FUNCTION(transmissionBandwidth);
  switch (transmissionBandwidth) {
  case 6:
    return 1.4e6;
  case 15:
    return 3.0e6;
  case 25:
    return 5.0e6;
  case 50:
    return 10.0e6;
  case 75:
    return 15.0e6;
  case 100:
    return 20.0e6;
  default:
    NS_FATAL_ERROR("invalid bandwidth value " << transmissionBandwidth);
  }
}

struct LteSpectrumModelId {
  LteSpectrumModelId(uint32_t f, uint8_t b);
  uint32_t earfcn;
  uint16_t bandwidth;
};

LteSpectrumModelId::LteSpectrumModelId(uint32_t f, uint8_t b)
    : earfcn(f), bandwidth(b) {}

bool operator<(const LteSpectrumModelId &a, const LteSpectrumModelId &b) {
  return ((a.earfcn < b.earfcn) ||
          ((a.earfcn == b.earfcn) && (a.bandwidth < b.bandwidth)));
}

static std::map<LteSpectrumModelId, Ptr<SpectrumModel>> g_lteSpectrumModelMap;

Ptr<SpectrumModel>
LteSpectrumValueHelper::GetSpectrumModel(uint32_t earfcn,
                                         uint16_t txBandwidthConfiguration) {
  NS_LOG_FUNCTION(earfcn << txBandwidthConfiguration);
  Ptr<SpectrumModel> ret;
  LteSpectrumModelId key(earfcn, txBandwidthConfiguration);
  auto it = g_lteSpectrumModelMap.find(key);
  if (it != g_lteSpectrumModelMap.end()) {
    ret = it->second;
  } else {
    double fc = GetCarrierFrequency(earfcn);
    NS_ASSERT_MSG(fc != 0, "invalid EARFCN=" << earfcn);

    double f = fc - (txBandwidthConfiguration * 180e3 / 2.0);
    Bands rbs;
    for (uint16_t numrb = 0; numrb < txBandwidthConfiguration; ++numrb) {
      BandInfo rb;
      rb.fl = f;
      f += 90e3;
      rb.fc = f;
      f += 90e3;
      rb.fh = f;
      rbs.push_back(rb);
    }
    ret = Create<SpectrumModel>(rbs);
    g_lteSpectrumModelMap.insert(
        std::pair<LteSpectrumModelId, Ptr<SpectrumModel>>(key, ret));
  }
  NS_LOG_LOGIC("returning SpectrumModel::GetUid () == " << ret->GetUid());
  return ret;
}

Ptr<SpectrumValue> LteSpectrumValueHelper::CreateTxPowerSpectralDensity(
    uint32_t earfcn, uint16_t txBandwidthConfiguration, double powerTx,
    std::vector<int> activeRbs) {
  NS_LOG_FUNCTION(earfcn << txBandwidthConfiguration << powerTx << activeRbs);

  Ptr<SpectrumModel> model = GetSpectrumModel(earfcn, txBandwidthConfiguration);
  Ptr<SpectrumValue> txPsd = Create<SpectrumValue>(model);

  double powerTxW = std::pow(10., (powerTx - 30) / 10);

  double txPowerDensity = (powerTxW / (txBandwidthConfiguration * 180000));

  for (auto it = activeRbs.begin(); it != activeRbs.end(); it++) {
    int rbId = (*it);
    (*txPsd)[rbId] = txPowerDensity;
  }

  NS_LOG_LOGIC(*txPsd);

  return txPsd;
}

Ptr<SpectrumValue> LteSpectrumValueHelper::CreateTxPowerSpectralDensity(
    uint32_t earfcn, uint16_t txBandwidthConfiguration, double powerTx,
    std::map<int, double> powerTxMap, std::vector<int> activeRbs) {
  NS_LOG_FUNCTION(earfcn << txBandwidthConfiguration << activeRbs);

  Ptr<SpectrumModel> model = GetSpectrumModel(earfcn, txBandwidthConfiguration);
  Ptr<SpectrumValue> txPsd = Create<SpectrumValue>(model);

  double basicPowerTxW = std::pow(10., (powerTx - 30) / 10);

  for (auto it = activeRbs.begin(); it != activeRbs.end(); it++) {
    int rbId = (*it);

    auto powerIt = powerTxMap.find(rbId);

    double txPowerDensity;

    if (powerIt != powerTxMap.end()) {
      double powerTxW = std::pow(10., (powerIt->second - 30) / 10);
      txPowerDensity = (powerTxW / (txBandwidthConfiguration * 180000));
    } else {
      txPowerDensity = (basicPowerTxW / (txBandwidthConfiguration * 180000));
    }

    (*txPsd)[rbId] = txPowerDensity;
  }

  NS_LOG_LOGIC(*txPsd);

  return txPsd;
}

Ptr<SpectrumValue> LteSpectrumValueHelper::CreateUlTxPowerSpectralDensity(
    uint16_t earfcn, uint16_t txBandwidthConfiguration, double powerTx,
    std::vector<int> activeRbs) {
  NS_LOG_FUNCTION(earfcn << txBandwidthConfiguration << powerTx << activeRbs);

  Ptr<SpectrumModel> model = GetSpectrumModel(earfcn, txBandwidthConfiguration);
  Ptr<SpectrumValue> txPsd = Create<SpectrumValue>(model);

  double powerTxW = std::pow(10., (powerTx - 30) / 10);

  double txPowerDensity = (powerTxW / (activeRbs.size() * 180000));

  for (auto it = activeRbs.begin(); it != activeRbs.end(); it++) {
    int rbId = (*it);
    (*txPsd)[rbId] = txPowerDensity;
  }

  NS_LOG_LOGIC(*txPsd);

  return txPsd;
}

Ptr<SpectrumValue> LteSpectrumValueHelper::CreateNoisePowerSpectralDensity(
    uint32_t earfcn, uint16_t txBandwidthConfiguration, double noiseFigure) {
  NS_LOG_FUNCTION(earfcn << txBandwidthConfiguration << noiseFigure);
  Ptr<SpectrumModel> model = GetSpectrumModel(earfcn, txBandwidthConfiguration);
  return CreateNoisePowerSpectralDensity(noiseFigure, model);
}

Ptr<SpectrumValue> LteSpectrumValueHelper::CreateNoisePowerSpectralDensity(
    double noiseFigureDb, Ptr<SpectrumModel> spectrumModel) {
  NS_LOG_FUNCTION(noiseFigureDb << spectrumModel);

  const double kT_dBm_Hz = -174.0;
  double kT_W_Hz = std::pow(10.0, (kT_dBm_Hz - 30) / 10.0);
  double noiseFigureLinear = std::pow(10.0, noiseFigureDb / 10.0);
  double noisePowerSpectralDensity = kT_W_Hz * noiseFigureLinear;

  Ptr<SpectrumValue> noisePsd = Create<SpectrumValue>(spectrumModel);
  (*noisePsd) = noisePowerSpectralDensity;
  return noisePsd;
}

} // namespace ns3
