

#ifndef RTT_ESTIMATOR_H
#define RTT_ESTIMATOR_H

#include "ns3/nstime.h"
#include "ns3/object.h"

namespace ns3 {

class RttEstimator : public Object {
public:
  static TypeId GetTypeId();

  RttEstimator();
  RttEstimator(const RttEstimator &r);

  ~RttEstimator() override;

  TypeId GetInstanceTypeId() const override;

  virtual void Measurement(Time t) = 0;

  virtual Ptr<RttEstimator> Copy() const = 0;

  virtual void Reset();

  Time GetEstimate() const;

  Time GetVariation() const;

  uint32_t GetNSamples() const;

private:
  Time m_initialEstimatedRtt;

protected:
  Time m_estimatedRtt;
  Time m_estimatedVariation;
  uint32_t m_nSamples;
};

class RttMeanDeviation : public RttEstimator {
public:
  static TypeId GetTypeId();

  RttMeanDeviation();

  RttMeanDeviation(const RttMeanDeviation &r);

  TypeId GetInstanceTypeId() const override;

  void Measurement(Time measure) override;

  Ptr<RttEstimator> Copy() const override;

  void Reset() override;

private:
  uint32_t CheckForReciprocalPowerOfTwo(double val) const;
  void IntegerUpdate(Time m, uint32_t rttShift, uint32_t variationShift);
  void FloatingPointUpdate(Time m);
  double m_alpha;
  double m_beta;
};

} // namespace ns3

#endif
