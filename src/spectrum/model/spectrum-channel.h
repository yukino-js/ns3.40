
#ifndef SPECTRUM_CHANNEL_H
#define SPECTRUM_CHANNEL_H

#include "phased-array-spectrum-propagation-loss-model.h"
#include "spectrum-phy.h"
#include "spectrum-propagation-loss-model.h"
#include "spectrum-signal-parameters.h"
#include "spectrum-transmit-filter.h"

#include <ns3/channel.h>
#include <ns3/mobility-model.h>
#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/propagation-delay-model.h>
#include <ns3/propagation-loss-model.h>
#include <ns3/traced-callback.h>

namespace ns3 {

class PacketBurst;
class SpectrumValue;

class SpectrumChannel : public Channel {
public:
  SpectrumChannel();
  ~SpectrumChannel() override;

  void DoDispose() override;

  static TypeId GetTypeId();

  void AddPropagationLossModel(Ptr<PropagationLossModel> loss);

  void AddSpectrumPropagationLossModel(Ptr<SpectrumPropagationLossModel> loss);

  void AddPhasedArraySpectrumPropagationLossModel(
      Ptr<PhasedArraySpectrumPropagationLossModel> loss);

  void SetPropagationDelayModel(Ptr<PropagationDelayModel> delay);

  Ptr<SpectrumPropagationLossModel> GetSpectrumPropagationLossModel();

  Ptr<PhasedArraySpectrumPropagationLossModel>
  GetPhasedArraySpectrumPropagationLossModel();

  Ptr<PropagationLossModel> GetPropagationLossModel();

  void AddSpectrumTransmitFilter(Ptr<SpectrumTransmitFilter> filter);

  Ptr<const SpectrumTransmitFilter> GetSpectrumTransmitFilter() const;

  virtual void StartTx(Ptr<SpectrumSignalParameters> params) = 0;

  virtual void RemoveRx(Ptr<SpectrumPhy> phy) = 0;

  virtual void AddRx(Ptr<SpectrumPhy> phy) = 0;

  typedef void (*LossTracedCallback)(Ptr<const SpectrumPhy> txPhy,
                                     Ptr<const SpectrumPhy> rxPhy,
                                     double lossDb);
  typedef void (*GainTracedCallback)(Ptr<const MobilityModel> txMobility,
                                     Ptr<const MobilityModel> rxMobility,
                                     double txAntennaGain, double rxAntennaGain,
                                     double propagationGain, double pathloss);
  typedef void (*SignalParametersTracedCallback)(
      Ptr<SpectrumSignalParameters> params);

protected:
  TracedCallback<Ptr<const SpectrumPhy>, Ptr<const SpectrumPhy>, double>
      m_pathLossTrace;

  TracedCallback<Ptr<const MobilityModel>, Ptr<const MobilityModel>, double,
                 double, double, double>
      m_gainTrace;

  TracedCallback<Ptr<SpectrumSignalParameters>> m_txSigParamsTrace;

  double m_maxLossDb;

  Ptr<PropagationLossModel> m_propagationLoss;

  Ptr<PropagationDelayModel> m_propagationDelay;

  Ptr<SpectrumPropagationLossModel> m_spectrumPropagationLoss;

  Ptr<PhasedArraySpectrumPropagationLossModel>
      m_phasedArraySpectrumPropagationLoss;

  Ptr<SpectrumTransmitFilter> m_filter{nullptr};
};

} // namespace ns3

#endif
