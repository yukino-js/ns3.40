#ifndef TCP_SOCKET_BASE_H
#define TCP_SOCKET_BASE_H

#include "ipv4-header.h"
#include "ipv6-header.h"
#include "tcp-socket-state.h"
#include "tcp-socket.h"

#include "ns3/data-rate.h"
#include "ns3/node.h"
#include "ns3/sequence-number.h"
#include "ns3/timer.h"
#include "ns3/traced-value.h"

#include <queue>
#include <stdint.h>

namespace ns3 {

class Ipv4EndPoint;
class Ipv6EndPoint;
class Node;
class Packet;
class TcpL4Protocol;
class TcpHeader;
class TcpCongestionOps;
class TcpRecoveryOps;
class RttEstimator;
class TcpRxBuffer;
class TcpTxBuffer;
class TcpOption;
class Ipv4Interface;
class Ipv6Interface;
class TcpRateOps;

class RttHistory {
public:
  RttHistory(SequenceNumber32 s, uint32_t c, Time t);
  RttHistory(const RttHistory &h);

public:
  SequenceNumber32 seq;
  uint32_t count;
  Time time;
  bool retx;
};

class TcpSocketBase : public TcpSocket {
public:
  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  friend class TcpGeneralTest;

  TcpSocketBase();

  TcpSocketBase(const TcpSocketBase &sock);
  ~TcpSocketBase() override;

  virtual void SetNode(Ptr<Node> node);

  virtual void SetTcp(Ptr<TcpL4Protocol> tcp);

  virtual void SetRtt(Ptr<RttEstimator> rtt);

  void SetMinRto(Time minRto);

  Time GetMinRto() const;

  void SetClockGranularity(Time clockGranularity);

  Time GetClockGranularity() const;

  Ptr<TcpTxBuffer> GetTxBuffer() const;

  Ptr<TcpRxBuffer> GetRxBuffer() const;

  void SetRetxThresh(uint32_t retxThresh);

  uint32_t GetRetxThresh() const { return m_retxThresh; }

  TracedCallback<DataRate, DataRate> m_pacingRateTrace;

  TracedCallback<uint32_t, uint32_t> m_cWndTrace;

  TracedCallback<uint32_t, uint32_t> m_cWndInflTrace;

  TracedCallback<uint32_t, uint32_t> m_ssThTrace;

  TracedCallback<TcpSocketState::TcpCongState_t, TcpSocketState::TcpCongState_t>
      m_congStateTrace;

  TracedCallback<TcpSocketState::EcnState_t, TcpSocketState::EcnState_t>
      m_ecnStateTrace;

  TracedCallback<SequenceNumber32, SequenceNumber32> m_highTxMarkTrace;

  TracedCallback<SequenceNumber32, SequenceNumber32> m_nextTxSequenceTrace;

  TracedCallback<uint32_t, uint32_t> m_bytesInFlightTrace;

  TracedCallback<Time, Time> m_lastRttTrace;

  void UpdatePacingRateTrace(DataRate oldValue, DataRate newValue) const;

  void UpdateCwnd(uint32_t oldValue, uint32_t newValue) const;

  void UpdateCwndInfl(uint32_t oldValue, uint32_t newValue) const;

  void UpdateSsThresh(uint32_t oldValue, uint32_t newValue) const;

  void UpdateCongState(TcpSocketState::TcpCongState_t oldValue,
                       TcpSocketState::TcpCongState_t newValue) const;

  void UpdateEcnState(TcpSocketState::EcnState_t oldValue,
                      TcpSocketState::EcnState_t newValue) const;

  void UpdateHighTxMark(SequenceNumber32 oldValue,
                        SequenceNumber32 newValue) const;

  void UpdateNextTxSequence(SequenceNumber32 oldValue,
                            SequenceNumber32 newValue) const;

  void UpdateBytesInFlight(uint32_t oldValue, uint32_t newValue) const;

  void UpdateRtt(Time oldValue, Time newValue) const;

  void SetCongestionControlAlgorithm(Ptr<TcpCongestionOps> algo);

  void SetRecoveryAlgorithm(Ptr<TcpRecoveryOps> recovery);

  inline uint8_t MarkEcnEct0(uint8_t tos) const {
    return ((tos & 0xfc) | 0x02);
  }

