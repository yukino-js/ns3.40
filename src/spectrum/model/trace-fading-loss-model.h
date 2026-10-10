
#ifndef TRACE_FADING_LOSS_MODEL_H
#define TRACE_FADING_LOSS_MODEL_H

#include "spectrum-propagation-loss-model.h"

#include "ns3/random-variable-stream.h"
#include <ns3/nstime.h>
#include <ns3/object.h>

#include <map>

namespace ns3 {

class MobilityModel;

class TraceFadingLossModel : public SpectrumPropagationLossModel {
public:
  TraceFadingLossModel();
  ~TraceFadingLossModel() override;

  static TypeId GetTypeId();

  void DoInitialize() override;

  typedef std::pair<Ptr<const MobilityModel>, Ptr<const MobilityModel>>
      ChannelRealizationId_t;

  int64_t AssignStreams(int64_t stream);

private:
  Ptr<SpectrumValue>
  DoCalcRxPowerSpectralDensity(Ptr<const SpectrumSignalParameters> params,
                               Ptr<const MobilityModel> a,
                               Ptr<const MobilityModel> b) const override;

  double GetValue(int subChannel, double speed);

  void SetTraceFileName(std::string fileName);
  void SetTraceLength(Time t);

  void LoadTrace();

  mutable std::map<ChannelRealizationId_t, int> m_windowOffsetsMap;

  mutable std::map<ChannelRealizationId_t, Ptr<UniformRandomVariable>>
      m_startVariableMap;

  typedef std::vector<double> FadingTraceSample;
  typedef std::vector<FadingTraceSample> FadingTrace;

  std::string m_traceFile;

  FadingTrace m_fadingTrace;

  Time m_traceLength;
  uint32_t m_samplesNum;
  Time m_windowSize;
  uint32_t m_rbNum;
  mutable Time m_lastWindowUpdate;
  uint32_t m_timeGranularity;
  mutable uint64_t m_currentStream;
  mutable uint64_t m_lastStream;
  uint64_t m_streamSetSize;
  mutable bool m_streamsAssigned;
};

} // namespace ns3

#endif
