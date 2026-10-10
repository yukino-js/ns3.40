
#ifndef PROPAGATION_LOSS_MODEL_H
#define PROPAGATION_LOSS_MODEL_H

#include "ns3/object.h"
#include "ns3/random-variable-stream.h"

#include <map>

namespace ns3 {

class MobilityModel;

class PropagationLossModel : public Object {
public:
  static TypeId GetTypeId();

  PropagationLossModel();
  ~PropagationLossModel() override;

  PropagationLossModel(const PropagationLossModel &) = delete;
  PropagationLossModel &operator=(const PropagationLossModel &) = delete;

  void SetNext(Ptr<PropagationLossModel> next);

  Ptr<PropagationLossModel> GetNext();

  double CalcRxPower(double txPowerDbm, Ptr<MobilityModel> a,
                     Ptr<MobilityModel> b) const;

  int64_t AssignStreams(int64_t stream);

protected:
  virtual int64_t DoAssignStreams(int64_t stream) = 0;

private:
  virtual double DoCalcRxPower(double txPowerDbm, Ptr<MobilityModel> a,
                               Ptr<MobilityModel> b) const = 0;

  Ptr<PropagationLossModel> m_next;
};

class RandomPropagationLossModel : public PropagationLossModel {
public:
  static TypeId GetTypeId();

  RandomPropagationLossModel();
  ~RandomPropagationLossModel() override;

  RandomPropagationLossModel(const RandomPropagationLossModel &) = delete;
  RandomPropagationLossModel &
  operator=(const RandomPropagationLossModel &) = delete;

private:
  double DoCalcRxPower(double txPowerDbm, Ptr<MobilityModel> a,
                       Ptr<MobilityModel> b) const override;
  int64_t DoAssignStreams(int64_t stream) override;

  Ptr<RandomVariableStream> m_variable;
};

class FriisPropagationLossModel : public PropagationLossModel {
public:
  static TypeId GetTypeId();
  FriisPropagationLossModel();

  FriisPropagationLossModel(const FriisPropagationLossModel &) = delete;
  FriisPropagationLossModel &
  operator=(const FriisPropagationLossModel &) = delete;

  void SetFrequency(double frequency);
  void SetSystemLoss(double systemLoss);

  void SetMinLoss(double minLoss);

  double GetMinLoss() const;

  double GetFrequency() const;
  double GetSystemLoss() const;

private:
  double DoCalcRxPower(double txPowerDbm, Ptr<MobilityModel> a,
                       Ptr<MobilityModel> b) const override;
  int64_t DoAssignStreams(int64_t stream) override;

  double DbmToW(double dbm) const;

  double DbmFromW(double w) const;

  double m_lambda;
  double m_frequency;
  double m_systemLoss;
  double m_minLoss;
};

class TwoRayGroundPropagationLossModel : public PropagationLossModel {
public:
  static TypeId GetTypeId();
  TwoRayGroundPropagationLossModel();

  TwoRayGroundPropagationLossModel(const TwoRayGroundPropagationLossModel &) =
      delete;
  TwoRayGroundPropagationLossModel &
  operator=(const TwoRayGroundPropagationLossModel &) = delete;

  void SetFrequency(double frequency);

  void SetSystemLoss(double systemLoss);
  void SetMinDistance(double minDistance);
  double GetMinDistance() const;

  double GetFrequency() const;

  double GetSystemLoss() const;
  void SetHeightAboveZ(double heightAboveZ);

private:
  double DoCalcRxPower(double txPowerDbm, Ptr<MobilityModel> a,
                       Ptr<MobilityModel> b) const override;
  int64_t DoAssignStreams(int64_t stream) override;

  double DbmToW(double dbm) const;

  double DbmFromW(double w) const;

  double m_lambda;
  double m_frequency;
  double m_systemLoss;
  double m_minDistance;
  double m_heightAboveZ;
};

class LogDistancePropagationLossModel : public PropagationLossModel {
public:
  static TypeId GetTypeId();
  LogDistancePropagationLossModel();

  LogDistancePropagationLossModel(const LogDistancePropagationLossModel &) =
      delete;
  LogDistancePropagationLossModel &
  operator=(const LogDistancePropagationLossModel &) = delete;