  inline uint8_t MarkEcnEct1(uint8_t tos) const {
    return ((tos & 0xfc) | 0x01);
  }

  inline uint8_t MarkEcnCe(uint8_t tos) const { return ((tos & 0xfc) | 0x03); }

  inline uint8_t ClearEcnBits(uint8_t tos) const { return tos & 0xfc; }

  inline bool CheckNoEcn(uint8_t tos) const { return ((tos & 0x03) == 0x00); }

  inline bool CheckEcnEct0(uint8_t tos) const { return ((tos & 0x03) == 0x02); }

  inline bool CheckEcnEct1(uint8_t tos) const { return ((tos & 0x03) == 0x01); }

  inline bool CheckEcnCe(uint8_t tos) const { return ((tos & 0x03) == 0x03); }

  inline uint8_t
  MarkEcnCodePoint(const uint8_t tos,
                   const TcpSocketState::EcnCodePoint_t codePoint) const {
    return ((tos & 0xfc) | codePoint);
  }

  void SetUseEcn(TcpSocketState::UseEcn_t useEcn);

  void SetPacingStatus(bool pacing);

  void SetPaceInitialWindow(bool paceWindow);

  SocketErrno GetErrno() const override;
  SocketType GetSocketType() const override;
  Ptr<Node> GetNode() const override;
  int Bind() override;
  int Bind6() override;
  int Bind(const Address &address) override;
  int Connect(const Address &address) override;
  int Listen() override;
  int Close() override;
  int ShutdownSend() override;
  int ShutdownRecv() override;
  int Send(Ptr<Packet> p, uint32_t flags) override;
  int SendTo(Ptr<Packet> p, uint32_t flags, const Address &toAddress) override;
  Ptr<Packet> Recv(uint32_t maxSize, uint32_t flags) override;
  Ptr<Packet> RecvFrom(uint32_t maxSize, uint32_t flags,
                       Address &fromAddress) override;
  uint32_t GetTxAvailable() const override;
  uint32_t GetRxAvailable() const override;
  int GetSockName(Address &address) const override;
  int GetPeerName(Address &address) const override;
  void BindToNetDevice(Ptr<NetDevice> netdevice) override;

  typedef void (*TcpTxRxTracedCallback)(const Ptr<const Packet> packet,
                                        const TcpHeader &header,
                                        const Ptr<const TcpSocketBase> socket);

protected:
  void SetSndBufSize(uint32_t size) override;
  uint32_t GetSndBufSize() const override;
  void SetRcvBufSize(uint32_t size) override;
  uint32_t GetRcvBufSize() const override;
  void SetSegSize(uint32_t size) override;
  uint32_t GetSegSize() const override;
  void SetInitialSSThresh(uint32_t threshold) override;
  uint32_t GetInitialSSThresh() const override;
  void SetInitialCwnd(uint32_t cwnd) override;
  uint32_t GetInitialCwnd() const override;
  void SetConnTimeout(Time timeout) override;
  Time GetConnTimeout() const override;
  void SetSynRetries(uint32_t count) override;
  uint32_t GetSynRetries() const override;
  void SetDataRetries(uint32_t retries) override;
  uint32_t GetDataRetries() const override;
  void SetDelAckTimeout(Time timeout) override;
  Time GetDelAckTimeout() const override;
  void SetDelAckMaxCount(uint32_t count) override;
  uint32_t GetDelAckMaxCount() const override;
  void SetTcpNoDelay(bool noDelay) override;
  bool GetTcpNoDelay() const override;
  void SetPersistTimeout(Time timeout) override;
  Time GetPersistTimeout() const override;
  bool SetAllowBroadcast(bool allowBroadcast) override;
  bool GetAllowBroadcast() const override;

  int SetupCallback();

  int DoConnect();

  void ConnectionSucceeded();

  int SetupEndpoint();

  int SetupEndpoint6();

  virtual void CompleteFork(Ptr<Packet> p, const TcpHeader &tcpHeader,
                            const Address &fromAddress,
                            const Address &toAddress);

  bool IsValidTcpSegment(const SequenceNumber32 seq,
                         const uint32_t tcpHeaderSize,
                         const uint32_t tcpPayloadSize);

  void ForwardUp(Ptr<Packet> packet, Ipv4Header header, uint16_t port,
                 Ptr<Ipv4Interface> incomingInterface);

