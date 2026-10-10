
#ifndef CHANNEL_CONDITION_MODEL_H
#define CHANNEL_CONDITION_MODEL_H

#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/random-variable-stream.h"
#include "ns3/vector.h"

#include <unordered_map>

namespace ns3 {

class MobilityModel;

class ChannelCondition : public Object {
public:
  enum LosConditionValue { LOS, NLOS, NLOSv, LC_ND };

  enum O2iConditionValue { O2O, O2I, I2I, O2I_ND };

  enum O2iLowHighConditionValue { LOW, HIGH, LH_O2I_ND };

  static TypeId GetTypeId();

  ChannelCondition();

  ChannelCondition(LosConditionValue losCondition,
                   O2iConditionValue o2iCondition = O2O,
                   O2iLowHighConditionValue o2iLowHighCondition = LOW);

  ~ChannelCondition() override;

  LosConditionValue GetLosCondition() const;

  void SetLosCondition(LosConditionValue losCondition);

  O2iConditionValue GetO2iCondition() const;

  void SetO2iCondition(O2iConditionValue o2iCondition);

  O2iLowHighConditionValue GetO2iLowHighCondition() const;

  void SetO2iLowHighCondition(O2iLowHighConditionValue o2iLowHighCondition);

  bool IsLos() const;

  bool IsNlos() const;

  bool IsNlosv() const;

  bool IsO2i() const;

  bool IsO2o() const;

  bool IsI2i() const;

  bool IsEqual(LosConditionValue losCondition,
               O2iConditionValue o2iCondition) const;

private:
  LosConditionValue m_losCondition;
  O2iConditionValue m_o2iCondition;
  O2iLowHighConditionValue m_o2iLowHighCondition;

  friend std::ostream &operator<<(std::ostream &os, LosConditionValue cond);
};

class ChannelConditionModel : public Object {
public:
  static TypeId GetTypeId();

  ChannelConditionModel();

  ~ChannelConditionModel() override;

  ChannelConditionModel(const ChannelConditionModel &) = delete;
  ChannelConditionModel &operator=(const ChannelConditionModel &) = delete;

  virtual Ptr<ChannelCondition>
  GetChannelCondition(Ptr<const MobilityModel> a,
                      Ptr<const MobilityModel> b) const = 0;

  virtual int64_t AssignStreams(int64_t stream) = 0;
};

class AlwaysLosChannelConditionModel : public ChannelConditionModel {
public:
  static TypeId GetTypeId();

  AlwaysLosChannelConditionModel();

  ~AlwaysLosChannelConditionModel() override;

  AlwaysLosChannelConditionModel(const AlwaysLosChannelConditionModel &) =
      delete;
  AlwaysLosChannelConditionModel &
  operator=(const AlwaysLosChannelConditionModel &) = delete;

  Ptr<ChannelCondition>
  GetChannelCondition(Ptr<const MobilityModel> a,
                      Ptr<const MobilityModel> b) const override;

  int64_t AssignStreams(int64_t stream) override;
};

class NeverLosChannelConditionModel : public ChannelConditionModel {
public:
  static TypeId GetTypeId();

  NeverLosChannelConditionModel();

  ~NeverLosChannelConditionModel() override;

  NeverLosChannelConditionModel(const NeverLosChannelConditionModel &) = delete;
  NeverLosChannelConditionModel &
  operator=(const NeverLosChannelConditionModel &) = delete;

  Ptr<ChannelCondition>
  GetChannelCondition(Ptr<const MobilityModel> a,
                      Ptr<const MobilityModel> b) const override;

  int64_t AssignStreams(int64_t stream) override;
};

class NeverLosVehicleChannelConditionModel : public ChannelConditionModel {
public:
  static TypeId GetTypeId();

  NeverLosVehicleChannelConditionModel();

  ~NeverLosVehicleChannelConditionModel() override;

  NeverLosVehicleChannelConditionModel(
      const NeverLosVehicleChannelConditionModel &) = delete;
  NeverLosVehicleChannelConditionModel &
  operator=(const NeverLosVehicleChannelConditionModel &) = delete;

