
#include "yans-error-rate-model.h"

#include "wifi-tx-vector.h"
#include "wifi-utils.h"

#include "ns3/log.h"

#include <cmath>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("YansErrorRateModel");

NS_OBJECT_ENSURE_REGISTERED(YansErrorRateModel);

TypeId YansErrorRateModel::GetTypeId() {
  static TypeId tid = TypeId("ns3::YansErrorRateModel")
                          .SetParent<ErrorRateModel>()
                          .SetGroupName("Wifi")
                          .AddConstructor<YansErrorRateModel>();
  return tid;
}

YansErrorRateModel::YansErrorRateModel() {}

double YansErrorRateModel::GetBpskBer(double snr, uint32_t signalSpread,
                                      uint64_t phyRate) const {
  NS_LOG_FUNCTION(this << snr << signalSpread << phyRate);
  double EbNo = snr * signalSpread / phyRate;
  double z = std::sqrt(EbNo);
  double ber = 0.5 * erfc(z);
  NS_LOG_INFO("bpsk snr=" << snr << " ber=" << ber);
  return ber;
}

double YansErrorRateModel::GetQamBer(double snr, unsigned int m,
                                     uint32_t signalSpread,
                                     uint64_t phyRate) const {
  NS_LOG_FUNCTION(this << snr << m << signalSpread << phyRate);
  double EbNo = snr * signalSpread / phyRate;
  double z = std::sqrt((1.5 * log2(m) * EbNo) / (m - 1.0));
  double z1 = ((1.0 - 1.0 / std::sqrt(m)) * erfc(z));
  double z2 = 1 - std::pow((1 - z1), 2);
  double ber = z2 / log2(m);
  NS_LOG_INFO("Qam m=" << m << " rate=" << phyRate << " snr=" << snr
                       << " ber=" << ber);
  return ber;
}

uint32_t YansErrorRateModel::Factorial(uint32_t k) const {
  uint32_t fact = 1;
  while (k > 0) {
    fact *= k;
    k--;
  }
  return fact;
}

double YansErrorRateModel::Binomial(uint32_t k, double p, uint32_t n) const {
  double retval = Factorial(n) / (Factorial(k) * Factorial(n - k)) *
                  std::pow(p, static_cast<double>(k)) *
                  std::pow(1 - p, static_cast<double>(n - k));
  return retval;
}

double YansErrorRateModel::CalculatePdOdd(double ber, unsigned int d) const {
  NS_ASSERT((d % 2) == 1);
  unsigned int dstart = (d + 1) / 2;
  unsigned int dend = d;
  double pd = 0;

  for (unsigned int i = dstart; i < dend; i++) {
    pd += Binomial(i, ber, d);
  }
  return pd;
}

double YansErrorRateModel::CalculatePdEven(double ber, unsigned int d) const {
  NS_ASSERT((d % 2) == 0);
  unsigned int dstart = d / 2 + 1;
  unsigned int dend = d;
  double pd = 0;

  for (unsigned int i = dstart; i < dend; i++) {
    pd += Binomial(i, ber, d);
  }
  pd += 0.5 * Binomial(d / 2, ber, d);

  return pd;
}

double YansErrorRateModel::CalculatePd(double ber, unsigned int d) const {
  NS_LOG_FUNCTION(this << ber << d);
  double pd;
  if ((d % 2) == 0) {
    pd = CalculatePdEven(ber, d);
  } else {
    pd = CalculatePdOdd(ber, d);
  }
  return pd;
}

double YansErrorRateModel::GetFecBpskBer(double snr, uint64_t nbits,
                                         uint32_t signalSpread,
                                         uint64_t phyRate, uint32_t dFree,
                                         uint32_t adFree) const {
  NS_LOG_FUNCTION(this << snr << nbits << signalSpread << phyRate << dFree
                       << adFree);
  double ber = GetBpskBer(snr, signalSpread, phyRate);
  if (ber == 0.0) {
    return 1.0;
  }
  double pd = CalculatePd(ber, dFree);
  double pmu = adFree * pd;
  pmu = std::min(pmu, 1.0);
  double pms = std::pow(1 - pmu, nbits);
  return pms;
}

double YansErrorRateModel::GetFecQamBer(double snr, uint64_t nbits,
                                        uint32_t signalSpread, uint64_t phyRate,
                                        uint32_t m, uint32_t dFree,
                                        uint32_t adFree,
                                        uint32_t adFreePlusOne) const {
  NS_LOG_FUNCTION(this << snr << nbits << signalSpread << phyRate << m << dFree
                       << adFree << adFreePlusOne);
  double ber = GetQamBer(snr, m, signalSpread, phyRate);
  if (ber == 0.0) {
    return 1.0;
  }
  double pd = CalculatePd(ber, dFree);
  double pmu = adFree * pd;
  pd = CalculatePd(ber, dFree + 1);
  pmu += adFreePlusOne * pd;
  pmu = std::min(pmu, 1.0);
  double pms = std::pow(1 - pmu, nbits);
  return pms;
}

