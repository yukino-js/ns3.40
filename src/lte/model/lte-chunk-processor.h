
#ifndef LTE_CHUNK_PROCESSOR_H
#define LTE_CHUNK_PROCESSOR_H

#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/ptr.h>

namespace ns3 {

class SpectrumValue;

typedef Callback<void, const SpectrumValue &> LteChunkProcessorCallback;

class LteChunkProcessor : public SimpleRefCount<LteChunkProcessor> {
public:
  LteChunkProcessor();
  virtual ~LteChunkProcessor();

  virtual void AddCallback(LteChunkProcessorCallback c);

  virtual void Start();

  virtual void EvaluateChunk(const SpectrumValue &sinr, Time duration);

  virtual void End();

private:
  Ptr<SpectrumValue> m_sumValues;
  Time m_totDuration;

  std::vector<LteChunkProcessorCallback> m_lteChunkProcessorCallbacks;
};

class LteSpectrumValueCatcher {
public:
  void ReportValue(const SpectrumValue &value);

  Ptr<SpectrumValue> GetValue();

private:
  Ptr<SpectrumValue> m_value;
};

} // namespace ns3

#endif
