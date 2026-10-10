
#ifndef TCPBBR_H
#define TCPBBR_H

#include "tcp-congestion-ops.h"
#include "windowed-filter.h"

#include "ns3/data-rate.h"
#include "ns3/random-variable-stream.h"
#include "ns3/traced-value.h"

class TcpBbrCheckGainValuesTest;

namespace ns3 {

class TcpBbr : public TcpCongestionOps {
public:
  static const uint8_t GAIN_CYCLE_LENGTH = 8;

  const static double PACING_GAIN_CYCLE[];
  static TypeId GetTypeId();

  TcpBbr();

  TcpBbr(const TcpBbr &sock);

  enum BbrMode_t {
    BBR_STARTUP,
    BBR_DRAIN,
    BBR_PROBE_BW,
    BBR_PROBE_RTT,
  };

  typedef WindowedFilter<DataRate, MaxFilter<DataRate>, uint32_t, uint32_t>
      MaxBandwidthFilter_t;

  static const char *const BbrModeName[BBR_PROBE_RTT + 1];

  virtual void SetStream(uint32_t stream);

  std::string GetName() const override;
  bool HasCongControl() const override;
  void CongControl(Ptr<TcpSocketState> tcb,
                   const TcpRateOps::TcpRateConnection &rc,
                   const TcpRateOps::TcpRateSample &rs) override;
  void
  CongestionStateSet(Ptr<TcpSocketState> tcb,
                     const TcpSocketState::TcpCongState_t newState) override;
  void CwndEvent(Ptr<TcpSocketState> tcb,
                 const TcpSocketState::TcpCAEvent_t event) override;
  uint32_t GetSsThresh(Ptr<const TcpSocketState> tcb,
                       uint32_t bytesInFlight) override;
  Ptr<TcpCongestionOps> Fork() override;

protected:
  friend class TcpBbrCheckGainValuesTest;

  void AdvanceCyclePhase();

  void CheckCyclePhase(Ptr<TcpSocketState> tcb,
                       const TcpRateOps::TcpRateSample &rs);

  void CheckDrain(Ptr<TcpSocketState> tcb);

  void CheckFullPipe(const TcpRateOps::TcpRateSample &rs);

  void CheckProbeRTT(Ptr<TcpSocketState> tcb,
                     const TcpRateOps::TcpRateSample &rs);

  void EnterDrain();

  void EnterProbeBW();

  void EnterProbeRTT();

  void EnterStartup();

  void ExitProbeRTT();

  uint32_t GetBbrState();

  double GetPacingGain();

  double GetCwndGain();

  void HandleProbeRTT(Ptr<TcpSocketState> tcb);

  void HandleRestartFromIdle(Ptr<TcpSocketState> tcb,
                             const TcpRateOps::TcpRateSample &rs);

  uint32_t InFlight(Ptr<TcpSocketState> tcb, double gain);

  void InitFullPipe();

  void InitPacingRate(Ptr<TcpSocketState> tcb);

  void InitRoundCounting();

  bool IsNextCyclePhase(Ptr<TcpSocketState> tcb,
                        const TcpRateOps::TcpRateSample &rs);

  void ModulateCwndForProbeRTT(Ptr<TcpSocketState> tcb);

  bool ModulateCwndForRecovery(Ptr<TcpSocketState> tcb,
                               const TcpRateOps::TcpRateSample &rs);

  void RestoreCwnd(Ptr<TcpSocketState> tcb);

  void SaveCwnd(Ptr<const TcpSocketState> tcb);

  void SetCwnd(Ptr<TcpSocketState> tcb, const TcpRateOps::TcpRateSample &rs);

  void SetPacingRate(Ptr<TcpSocketState> tcb, double gain);

  void SetSendQuantum(Ptr<TcpSocketState> tcb);

  void UpdateBtlBw(Ptr<TcpSocketState> tcb,
                   const TcpRateOps::TcpRateSample &rs);

  void UpdateControlParameters(Ptr<TcpSocketState> tcb,
                               const TcpRateOps::TcpRateSample &rs);

  void UpdateModelAndState(Ptr<TcpSocketState> tcb,
                           const TcpRateOps::TcpRateSample &rs);

  void UpdateRound(Ptr<TcpSocketState> tcb,
                   const TcpRateOps::TcpRateSample &rs);

  void UpdateRTprop(Ptr<TcpSocketState> tcb);

  void UpdateTargetCwnd(Ptr<TcpSocketState> tcb);

  void SetBbrState(BbrMode_t state);

  uint32_t AckAggregationCwnd();

  void UpdateAckAggregation(Ptr<TcpSocketState> tcb,
                            const TcpRateOps::TcpRateSample &rs);

private:
  BbrMode_t m_state{BbrMode_t::BBR_STARTUP};
  MaxBandwidthFilter_t m_maxBwFilter;
  uint32_t m_bandwidthWindowLength{0};
  double m_pacingGain{0};
  double m_cWndGain{0};
  double m_highGain{0};
  bool m_isPipeFilled{false};
  uint32_t m_minPipeCwnd{0};
  uint32_t m_roundCount{0};
  bool m_roundStart{false};
  uint32_t m_nextRoundDelivered{0};
  Time m_probeRttDuration{MilliSeconds(200)};
  Time m_probeRtPropStamp{Seconds(0)};
  Time m_probeRttDoneStamp{Seconds(0)};
  bool m_probeRttRoundDone{false};
  bool m_packetConservation{false};
  uint32_t m_priorCwnd{0};
  bool m_idleRestart{false};
  uint32_t m_targetCWnd{0};
  DataRate m_fullBandwidth{0};
  uint32_t m_fullBandwidthCount{0};
  Time m_rtProp{Time::Max()};
  uint32_t m_sendQuantum{0};
  Time m_cycleStamp{Seconds(0)};
  uint32_t m_cycleIndex{0};
  bool m_rtPropExpired{false};
  Time m_rtPropFilterLen{Seconds(10)};
  Time m_rtPropStamp{Seconds(0)};
  bool m_isInitialized{false};
  Ptr<UniformRandomVariable> m_uv{nullptr};
  uint64_t m_delivered{0};
  uint32_t m_appLimited{0};
  uint32_t m_txItemDelivered{0};
  uint32_t m_extraAckedGain{1};
  uint32_t m_extraAcked[2]{0, 0};
  uint32_t m_extraAckedWinRtt{0};
  uint32_t m_extraAckedWinRttLength{5};
  uint32_t m_ackEpochAckedResetThresh{1 << 17};
  uint32_t m_extraAckedIdx{0};
  Time m_ackEpochTime{Seconds(0)};
  uint32_t m_ackEpochAcked{0};
  bool m_hasSeenRtt{false};
};

} // namespace ns3
#endif
