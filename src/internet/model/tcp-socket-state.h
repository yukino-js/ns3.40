#ifndef TCP_SOCKET_STATE_H
#define TCP_SOCKET_STATE_H

#include "tcp-rx-buffer.h"

#include "ns3/data-rate.h"
#include "ns3/object.h"
#include "ns3/sequence-number.h"
#include "ns3/traced-value.h"

namespace ns3 {

class TcpSocketState : public Object {
public:
  static TypeId GetTypeId();

  TcpSocketState() : Object() {}

  TcpSocketState(const TcpSocketState &other);

  enum TcpCongState_t {
    CA_OPEN,
    CA_DISORDER,
    CA_CWR,
    CA_RECOVERY,
    CA_LOSS,
    CA_LAST_STATE
  };

  enum TcpCAEvent_t {
    CA_EVENT_TX_START,
    CA_EVENT_CWND_RESTART,
    CA_EVENT_COMPLETE_CWR,
    CA_EVENT_LOSS,
    CA_EVENT_ECN_NO_CE,
    CA_EVENT_ECN_IS_CE,
    CA_EVENT_DELAYED_ACK,
    CA_EVENT_NON_DELAYED_ACK,
  };

  enum UseEcn_t {
    Off = 0,
    On = 1,
    AcceptOnly = 2,
  };

  enum EcnCodePoint_t {
    NotECT = 0,
    Ect1 = 1,
    Ect0 = 2,
    CongExp = 3,
  };

  enum EcnMode_t {
    ClassicEcn,
    DctcpEcn,
  };

  enum EcnState_t {
    ECN_DISABLED = 0,
    ECN_IDLE,
    ECN_CE_RCVD,
    ECN_SENDING_ECE,
    ECN_ECE_RCVD,
    ECN_CWR_SENT
  };

  static const char *const TcpCongStateName[TcpSocketState::CA_LAST_STATE];

  static const char *const EcnStateName[TcpSocketState::ECN_CWR_SENT + 1];

  TracedValue<uint32_t> m_cWnd{0};
  TracedValue<uint32_t> m_cWndInfl{0};
  TracedValue<uint32_t> m_ssThresh{0};
  uint32_t m_initialCWnd{0};
  uint32_t m_initialSsThresh{0};

  bool m_isRetransDataAcked{false};

  uint32_t m_segmentSize{0};
  SequenceNumber32 m_lastAckedSeq{0};

  TracedValue<TcpCongState_t> m_congState{CA_OPEN};

  TracedValue<EcnState_t> m_ecnState{ECN_DISABLED};

  TracedValue<SequenceNumber32> m_highTxMark{0};
  TracedValue<SequenceNumber32> m_nextTxSequence{0};

  uint32_t m_rcvTimestampValue{0};
  uint32_t m_rcvTimestampEchoReply{0};

  bool m_pacing{false};
  DataRate m_maxPacingRate{0};
  TracedValue<DataRate> m_pacingRate{0};
  uint16_t m_pacingSsRatio{0};
  uint16_t m_pacingCaRatio{0};
  bool m_paceInitialWindow{false};

  Time m_minRtt{Time::Max()};

  TracedValue<uint32_t> m_bytesInFlight{0};
  TracedValue<Time> m_lastRtt{Seconds(0.0)};

  Ptr<TcpRxBuffer> m_rxBuffer;

  EcnMode_t m_ecnMode{ClassicEcn};
  UseEcn_t m_useEcn{Off};

  EcnCodePoint_t m_ectCodePoint{Ect0};

  uint32_t m_lastAckedSackedBytes{0};

  uint32_t GetCwndInSegments() const { return m_cWnd / m_segmentSize; }

  uint32_t GetSsThreshInSegments() const { return m_ssThresh / m_segmentSize; }

  Callback<void, uint8_t> m_sendEmptyPacketCallback;
};

namespace TracedValueCallback {

typedef void (*TcpCongState)(const TcpSocketState::TcpCongState_t oldValue,
                             const TcpSocketState::TcpCongState_t newValue);

typedef void (*EcnState)(const TcpSocketState::EcnState_t oldValue,
                         const TcpSocketState::EcnState_t newValue);

} // namespace TracedValueCallback

} // namespace ns3

#endif