  void ForwardUp6(Ptr<Packet> packet, Ipv6Header header, uint16_t port,
                  Ptr<Ipv6Interface> incomingInterface);

  virtual void DoForwardUp(Ptr<Packet> packet, const Address &fromAddress,
                           const Address &toAddress);

  void ForwardIcmp(Ipv4Address icmpSource, uint8_t icmpTtl, uint8_t icmpType,
                   uint8_t icmpCode, uint32_t icmpInfo);

  void ForwardIcmp6(Ipv6Address icmpSource, uint8_t icmpTtl, uint8_t icmpType,
                    uint8_t icmpCode, uint32_t icmpInfo);

  uint32_t SendPendingData(bool withAck = false);

  virtual uint32_t SendDataPacket(SequenceNumber32 seq, uint32_t maxSize,
                                  bool withAck);

  virtual void SendEmptyPacket(uint8_t flags);

  void SendRST();

  bool OutOfRange(SequenceNumber32 head, SequenceNumber32 tail) const;

  int DoClose();

  void CloseAndNotify();

  void Destroy();

  void Destroy6();

  void DeallocateEndPoint();

  void PeerClose(Ptr<Packet> p, const TcpHeader &tcpHeader);

  void DoPeerClose();

  void CancelAllTimers();

  void TimeWait();

  void ProcessEstablished(Ptr<Packet> packet, const TcpHeader &tcpHeader);

  void ProcessListen(Ptr<Packet> packet, const TcpHeader &tcpHeader,
                     const Address &fromAddress, const Address &toAddress);

  void ProcessSynSent(Ptr<Packet> packet, const TcpHeader &tcpHeader);

  void ProcessSynRcvd(Ptr<Packet> packet, const TcpHeader &tcpHeader,
                      const Address &fromAddress, const Address &toAddress);

  void ProcessWait(Ptr<Packet> packet, const TcpHeader &tcpHeader);

  void ProcessClosing(Ptr<Packet> packet, const TcpHeader &tcpHeader);

  void ProcessLastAck(Ptr<Packet> packet, const TcpHeader &tcpHeader);

  virtual uint32_t UnAckDataCount() const;

  virtual uint32_t BytesInFlight() const;

  virtual uint32_t Window() const;

  virtual uint32_t AvailableWindow() const;

  virtual uint16_t AdvertisedWindowSize(bool scale = true) const;

  void UpdateWindowSize(const TcpHeader &header);

  virtual Ptr<TcpSocketBase> Fork();

  virtual void ReceivedAck(Ptr<Packet> packet, const TcpHeader &tcpHeader);

  virtual void ProcessAck(const SequenceNumber32 &ackNumber,
                          bool scoreboardUpdated, uint32_t currentDelivered,
                          const SequenceNumber32 &oldHeadSequence);

  virtual void ReceivedData(Ptr<Packet> packet, const TcpHeader &tcpHeader);

  virtual void EstimateRtt(const TcpHeader &tcpHeader);

  virtual void UpdateRttHistory(const SequenceNumber32 &seq, uint32_t sz,
                                bool isRetransmission);

  virtual void NewAck(const SequenceNumber32 &seq, bool resetRTO);

  void DupAck(uint32_t currentDelivered);

  void EnterCwr(uint32_t currentDelivered);

  void EnterRecovery(uint32_t currentDelivered);

  virtual void ReTxTimeout();

  virtual void DelAckTimeout();

  virtual void LastAckTimeout();

  virtual void PersistTimeout();

  void DoRetransmit();

  void AddOptions(TcpHeader &tcpHeader);

  void ReadOptions(const TcpHeader &tcpHeader, uint32_t *bytesSacked);

  bool IsTcpOptionEnabled(uint8_t kind) const;

  void ProcessOptionWScale(const Ptr<const TcpOption> option);
  void AddOptionWScale(TcpHeader &header);

  uint8_t CalculateWScale() const;

  void ProcessOptionSackPermitted(const Ptr<const TcpOption> option);

  uint32_t ProcessOptionSack(const Ptr<const TcpOption> option);

  void AddOptionSackPermitted(TcpHeader &header);

  void AddOptionSack(TcpHeader &header);

  void ProcessOptionTimestamp(const Ptr<const TcpOption> option,
                              const SequenceNumber32 &seq);
  void AddOptionTimestamp(TcpHeader &header);

