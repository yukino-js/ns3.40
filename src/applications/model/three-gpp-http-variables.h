
#ifndef THREE_GPP_HTTP_VARIABLES_H
#define THREE_GPP_HTTP_VARIABLES_H

#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/random-variable-stream.h>

namespace ns3 {

class ThreeGppHttpVariables : public Object {
public:
  ThreeGppHttpVariables();

  static TypeId GetTypeId();

  uint32_t GetMtuSize();

  uint32_t GetRequestSize();

  Time GetMainObjectGenerationDelay();

  uint32_t GetMainObjectSize();

  Time GetEmbeddedObjectGenerationDelay();

  uint32_t GetEmbeddedObjectSize();

  uint32_t GetNumOfEmbeddedObjects();

  Time GetReadingTime();

  Time GetParsingTime();

  int64_t AssignStreams(int64_t stream);

  void SetRequestSize(uint32_t constant);
  void SetMainObjectGenerationDelay(Time constant);
  void SetMainObjectSizeMean(uint32_t mean);
  void SetMainObjectSizeStdDev(uint32_t stdDev);
  void SetEmbeddedObjectGenerationDelay(Time constant);
  void SetEmbeddedObjectSizeMean(uint32_t mean);
  void SetEmbeddedObjectSizeStdDev(uint32_t stdDev);
  void SetNumOfEmbeddedObjectsMax(uint32_t max);
  void SetNumOfEmbeddedObjectsShape(double shape);
  void SetNumOfEmbeddedObjectsScale(uint32_t scale);
  void SetReadingTimeMean(Time mean);
  void SetParsingTimeMean(Time mean);

private:
  void UpdateMainObjectMuAndSigma();
  void UpdateEmbeddedObjectMuAndSigma();

  void DoInitialize() override;

  Ptr<UniformRandomVariable> m_mtuSizeRng;
  Ptr<ConstantRandomVariable> m_requestSizeRng;
  Ptr<ConstantRandomVariable> m_mainObjectGenerationDelayRng;
  Ptr<LogNormalRandomVariable> m_mainObjectSizeRng;
  uint32_t m_mainObjectSizeMean;
  uint32_t m_mainObjectSizeStdDev;
  uint32_t m_mainObjectSizeMin;
  uint32_t m_mainObjectSizeMax;
  uint32_t m_lowMtu;
  uint32_t m_highMtu;
  double m_highMtuProbability;
  Ptr<ConstantRandomVariable> m_embeddedObjectGenerationDelayRng;
  Ptr<LogNormalRandomVariable> m_embeddedObjectSizeRng;
  uint32_t m_embeddedObjectSizeMean;
  uint32_t m_embeddedObjectSizeStdDev;
  uint32_t m_embeddedObjectSizeMin;
  uint32_t m_embeddedObjectSizeMax;
  Ptr<ParetoRandomVariable> m_numOfEmbeddedObjectsRng;
  uint32_t m_numOfEmbeddedObjectsScale;
  Ptr<ExponentialRandomVariable> m_readingTimeRng;
  Ptr<ExponentialRandomVariable> m_parsingTimeRng;
};

} // namespace ns3

#endif
