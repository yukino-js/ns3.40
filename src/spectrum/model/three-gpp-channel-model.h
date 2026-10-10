
#ifndef THREE_GPP_CHANNEL_H
#define THREE_GPP_CHANNEL_H

#include "matrix-based-channel-model.h"

#include "ns3/angles.h"
#include <ns3/boolean.h>
#include <ns3/channel-condition-model.h>

#include <complex.h>
#include <unordered_map>

namespace ns3 {

class MobilityModel;

class ThreeGppChannelModel : public MatrixBasedChannelModel {
public:
  ThreeGppChannelModel();

  ~ThreeGppChannelModel() override;

  void DoDispose() override;

  static TypeId GetTypeId();

  void SetChannelConditionModel(Ptr<ChannelConditionModel> model);

  Ptr<ChannelConditionModel> GetChannelConditionModel() const;

  void SetFrequency(double f);

  double GetFrequency() const;

  void SetScenario(const std::string &scenario);

  std::string GetScenario() const;

  Ptr<const ChannelMatrix>
  GetChannel(Ptr<const MobilityModel> aMob, Ptr<const MobilityModel> bMob,
             Ptr<const PhasedArrayModel> aAntenna,
             Ptr<const PhasedArrayModel> bAntenna) override;

  Ptr<const ChannelParams>
  GetParams(Ptr<const MobilityModel> aMob,
            Ptr<const MobilityModel> bMob) const override;
  int64_t AssignStreams(int64_t stream);

protected:
  static std::pair<double, double> WrapAngles(double azimuthRad,
                                              double inclinationRad);

  void Shuffle(double *first, double *last) const;

  struct ThreeGppChannelParams : public MatrixBasedChannelModel::ChannelParams {
    ChannelCondition::LosConditionValue m_losCondition;
    ChannelCondition::O2iConditionValue m_o2iCondition;
    MatrixBasedChannelModel::Double2DVector m_nonSelfBlocking;
    Vector m_preLocUT;
    Vector m_locUT;
    MatrixBasedChannelModel::Double2DVector m_norRvAngles;
    double m_DS;
    double m_K_factor;
    uint8_t m_reducedClusterNumber;
    MatrixBasedChannelModel::Double2DVector m_rayAodRadian;
    MatrixBasedChannelModel::Double2DVector m_rayAoaRadian;
    MatrixBasedChannelModel::Double2DVector m_rayZodRadian;
    MatrixBasedChannelModel::Double2DVector m_rayZoaRadian;
    MatrixBasedChannelModel::Double3DVector m_clusterPhase;
    MatrixBasedChannelModel::Double2DVector m_crossPolarizationPowerRatios;
    Vector m_speed;
    double m_dis2D;
    double m_dis3D;
    DoubleVector m_clusterPower;
    DoubleVector m_attenuation_dB;
    uint8_t m_cluster1st;
    uint8_t m_cluster2nd;
  };

  struct ParamsTable : public SimpleRefCount<ParamsTable> {
    uint8_t m_numOfCluster = 0;
    uint8_t m_raysPerCluster = 0;
    double m_uLgDS = 0;
    double m_sigLgDS = 0;
    double m_uLgASD = 0;
    double m_sigLgASD = 0;
    double m_uLgASA = 0;
    double m_sigLgASA = 0;
    double m_uLgZSA = 0;
    double m_sigLgZSA = 0;
    double m_uLgZSD = 0;
    double m_sigLgZSD = 0;
    double m_offsetZOD = 0;
    double m_cDS = 0;
    double m_cASD = 0;
    double m_cASA = 0;
    double m_cZSA = 0;
    double m_uK = 0;
    double m_sigK = 0;
    double m_rTau = 0;
    double m_uXpr = 0;
    double m_sigXpr = 0;
    double m_perClusterShadowingStd = 0;
    double m_sqrtC[7][7];
  };

  virtual Ptr<const ParamsTable>
  GetThreeGppTable(Ptr<const ChannelCondition> channelCondition, double hBS,
                   double hUT, double distance2D) const;

  Ptr<ThreeGppChannelParams>
  GenerateChannelParameters(const Ptr<const ChannelCondition> channelCondition,
                            const Ptr<const ParamsTable> table3gpp,
                            const Ptr<const MobilityModel> aMob,
                            const Ptr<const MobilityModel> bMob) const;

  virtual Ptr<ChannelMatrix> GetNewChannel(
      Ptr<const ThreeGppChannelParams> channelParams,
      Ptr<const ParamsTable> table3gpp, const Ptr<const MobilityModel> sMob,
      const Ptr<const MobilityModel> uMob, Ptr<const PhasedArrayModel> sAntenna,
      Ptr<const PhasedArrayModel> uAntenna) const;
  DoubleVector CalcAttenuationOfBlockage(
      const Ptr<ThreeGppChannelModel::ThreeGppChannelParams> channelParams,
      const DoubleVector &clusterAOA, const DoubleVector &clusterZOA) const;

  bool
  ChannelParamsNeedsUpdate(Ptr<const ThreeGppChannelParams> channelParams,
                           Ptr<const ChannelCondition> channelCondition) const;

  bool ChannelMatrixNeedsUpdate(Ptr<const ThreeGppChannelParams> channelParams,
                                Ptr<const ChannelMatrix> channelMatrix);

  std::unordered_map<uint64_t, Ptr<ChannelMatrix>> m_channelMatrixMap;
  std::unordered_map<uint64_t, Ptr<ThreeGppChannelParams>> m_channelParamsMap;
  Time m_updatePeriod;
  double m_frequency;
  std::string m_scenario;
  Ptr<ChannelConditionModel> m_channelConditionModel;
  Ptr<UniformRandomVariable> m_uniformRv;
  Ptr<NormalRandomVariable> m_normalRv;
  Ptr<UniformRandomVariable> m_uniformRvShuffle;

  double m_vScatt;
  Ptr<UniformRandomVariable> m_uniformRvDoppler;

  bool m_blockage;
  uint16_t m_numNonSelfBlocking;
  bool m_portraitMode;
  double m_blockerSpeed;

  static const uint8_t PHI_INDEX = 0;
  static const uint8_t X_INDEX = 1;
  static const uint8_t THETA_INDEX = 2;
  static const uint8_t Y_INDEX = 3;
  static const uint8_t R_INDEX = 4;
};
} // namespace ns3

#endif
