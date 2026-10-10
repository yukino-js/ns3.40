
#ifndef LTE_INTERFERENCE_H
#define LTE_INTERFERENCE_H

#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/packet.h>
#include <ns3/spectrum-value.h>

#include <list>

namespace ns3 {

class LteChunkProcessor;

class LteInterference : public Object {
public:
  LteInterference();
  ~LteInterference() override;

  static TypeId GetTypeId();
  void DoDispose() override;

  virtual void AddSinrChunkProcessor(Ptr<LteChunkProcessor> p);

  virtual void AddInterferenceChunkProcessor(Ptr<LteChunkProcessor> p);

  virtual void AddRsPowerChunkProcessor(Ptr<LteChunkProcessor> p);

  virtual void StartRx(Ptr<const SpectrumValue> rxPsd);

  virtual void EndRx();

  virtual void AddSignal(Ptr<const SpectrumValue> spd, const Time duration);

  virtual void SetNoisePowerSpectralDensity(Ptr<const SpectrumValue> noisePsd);

protected:
  virtual void ConditionallyEvaluateChunk();
  virtual void DoAddSignal(Ptr<const SpectrumValue> spd);
  virtual void DoSubtractSignal(Ptr<const SpectrumValue> spd,
                                uint32_t signalId);

  bool m_receiving{false};

  Ptr<SpectrumValue> m_rxSignal{nullptr};

  Ptr<SpectrumValue> m_allSignals{nullptr};

  Ptr<const SpectrumValue> m_noise{nullptr};

  Time m_lastChangeTime{Seconds(0)};

  uint32_t m_lastSignalId{0};
  uint32_t m_lastSignalIdBeforeReset{0};

  std::list<Ptr<LteChunkProcessor>> m_rsPowerChunkProcessorList;

  std::list<Ptr<LteChunkProcessor>> m_sinrChunkProcessorList;

  std::list<Ptr<LteChunkProcessor>> m_interfChunkProcessorList;
};

} // namespace ns3

#endif
