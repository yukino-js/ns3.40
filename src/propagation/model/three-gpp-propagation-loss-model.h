
#ifndef THREE_GPP_PROPAGATION_LOSS_MODEL_H
#define THREE_GPP_PROPAGATION_LOSS_MODEL_H

#include "channel-condition-model.h"
#include "propagation-loss-model.h"

namespace ns3 {

class ThreeGppPropagationLossModel : public PropagationLossModel {
public:
  static TypeId GetTypeId();

  ThreeGppPropagationLossModel();

  ~ThreeGppPropagationLossModel() override;

  ThreeGppPropagationLossModel(const ThreeGppPropagationLossModel &) = delete;
  ThreeGppPropagationLossModel &
  operator=(const ThreeGppPropagationLossModel &) = delete;

  void SetChannelConditionModel(Ptr<ChannelConditionModel> model);

  Ptr<ChannelConditionModel> GetChannelConditionModel() const;

  void SetFrequency(double f);

  double GetFrequency() const;

  bool IsO2iLowPenetrationLoss(Ptr<const ChannelCondition> cond) const;

private:
  double DoCalcRxPower(double txPowerDbm, Ptr<MobilityModel> a,
                       Ptr<MobilityModel> b) const override;

  int64_t DoAssignStreams(int64_t stream) override;

  double GetLoss(Ptr<ChannelCondition> cond, double distance2D,
                 double distance3D, double hUt, double hBs) const;

  virtual double GetLossLos(double distance2D, double distance3D, double hUt,
                            double hBs) const = 0;

  virtual double GetO2iDistance2dIn() const = 0;

  virtual double
  GetO2iLowPenetrationLoss(Ptr<MobilityModel> a, Ptr<MobilityModel> b,
                           ChannelCondition::LosConditionValue cond) const;

  virtual double
  GetO2iHighPenetrationLoss(Ptr<MobilityModel> a, Ptr<MobilityModel> b,
                            ChannelCondition::LosConditionValue cond) const;

  virtual bool
  DoIsO2iLowPenetrationLoss(Ptr<const ChannelCondition> cond) const;

  virtual double GetLossNlos(double distance2D, double distance3D, double hUt,
                             double hBs) const = 0;

  virtual double GetLossNlosv(double distance2D, double distance3D, double hUt,
                              double hBs) const;

  virtual std::pair<double, double> GetUtAndBsHeights(double za,
                                                      double zb) const;

  double GetShadowing(Ptr<MobilityModel> a, Ptr<MobilityModel> b,
                      ChannelCondition::LosConditionValue cond) const;

  virtual double
  GetShadowingStd(Ptr<MobilityModel> a, Ptr<MobilityModel> b,
                  ChannelCondition::LosConditionValue cond) const = 0;

  virtual double GetShadowingCorrelationDistance(
      ChannelCondition::LosConditionValue cond) const = 0;

  static uint32_t GetKey(Ptr<MobilityModel> a, Ptr<MobilityModel> b);

  static Vector GetVectorDifference(Ptr<MobilityModel> a, Ptr<MobilityModel> b);

protected:
  void DoDispose() override;

  static double Calculate2dDistance(Vector a, Vector b);

  Ptr<ChannelConditionModel> m_channelConditionModel;
  double m_frequency;
  bool m_shadowingEnabled;
  bool m_enforceRanges;
  bool m_buildingPenLossesEnabled;
  Ptr<NormalRandomVariable> m_normRandomVariable;

  struct ShadowingMapItem {
    double m_shadowing;
    ChannelCondition::LosConditionValue m_condition;
    Vector m_distance;
  };

  mutable std::unordered_map<uint32_t, ShadowingMapItem> m_shadowingMap;

  struct O2iLossMapItem {
    double m_o2iLoss;
    ChannelCondition::LosConditionValue m_condition;
  };

  mutable std::unordered_map<uint32_t, O2iLossMapItem> m_o2iLossMap;

  Ptr<UniformRandomVariable> m_randomO2iVar1;
  Ptr<UniformRandomVariable> m_randomO2iVar2;
  Ptr<NormalRandomVariable> m_normalO2iLowLossVar;
  Ptr<NormalRandomVariable> m_normalO2iHighLossVar;
};

class ThreeGppRmaPropagationLossModel : public ThreeGppPropagationLossModel {
public:
  static TypeId GetTypeId();

  ThreeGppRmaPropagationLossModel();

  ~ThreeGppRmaPropagationLossModel() override;