double YansErrorRateModel::DoGetChunkSuccessRate(
    WifiMode mode, const WifiTxVector &txVector, double snr, uint64_t nbits,
    uint8_t numRxAntennas, WifiPpduField field, uint16_t staId) const {
  NS_LOG_FUNCTION(this << mode << txVector << snr << nbits << +numRxAntennas
                       << field << staId);
  if (mode.GetModulationClass() >= WIFI_MOD_CLASS_ERP_OFDM) {
    uint64_t phyRate;
    if ((txVector.IsMu() && (staId == SU_STA_ID)) ||
        (mode != txVector.GetMode(staId))) {
      phyRate = mode.GetPhyRate(
          txVector.GetChannelWidth() >= 40 ? 20 : txVector.GetChannelWidth());
    } else {
      phyRate = mode.GetPhyRate(txVector, staId);
    }
    if (mode.GetConstellationSize() == 2) {
      if (mode.GetCodeRate() == WIFI_CODE_RATE_1_2) {
        return GetFecBpskBer(snr, nbits, txVector.GetChannelWidth() * 1000000,
                             phyRate, 10, 11);
      } else {
        return GetFecBpskBer(snr, nbits, txVector.GetChannelWidth() * 1000000,
                             phyRate, 5, 8);
      }
    } else if (mode.GetConstellationSize() == 4) {
      if (mode.GetCodeRate() == WIFI_CODE_RATE_1_2) {
        return GetFecQamBer(snr, nbits, txVector.GetChannelWidth() * 1000000,
                            phyRate, 4, 10, 11, 0);
      } else {
        return GetFecQamBer(snr, nbits, txVector.GetChannelWidth() * 1000000,
                            phyRate, 4, 5, 8, 31);
      }
    } else if (mode.GetConstellationSize() == 16) {
      if (mode.GetCodeRate() == WIFI_CODE_RATE_1_2) {
        return GetFecQamBer(snr, nbits, txVector.GetChannelWidth() * 1000000,
                            phyRate, 16, 10, 11, 0);
      } else {
        return GetFecQamBer(snr, nbits, txVector.GetChannelWidth() * 1000000,
                            phyRate, 16, 5, 8, 31);
      }
    } else if (mode.GetConstellationSize() == 64) {
      if (mode.GetCodeRate() == WIFI_CODE_RATE_2_3) {
        return GetFecQamBer(snr, nbits, txVector.GetChannelWidth() * 1000000,
                            phyRate, 64, 6, 1, 16);
      }
      if (mode.GetCodeRate() == WIFI_CODE_RATE_5_6) {
        return GetFecQamBer(snr, nbits, txVector.GetChannelWidth() * 1000000,
                            phyRate, 64, 4, 14, 69);
      } else {
        return GetFecQamBer(snr, nbits, txVector.GetChannelWidth() * 1000000,
                            phyRate, 64, 5, 8, 31);
      }
    } else if (mode.GetConstellationSize() == 256) {
      if (mode.GetCodeRate() == WIFI_CODE_RATE_5_6) {
        return GetFecQamBer(snr, nbits, txVector.GetChannelWidth() * 1000000,
                            phyRate, 256, 4, 14, 69);
      } else {
        return GetFecQamBer(snr, nbits, txVector.GetChannelWidth() * 1000000,
                            phyRate, 256, 5, 8, 31);
      }
    } else if (mode.GetConstellationSize() == 1024) {
      if (mode.GetCodeRate() == WIFI_CODE_RATE_5_6) {
        return GetFecQamBer(snr, nbits, txVector.GetChannelWidth() * 1000000,
                            phyRate, 1024, 4, 14, 69);
      } else {
        return GetFecQamBer(snr, nbits, txVector.GetChannelWidth() * 1000000,
                            phyRate, 1024, 5, 8, 31);
      }
    } else if (mode.GetConstellationSize() == 4096) {
      if (mode.GetCodeRate() == WIFI_CODE_RATE_5_6) {
        return GetFecQamBer(snr, nbits, txVector.GetChannelWidth() * 1000000,
                            phyRate, 4096, 4, 14, 69);
      } else {
        return GetFecQamBer(snr, nbits, txVector.GetChannelWidth() * 1000000,
                            phyRate, 4096, 5, 8, 31);
      }
    }
  }
  return 0;
}

} // namespace ns3
