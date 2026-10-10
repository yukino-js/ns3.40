

#ifndef RED_QUEUE_DISC_H
#define RED_QUEUE_DISC_H

#include "queue-disc.h"

#include "ns3/boolean.h"
#include "ns3/data-rate.h"
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

class TraceContainer;

class RedQueueDisc : public QueueDisc {
public:
  static TypeId GetTypeId();
  RedQueueDisc();

  ~RedQueueDisc() override;

  enum FengStatus {
    Above,
    Between,
    Below,
  };

  enum {
    DTYPE_NONE,
    DTYPE_FORCED,
    DTYPE_UNFORCED,
  };

  void SetAredAlpha(double alpha);

  double GetAredAlpha();

  void SetAredBeta(double beta);

  double GetAredBeta();

  void SetFengAdaptiveA(double a);

  double GetFengAdaptiveA();

  void SetFengAdaptiveB(double b);

  double GetFengAdaptiveB();

  void SetTh(double minTh, double maxTh);

  int64_t AssignStreams(int64_t stream);

  static constexpr const char *UNFORCED_DROP = "Unforced drop";
  static constexpr const char *FORCED_DROP = "Forced drop";
  static constexpr const char *UNFORCED_MARK = "Unforced mark";
  static constexpr const char *FORCED_MARK = "Forced mark";

protected:
  void DoDispose() override;

private:
  bool DoEnqueue(Ptr<QueueDiscItem> item) override;
  Ptr<QueueDiscItem> DoDequeue() override;
  Ptr<const QueueDiscItem> DoPeek() override;
  bool CheckConfig() override;

  void InitializeParams() override;
  double Estimator(uint32_t nQueued, uint32_t m, double qAvg, double qW);
  void UpdateMaxP(double newAve);
  void UpdateMaxPFeng(double newAve);
  uint32_t DropEarly(Ptr<QueueDiscItem> item, uint32_t qSize);
  double CalculatePNew();
  double ModifyP(double p, uint32_t size);

  uint32_t m_meanPktSize;
  uint32_t m_idlePktSize;
  bool m_isWait;
  bool m_isGentle;
  bool m_isARED;
  bool m_isAdaptMaxP;
  double m_minTh;
  double m_maxTh;
  double m_qW;
  double m_lInterm;
  Time m_targetDelay;
  Time m_interval;
  double m_top;
  double m_bottom;
  double m_alpha;
  double m_beta;
  Time m_rtt;
  bool m_isFengAdaptive;
  bool m_isNonlinear;
  double m_b;
  double m_a;
  bool m_isNs1Compat;
  DataRate m_linkBandwidth;
  Time m_linkDelay;
  bool m_useEcn;
  bool m_useHardDrop;

  double m_vA;
  double m_vB;
  double m_vC;
  double m_vD;
  double m_curMaxP;
  Time m_lastSet;
  double m_vProb;
  uint32_t m_countBytes;
  uint32_t m_old;
  uint32_t m_idle;
  double m_ptc;
  double m_qAvg;
  uint32_t m_count;
  FengStatus m_fengStatus;
  uint32_t m_cautious;
  Time m_idleTime;

  Ptr<UniformRandomVariable> m_uv;
};

}; // namespace ns3

#endif
