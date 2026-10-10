
#ifndef FQ_COBALT_QUEUE_DISC
#define FQ_COBALT_QUEUE_DISC

#include "queue-disc.h"

#include "ns3/object-factory.h"

#include <list>
#include <map>

namespace ns3 {

class FqCobaltFlow : public QueueDiscClass {
public:
  static TypeId GetTypeId();
  FqCobaltFlow();

  ~FqCobaltFlow() override;

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

class FqCobaltQueueDisc : public QueueDisc {
public:
  static TypeId GetTypeId();
  FqCobaltQueueDisc();

  ~FqCobaltQueueDisc() override;

  void SetQuantum(uint32_t quantum);

  uint32_t GetQuantum() const;

  static constexpr const char *UNCLASSIFIED_DROP = "Unclassified drop";
  static constexpr const char *OVERLIMIT_DROP = "Overlimit drop";

private:
  bool DoEnqueue(Ptr<QueueDiscItem> item) override;
  Ptr<QueueDiscItem> DoDequeue() override;
  bool CheckConfig() override;
  void InitializeParams() override;

  uint32_t FqCobaltDrop();

  uint32_t SetAssociativeHash(uint32_t flowHash);

  std::string m_interval;
  std::string m_target;
  uint32_t m_quantum;
  uint32_t m_flows;
  uint32_t m_setWays;
  uint32_t m_dropBatchSize;
  uint32_t m_perturbation;
  bool m_useEcn;
  Time m_ceThreshold;
  bool m_enableSetAssociativeHash;
  bool m_useL4s;
  double m_increment;
  double m_decrement;
  double m_Pdrop;
  Time m_blueThreshold;

  std::list<Ptr<FqCobaltFlow>> m_newFlows;
  std::list<Ptr<FqCobaltFlow>> m_oldFlows;

  std::map<uint32_t, uint32_t> m_flowsIndices;
  std::map<uint32_t, uint32_t> m_tags;

  ObjectFactory m_flowFactory;
  ObjectFactory m_queueDiscFactory;
};

} // namespace ns3

#endif