  ThreeGppRmaPropagationLossModel(const ThreeGppRmaPropagationLossModel &) =
      delete;
  ThreeGppRmaPropagationLossModel &
  operator=(const ThreeGppRmaPropagationLossModel &) = delete;

private:
  double GetLossLos(double distance2D, double distance3D, double hUt,
                    double hBs) const override;

  double GetO2iDistance2dIn() const override;

  bool
  DoIsO2iLowPenetrationLoss(Ptr<const ChannelCondition> cond) const override;

  double GetLossNlos(double distance2D, double distance3D, double hUt,
                     double hBs) const override;

  double
  GetShadowingStd(Ptr<MobilityModel> a, Ptr<MobilityModel> b,
                  ChannelCondition::LosConditionValue cond) const override;

  double GetShadowingCorrelationDistance(
      ChannelCondition::LosConditionValue cond) const override;

  static double Pl1(double frequency, double distance3D, double h, double w);

  static double GetBpDistance(double frequency, double hA, double hB);

  double m_h;
  double m_w;
};

class ThreeGppUmaPropagationLossModel : public ThreeGppPropagationLossModel {
public:
  static TypeId GetTypeId();

  ThreeGppUmaPropagationLossModel();

  ~ThreeGppUmaPropagationLossModel() override;

  ThreeGppUmaPropagationLossModel(const ThreeGppUmaPropagationLossModel &) =
      delete;
  ThreeGppUmaPropagationLossModel &
  operator=(const ThreeGppUmaPropagationLossModel &) = delete;

private:
  int64_t DoAssignStreams(int64_t stream) override;

  double GetLossLos(double distance2D, double distance3D, double hUt,
                    double hBs) const override;

  double GetO2iDistance2dIn() const override;

  double GetLossNlos(double distance2D, double distance3D, double hUt,
                     double hBs) const override;

  double
  GetShadowingStd(Ptr<MobilityModel> a, Ptr<MobilityModel> b,
                  ChannelCondition::LosConditionValue cond) const override;

  double GetShadowingCorrelationDistance(
      ChannelCondition::LosConditionValue cond) const override;

  double GetBpDistance(double hUt, double hBs, double distance2D) const;

  Ptr<UniformRandomVariable> m_uniformVar;
};

class ThreeGppUmiStreetCanyonPropagationLossModel
    : public ThreeGppPropagationLossModel {
public:
  static TypeId GetTypeId();

  ThreeGppUmiStreetCanyonPropagationLossModel();

  ~ThreeGppUmiStreetCanyonPropagationLossModel() override;

  ThreeGppUmiStreetCanyonPropagationLossModel(
      const ThreeGppUmiStreetCanyonPropagationLossModel &) = delete;
  ThreeGppUmiStreetCanyonPropagationLossModel &
  operator=(const ThreeGppUmiStreetCanyonPropagationLossModel &) = delete;

private:
  double GetLossLos(double distance2D, double distance3D, double hUt,
                    double hBs) const override;

  double GetO2iDistance2dIn() const override;

  double GetLossNlos(double distance2D, double distance3D, double hUt,
                     double hBs) const override;

  double
  GetShadowingStd(Ptr<MobilityModel> a, Ptr<MobilityModel> b,
                  ChannelCondition::LosConditionValue cond) const override;

  double GetShadowingCorrelationDistance(
      ChannelCondition::LosConditionValue cond) const override;

  double GetBpDistance(double hUt, double hBs, double distance2D) const;

  std::pair<double, double> GetUtAndBsHeights(double za,
                                              double zb) const override;
};

class ThreeGppIndoorOfficePropagationLossModel
    : public ThreeGppPropagationLossModel {
public:
  static TypeId GetTypeId();

  ThreeGppIndoorOfficePropagationLossModel();

  ~ThreeGppIndoorOfficePropagationLossModel() override;

  ThreeGppIndoorOfficePropagationLossModel(
      const ThreeGppIndoorOfficePropagationLossModel &) = delete;
  ThreeGppIndoorOfficePropagationLossModel &
  operator=(const ThreeGppIndoorOfficePropagationLossModel &) = delete;

private:
  double GetLossLos(double distance2D, double distance3D, double hUt,
                    double hBs) const override;

  double GetO2iDistance2dIn() const override;

  double GetLossNlos(double distance2D, double distance3D, double hUt,
                     double hBs) const override;

  double
  GetShadowingStd(Ptr<MobilityModel> a, Ptr<MobilityModel> b,
                  ChannelCondition::LosConditionValue cond) const override;

  double GetShadowingCorrelationDistance(
      ChannelCondition::LosConditionValue cond) const override;
};

} // namespace ns3

#endif
