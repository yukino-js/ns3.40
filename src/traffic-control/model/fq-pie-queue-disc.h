
#ifndef FQ_PIE_QUEUE_DISC
#define FQ_PIE_QUEUE_DISC

#include "queue-disc.h"

#include "ns3/object-factory.h"

#include <list>
#include <map>

namespace ns3 {

class FqPieFlow : public QueueDiscClass {
public:
  static TypeId GetTypeId();
  FqPieFlow();

  ~FqPieFlow() override;

  enum FlowStatus { INACTIVE, NEW_FLOW, OLD_FLOW };

  void SetDeficit(uint32_t deficit);
  int32_t GetDeficit() const;
  void IncreaseDeficit(int32_t deficit);
  void SetStatus(FlowStatus status);
  FlowStatus GetStatus() const;
  void SetIndex(uint32_t index);
  uint32_t GetIndex() const;

private:
  int32_t m_deficit;
  FlowStatus m_status;
  uint32_t m_index;
};

class FqPieQueueDisc : public QueueDisc {
public:
  static TypeId GetTypeId();
  FqPieQueueDisc();

  ~FqPieQueueDisc() override;

  void SetQuantum(uint32_t quantum);

  uint32_t GetQuantum() const;

  static constexpr const char *UNCLASSIFIED_DROP = "Unclassified drop";
  static constexpr const char *OVERLIMIT_DROP = "Overlimit drop";

private:
  bool DoEnqueue(Ptr<QueueDiscItem> item) override;
  Ptr<QueueDiscItem> DoDequeue() override;
  bool CheckConfig() override;
  void InitializeParams() override;

  uint32_t FqPieDrop();

  uint32_t SetAssociativeHash(uint32_t flowHash);

  bool m_useEcn;
  double m_markEcnTh;
  Time m_ceThreshold;
  bool m_useL4s;
  Time m_sUpdate;
  Time m_tUpdate;
  Time m_qDelayRef;
  uint32_t m_meanPktSize;
  Time m_maxBurst;
  double m_a;
  double m_b;
  uint32_t m_dqThreshold;
  bool m_useDqRateEstimator;
  bool m_isCapDropAdjustment;
  bool m_useDerandomization;

  uint32_t m_quantum;
  uint32_t m_flows;
  uint32_t m_setWays;
  uint32_t m_dropBatchSize;
  uint32_t m_perturbation;
  bool m_enableSetAssociativeHash;

  std::list<Ptr<FqPieFlow>> m_newFlows;
  std::list<Ptr<FqPieFlow>> m_oldFlows;

  std::map<uint32_t, uint32_t> m_flowsIndices;
  std::map<uint32_t, uint32_t> m_tags;

  ObjectFactory m_flowFactory;
  ObjectFactory m_queueDiscFactory;
};

} // namespace ns3

#endif