  Ptr<ChannelCondition>
  GetChannelCondition(Ptr<const MobilityModel> a,
                      Ptr<const MobilityModel> b) const override;

  int64_t AssignStreams(int64_t stream) override;
};

class ThreeGppChannelConditionModel : public ChannelConditionModel {
public:
  static TypeId GetTypeId();

  ThreeGppChannelConditionModel();

  ~ThreeGppChannelConditionModel() override;

  Ptr<ChannelCondition>
  GetChannelCondition(Ptr<const MobilityModel> a,
                      Ptr<const MobilityModel> b) const override;

  int64_t AssignStreams(int64_t stream) override;

protected:
  void DoDispose() override;

  enum VehicleDensity { LOW, MEDIUM, HIGH, INVALID };

  static double Calculate2dDistance(const Vector &a, const Vector &b);

  Ptr<UniformRandomVariable> m_uniformVar;

private:
  Ptr<ChannelCondition>
  ComputeChannelCondition(Ptr<const MobilityModel> a,
                          Ptr<const MobilityModel> b) const;

  virtual double ComputePlos(Ptr<const MobilityModel> a,
                             Ptr<const MobilityModel> b) const = 0;

  virtual ChannelCondition::O2iConditionValue
  ComputeO2i(Ptr<const MobilityModel> a, Ptr<const MobilityModel> b) const;

  virtual double ComputePnlos(Ptr<const MobilityModel> a,
                              Ptr<const MobilityModel> b) const;

  static uint32_t GetKey(Ptr<const MobilityModel> a,
                         Ptr<const MobilityModel> b);

  struct Item {
    Ptr<ChannelCondition> m_condition;
    Time m_generatedTime;
  };

  std::unordered_map<uint32_t, Item> m_channelConditionMap;
  Time m_updatePeriod;

  double m_o2iThreshold{0};
  double m_o2iLowLossThreshold{0};
  bool m_linkO2iConditionToAntennaHeight{false};
  Ptr<UniformRandomVariable> m_uniformVarO2i;
  Ptr<UniformRandomVariable> m_uniformO2iLowHighLossVar;
};

class ThreeGppRmaChannelConditionModel : public ThreeGppChannelConditionModel {
public:
  static TypeId GetTypeId();

  ThreeGppRmaChannelConditionModel();

  ~ThreeGppRmaChannelConditionModel() override;

private:
  double ComputePlos(Ptr<const MobilityModel> a,
                     Ptr<const MobilityModel> b) const override;
};

class ThreeGppUmaChannelConditionModel : public ThreeGppChannelConditionModel {
public:
  static TypeId GetTypeId();

  ThreeGppUmaChannelConditionModel();

  ~ThreeGppUmaChannelConditionModel() override;

private:
  double ComputePlos(Ptr<const MobilityModel> a,
                     Ptr<const MobilityModel> b) const override;
};

class ThreeGppUmiStreetCanyonChannelConditionModel
    : public ThreeGppChannelConditionModel {
public:
  static TypeId GetTypeId();

  ThreeGppUmiStreetCanyonChannelConditionModel();

  ~ThreeGppUmiStreetCanyonChannelConditionModel() override;

private:
  double ComputePlos(Ptr<const MobilityModel> a,
                     Ptr<const MobilityModel> b) const override;
};

class ThreeGppIndoorMixedOfficeChannelConditionModel
    : public ThreeGppChannelConditionModel {
public:
  static TypeId GetTypeId();

  ThreeGppIndoorMixedOfficeChannelConditionModel();

  ~ThreeGppIndoorMixedOfficeChannelConditionModel() override;

private:
  double ComputePlos(Ptr<const MobilityModel> a,
                     Ptr<const MobilityModel> b) const override;
};

class ThreeGppIndoorOpenOfficeChannelConditionModel
    : public ThreeGppChannelConditionModel {
public:
  static TypeId GetTypeId();

  ThreeGppIndoorOpenOfficeChannelConditionModel();

  ~ThreeGppIndoorOpenOfficeChannelConditionModel() override;

private:
  double ComputePlos(Ptr<const MobilityModel> a,
                     Ptr<const MobilityModel> b) const override;
};

} // namespace ns3

#endif
