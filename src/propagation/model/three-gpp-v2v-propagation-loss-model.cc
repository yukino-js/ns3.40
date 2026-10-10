
#include "three-gpp-v2v-propagation-loss-model.h"

#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/string.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("ThreeGppV2vPropagationLossModel");

NS_OBJECT_ENSURE_REGISTERED(ThreeGppV2vUrbanPropagationLossModel);

TypeId ThreeGppV2vUrbanPropagationLossModel::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::ThreeGppV2vUrbanPropagationLossModel")
          .SetParent<ThreeGppPropagationLossModel>()
          .SetGroupName("Propagation")
          .AddConstructor<ThreeGppV2vUrbanPropagationLossModel>()
          .AddAttribute(
              "PercType3Vehicles",
              "The percentage of vehicles of type 3 (i.e., trucks) in the "
              "scenario",
              DoubleValue(0.0),
              MakeDoubleAccessor(
                  &ThreeGppV2vUrbanPropagationLossModel::m_percType3Vehicles),
              MakeDoubleChecker<double>(0.0, 100.0));
  return tid;
}

ThreeGppV2vUrbanPropagationLossModel::ThreeGppV2vUrbanPropagationLossModel()
    : ThreeGppPropagationLossModel() {
  NS_LOG_FUNCTION(this);
  m_uniformVar = CreateObject<UniformRandomVariable>();
  m_logNorVar = CreateObject<LogNormalRandomVariable>();
}

ThreeGppV2vUrbanPropagationLossModel::~ThreeGppV2vUrbanPropagationLossModel() {
  NS_LOG_FUNCTION(this);
}

double ThreeGppV2vUrbanPropagationLossModel::GetLossLos(double,
                                                        double distance3D,
                                                        double, double) const {
  NS_LOG_FUNCTION(this);

  double loss =
      38.77 + 16.7 * log10(distance3D) + 18.2 * log10(m_frequency / 1e9);

  return loss;
}

double ThreeGppV2vUrbanPropagationLossModel::GetO2iDistance2dIn() const {
  NS_LOG_WARN("O2I car penetration loss not yet implemented");
  return 0;
}

double ThreeGppV2vUrbanPropagationLossModel::GetLossNlosv(double distance2D,
                                                          double distance3D,
                                                          double hUt,
                                                          double hBs) const {
  NS_LOG_FUNCTION(this);

  double loss = GetLossLos(distance2D, distance3D, hUt, hBs) +
                GetAdditionalNlosvLoss(distance3D, hUt, hBs);

  return loss;
}

double ThreeGppV2vUrbanPropagationLossModel::GetAdditionalNlosvLoss(
    double distance3D, double hUt, double hBs) const {
  NS_LOG_FUNCTION(this);
  double additionalLoss = 0;
  double blockerHeight = 0;
  double mu_a = 0;
  double sigma_a = 0;
  double randomValue = m_uniformVar->GetValue() * 100.0;
  if (randomValue < m_percType3Vehicles) {
    blockerHeight = 3.0;
  } else {
    blockerHeight = 1.6;
  }

  if (std::min(hUt, hBs) > blockerHeight) {
    additionalLoss = 0;
  } else if (std::max(hUt, hBs) < blockerHeight) {
    mu_a = 9.0 + std::max(0.0, 15 * log10(distance3D) - 41.0);
    sigma_a = 4.5;
    m_logNorVar->SetAttribute(
        "Mu",
        DoubleValue(log(pow(mu_a, 2) / sqrt(pow(sigma_a, 2) + pow(mu_a, 2)))));
    m_logNorVar->SetAttribute(
        "Sigma", DoubleValue(sqrt(log(pow(sigma_a, 2) / pow(mu_a, 2) + 1))));
    additionalLoss = std::max(0.0, m_logNorVar->GetValue());
  } else {
    mu_a = 5.0 + std::max(0.0, 15 * log10(distance3D) - 41.0);
    sigma_a = 4.0;

    m_logNorVar->SetAttribute(
        "Mu",
        DoubleValue(log(pow(mu_a, 2) / sqrt(pow(sigma_a, 2) + pow(mu_a, 2)))));
    m_logNorVar->SetAttribute(
        "Sigma", DoubleValue(sqrt(log(pow(sigma_a, 2) / pow(mu_a, 2) + 1))));
    additionalLoss = std::max(0.0, m_logNorVar->GetValue());
  }

  return additionalLoss;
}

double ThreeGppV2vUrbanPropagationLossModel::GetLossNlos(double,
                                                         double distance3D,
                                                         double, double) const {
  NS_LOG_FUNCTION(this);

  double loss =
      36.85 + 30 * log10(distance3D) + 18.9 * log10(m_frequency / 1e9);

  return loss;
}

double ThreeGppV2vUrbanPropagationLossModel::GetShadowingStd(
    Ptr<MobilityModel>, Ptr<MobilityModel>,
    ChannelCondition::LosConditionValue cond) const {
  NS_LOG_FUNCTION(this);
  double shadowingStd;

  if (cond == ChannelCondition::LosConditionValue::LOS ||
      cond == ChannelCondition::LosConditionValue::NLOSv) {
    shadowingStd = 3.0;
  } else if (cond == ChannelCondition::LosConditionValue::NLOS) {
    shadowingStd = 4.0;
  } else {
    NS_FATAL_ERROR("Unknown channel condition");
  }

  return shadowingStd;
}

double ThreeGppV2vUrbanPropagationLossModel::GetShadowingCorrelationDistance(
    ChannelCondition::LosConditionValue cond) const {
  NS_LOG_FUNCTION(this);
  double correlationDistance;

  if (cond == ChannelCondition::LosConditionValue::LOS) {
    correlationDistance = 10;
  } else if (cond == ChannelCondition::LosConditionValue::NLOSv ||
             cond == ChannelCondition::LosConditionValue::NLOS) {
    correlationDistance = 13;
  } else {
    NS_FATAL_ERROR("Unknown channel condition");
  }

  return correlationDistance;
}

int64_t ThreeGppV2vUrbanPropagationLossModel::DoAssignStreams(int64_t stream) {
  NS_LOG_FUNCTION(this);

  m_normRandomVariable->SetStream(stream);
  m_uniformVar->SetStream(stream + 1);
  m_logNorVar->SetStream(stream + 2);
  return 3;
}

NS_OBJECT_ENSURE_REGISTERED(ThreeGppV2vHighwayPropagationLossModel);

TypeId ThreeGppV2vHighwayPropagationLossModel::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::ThreeGppV2vHighwayPropagationLossModel")
          .SetParent<ThreeGppV2vUrbanPropagationLossModel>()
          .SetGroupName("Propagation")
          .AddConstructor<ThreeGppV2vHighwayPropagationLossModel>();
  return tid;
}

ThreeGppV2vHighwayPropagationLossModel::ThreeGppV2vHighwayPropagationLossModel()
    : ThreeGppV2vUrbanPropagationLossModel() {
  NS_LOG_FUNCTION(this);
}

ThreeGppV2vHighwayPropagationLossModel::
    ~ThreeGppV2vHighwayPropagationLossModel() {
  NS_LOG_FUNCTION(this);
}

double ThreeGppV2vHighwayPropagationLossModel::GetLossLos(double,
                                                          double distance3D,
                                                          double,
                                                          double) const {
  NS_LOG_FUNCTION(this);

  double loss = 32.4 + 20 * log10(distance3D) + 20 * log10(m_frequency / 1e9);

  return loss;
}

} // namespace ns3