  static uint32_t SafeSubtraction(uint32_t a, uint32_t b);

  void NotifyPacingPerformed();

  bool IsPacingEnabled() const;

  void UpdatePacingRate();

  void AddSocketTags(const Ptr<Packet> &p) const;

  uint32_t GetRWnd() const;

  SequenceNumber32 GetHighRxAck() const;

protected:
  EventId m_retxEvent{};
  EventId m_lastAckEvent{};
  EventId m_delAckEvent{};
  EventId m_persistEvent{};
  EventId m_timewaitEvent{};

  uint32_t m_dupAckCount{0};
  uint32_t m_delAckCount{0};
  uint32_t m_delAckMaxCount{0};

  bool m_noDelay{false};

  uint32_t m_synCount{0};
  uint32_t m_synRetries{0};
  uint32_t m_dataRetrCount{0};
  uint32_t m_dataRetries{0};

  TracedValue<Time> m_rto{Seconds(0.0)};
  Time m_minRto{Time::Max()};
  Time m_clockGranularity{Seconds(0.001)};
  Time m_delAckTimeout{Seconds(0.0)};
  Time m_persistTimeout{Seconds(0.0)};
  Time m_cnTimeout{Seconds(0.0)};

  std::deque<RttHistory> m_history;

  Ipv4EndPoint *m_endPoint{nullptr};
  Ipv6EndPoint *m_endPoint6{nullptr};
  Ptr<Node> m_node;
  Ptr<TcpL4Protocol> m_tcp;
  Callback<void, Ipv4Address, uint8_t, uint8_t, uint8_t, uint32_t>
      m_icmpCallback;
  Callback<void, Ipv6Address, uint8_t, uint8_t, uint8_t, uint32_t>
      m_icmpCallback6;

  Ptr<RttEstimator> m_rtt;

  Ptr<TcpTxBuffer> m_txBuffer;

  TracedValue<TcpStates_t> m_state{CLOSED};

  mutable SocketErrno m_errno{ERROR_NOTERROR};

  bool m_closeNotified{false};
  bool m_closeOnEmpty{false};
  bool m_shutdownSend{false};
  bool m_shutdownRecv{false};
  bool m_connected{false};
  double m_msl{0.0};

  uint16_t m_maxWinSize{0};
  uint32_t m_bytesAckedNotProcessed{0};
  SequenceNumber32 m_highTxAck{0};
  TracedValue<uint32_t> m_rWnd{0};
  TracedValue<uint32_t> m_advWnd{0};
  TracedValue<SequenceNumber32> m_highRxMark{0};
  TracedValue<SequenceNumber32> m_highRxAckMark{0};

  bool m_sackEnabled{true};
  bool m_winScalingEnabled{true};
  uint8_t m_rcvWindShift{0};
  uint8_t m_sndWindShift{0};
  bool m_timestampEnabled{true};
  uint32_t m_timestampToEcho{0};

  EventId m_sendPendingDataEvent{};

  SequenceNumber32 m_recover{0};
  bool m_recoverActive{false};
  uint32_t m_retxThresh{3};
  bool m_limitedTx{true};

  Ptr<TcpSocketState> m_tcb;
  Ptr<TcpCongestionOps> m_congestionControl;
  Ptr<TcpRecoveryOps> m_recoveryOps;
  Ptr<TcpRateOps> m_rateOps;

  bool m_isFirstPartialAck{true};

  TracedCallback<Ptr<const Packet>, const TcpHeader &, Ptr<const TcpSocketBase>>
      m_txTrace;

  TracedCallback<Ptr<const Packet>, const TcpHeader &, Ptr<const TcpSocketBase>>
      m_rxTrace;

  Timer m_pacingTimer{Timer::CANCEL_ON_DESTROY};

  TracedValue<SequenceNumber32> m_ecnEchoSeq{0};
  TracedValue<SequenceNumber32> m_ecnCESeq{0};
  TracedValue<SequenceNumber32> m_ecnCWRSeq{0};
};

typedef void (*TcpCongStatesTracedValueCallback)(
    const TcpSocketState::TcpCongState_t oldValue,
    const TcpSocketState::TcpCongState_t newValue);

typedef void (*EcnStatesTracedValueCallback)(
    const TcpSocketState::EcnState_t oldValue,
    const TcpSocketState::EcnState_t newValue);

} // namespace ns3

#endif
