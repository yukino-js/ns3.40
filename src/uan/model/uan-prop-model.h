
#ifndef UAN_PROP_MODEL_H
#define UAN_PROP_MODEL_H

#include "ns3/mobility-model.h"
#include "ns3/nstime.h"
#include "ns3/object.h"

#include <complex>
#include <utility>
#include <vector>

namespace ns3 {

class UanTxMode;

class Tap {
public:
  Tap();
  Tap(Time delay, std::complex<double> amp);
  std::complex<double> GetAmp() const;
  Time GetDelay() const;

private:
  std::complex<double> m_amplitude;
  Time m_delay;
};

class UanPdp {
public:
  typedef std::vector<Tap>::const_iterator Iterator;
  UanPdp();
  UanPdp(std::vector<Tap> taps, Time resolution);
  UanPdp(std::vector<std::complex<double>> arrivals, Time resolution);
  UanPdp(std::vector<double> arrivals, Time resolution);
  ~UanPdp();

  void SetTap(std::complex<double> arrival, uint32_t index);
  void SetNTaps(uint32_t nTaps);
  void SetResolution(Time resolution);
  Iterator GetBegin() const;
  Iterator GetEnd() const;
  uint32_t GetNTaps() const;
  const Tap &GetTap(uint32_t i) const;
  Time GetResolution() const;
  double SumTapsNc(Time begin, Time end) const;
  std::complex<double> SumTapsC(Time begin, Time end) const;
  double SumTapsFromMaxNc(Time delay, Time duration) const;
  std::complex<double> SumTapsFromMaxC(Time delay, Time duration) const;
  UanPdp NormalizeToSumNc() const;
  static UanPdp CreateImpulsePdp();

private:
  friend std::ostream &operator<<(std::ostream &os, const UanPdp &pdp);
  friend std::istream &operator>>(std::istream &is, UanPdp &pdp);

  std::vector<Tap> m_taps;
  Time m_resolution;
};

std::ostream &operator<<(std::ostream &os, const UanPdp &pdp);
std::istream &operator>>(std::istream &is, UanPdp &pdp);

class UanPropModel : public Object {
public:
  static TypeId GetTypeId();

  virtual double GetPathLossDb(Ptr<MobilityModel> a, Ptr<MobilityModel> b,
                               UanTxMode txMode) = 0;

  virtual UanPdp GetPdp(Ptr<MobilityModel> a, Ptr<MobilityModel> b,
                        UanTxMode mode) = 0;
  virtual Time GetDelay(Ptr<MobilityModel> a, Ptr<MobilityModel> b,
                        UanTxMode mode) = 0;

  virtual void Clear();

  void DoDispose() override;
};

} // namespace ns3

#endif