  void SetPathLossExponent(double n);
  double GetPathLossExponent() const;

  void SetReference(double referenceDistance, double referenceLoss);

private:
  double DoCalcRxPower(double txPowerDbm, Ptr<MobilityModel> a,
                       Ptr<MobilityModel> b) const override;

  int64_t DoAssignStreams(int64_t stream) override;

  static Ptr<PropagationLossModel> CreateDefaultReference();

  double m_exponent;
  double m_referenceDistance;
  double m_referenceLoss;
};

class ThreeLogDistancePropagationLossModel : public PropagationLossModel {
public:
  static TypeId GetTypeId();
  ThreeLogDistancePropagationLossModel();

  ThreeLogDistancePropagationLossModel(
      const ThreeLogDistancePropagationLossModel &) = delete;
  ThreeLogDistancePropagationLossModel &
  operator=(const ThreeLogDistancePropagationLossModel &) = delete;

private:
  double DoCalcRxPower(double txPowerDbm, Ptr<MobilityModel> a,
                       Ptr<MobilityModel> b) const override;

  int64_t DoAssignStreams(int64_t stream) override;

  double m_distance0;
  double m_distance1;
  double m_distance2;

  double m_exponent0;
  double m_exponent1;
  double m_exponent2;

  double m_referenceLoss;
};

class NakagamiPropagationLossModel : public PropagationLossModel {
public:
  static TypeId GetTypeId();

  NakagamiPropagationLossModel();

  NakagamiPropagationLossModel(const NakagamiPropagationLossModel &) = delete;
  NakagamiPropagationLossModel &
  operator=(const NakagamiPropagationLossModel &) = delete;

private:
  double DoCalcRxPower(double txPowerDbm, Ptr<MobilityModel> a,
                       Ptr<MobilityModel> b) const override;

  int64_t DoAssignStreams(int64_t stream) override;

  double m_distance1;
  double m_distance2;

  double m_m0;
  double m_m1;
  double m_m2;

  Ptr<ErlangRandomVariable> m_erlangRandomVariable;
  Ptr<GammaRandomVariable> m_gammaRandomVariable;
};

class FixedRssLossModel : public PropagationLossModel {
public:
  static TypeId GetTypeId();

  FixedRssLossModel();
  ~FixedRssLossModel() override;

  FixedRssLossModel(const FixedRssLossModel &) = delete;
  FixedRssLossModel &operator=(const FixedRssLossModel &) = delete;

  void SetRss(double rss);

private:
  double DoCalcRxPower(double txPowerDbm, Ptr<MobilityModel> a,
                       Ptr<MobilityModel> b) const override;

  int64_t DoAssignStreams(int64_t stream) override;

  double m_rss;
};

class MatrixPropagationLossModel : public PropagationLossModel {
public:
  static TypeId GetTypeId();

  MatrixPropagationLossModel();
  ~MatrixPropagationLossModel() override;

  MatrixPropagationLossModel(const MatrixPropagationLossModel &) = delete;
  MatrixPropagationLossModel &
  operator=(const MatrixPropagationLossModel &) = delete;

  void SetLoss(Ptr<MobilityModel> a, Ptr<MobilityModel> b, double loss,
               bool symmetric = true);

  void SetDefaultLoss(double defaultLoss);

private:
  double DoCalcRxPower(double txPowerDbm, Ptr<MobilityModel> a,
                       Ptr<MobilityModel> b) const override;

  int64_t DoAssignStreams(int64_t stream) override;

  double m_default;

  typedef std::pair<Ptr<MobilityModel>, Ptr<MobilityModel>> MobilityPair;

  std::map<MobilityPair, double> m_loss;
};

class RangePropagationLossModel : public PropagationLossModel {
public:
  static TypeId GetTypeId();
  RangePropagationLossModel();

  RangePropagationLossModel(const RangePropagationLossModel &) = delete;
  RangePropagationLossModel &
  operator=(const RangePropagationLossModel &) = delete;

private:
  double DoCalcRxPower(double txPowerDbm, Ptr<MobilityModel> a,
                       Ptr<MobilityModel> b) const override;

  int64_t DoAssignStreams(int64_t stream) override;

  double m_range;
};

} // namespace ns3

#endif
