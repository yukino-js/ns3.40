
#ifndef THREE_GPP_SPECTRUM_PROPAGATION_LOSS_H
#define THREE_GPP_SPECTRUM_PROPAGATION_LOSS_H

#include "matrix-based-channel-model.h"
#include "phased-array-spectrum-propagation-loss-model.h"

#include "ns3/random-variable-stream.h"

#include <complex.h>
#include <map>
#include <unordered_map>

namespace ns3 {

class NetDevice;

class ThreeGppSpectrumPropagationLossModel
    : public PhasedArraySpectrumPropagationLossModel {
public:
  ThreeGppSpectrumPropagationLossModel();

  ~ThreeGppSpectrumPropagationLossModel() override;

  void DoDispose() override;

  static TypeId GetTypeId();

  void SetChannelModel(Ptr<MatrixBasedChannelModel> channel);

  Ptr<MatrixBasedChannelModel> GetChannelModel() const;

  void SetChannelModelAttribute(const std::string &name,
                                const AttributeValue &value);

  void GetChannelModelAttribute(const std::string &name,
                                AttributeValue &value) const;

  Ptr<SpectrumValue> DoCalcRxPowerSpectralDensity(
      Ptr<const SpectrumSignalParameters> params, Ptr<const MobilityModel> a,
      Ptr<const MobilityModel> b, Ptr<const PhasedArrayModel> aPhasedArrayModel,
      Ptr<const PhasedArrayModel> bPhasedArrayModel) const override;

private:
  struct LongTerm : public SimpleRefCount<LongTerm> {
    PhasedArrayModel::ComplexVector m_longTerm;
    Ptr<const MatrixBasedChannelModel::ChannelMatrix> m_channel;
    PhasedArrayModel::ComplexVector m_sW;
    PhasedArrayModel::ComplexVector m_uW;
  };

  double GetFrequency() const;

  PhasedArrayModel::ComplexVector
  GetLongTerm(Ptr<const MatrixBasedChannelModel::ChannelMatrix> channelMatrix,
              Ptr<const PhasedArrayModel> aPhasedArrayModel,
              Ptr<const PhasedArrayModel> bPhasedArrayModel) const;
  PhasedArrayModel::ComplexVector
  CalcLongTerm(Ptr<const MatrixBasedChannelModel::ChannelMatrix> channelMatrix,
               const PhasedArrayModel::ComplexVector &sW,
               const PhasedArrayModel::ComplexVector &uW) const;

  Ptr<SpectrumValue> CalcBeamformingGain(
      Ptr<SpectrumValue> txPsd, PhasedArrayModel::ComplexVector longTerm,
      Ptr<const MatrixBasedChannelModel::ChannelMatrix> channelMatrix,
      Ptr<const MatrixBasedChannelModel::ChannelParams> channelParams,
      const Vector &sSpeed, const Vector &uSpeed) const;

  mutable std::unordered_map<uint64_t, Ptr<const LongTerm>> m_longTermMap;
  Ptr<MatrixBasedChannelModel> m_channelModel;
};
} // namespace ns3

#endif
