
#ifndef DSSS_ERROR_RATE_MODEL_H
#define DSSS_ERROR_RATE_MODEL_H

#include <cstdint>

namespace ns3 {

#ifdef HAVE_GSL
struct FunctionParameters {
  double beta;
  double n;
};

double IntegralFunction(double x, void *params);
#endif

class DsssErrorRateModel {
public:
  static double DqpskFunction(double x);
  static double GetDsssDbpskSuccessRate(double sinr, uint64_t nbits);
  static double GetDsssDqpskSuccessRate(double sinr, uint64_t nbits);
  static double GetDsssDqpskCck5_5SuccessRate(double sinr, uint64_t nbits);
  static double GetDsssDqpskCck11SuccessRate(double sinr, uint64_t nbits);
#ifdef HAVE_GSL
  static double SymbolErrorProb16Cck(double e2);
  static double SymbolErrorProb256Cck(double e1);
#else

protected:
  static const double WLAN_SIR_PERFECT;
  static const double WLAN_SIR_IMPOSSIBLE;
#endif
};

} // namespace ns3

#endif
