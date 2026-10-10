#ifndef TCPGENERALTEST_H
#define TCPGENERALTEST_H

#include "ns3/error-model.h"
#include "ns3/simple-net-device.h"
#include "ns3/tcp-congestion-ops.h"
#include "ns3/tcp-rate-ops.h"
#include "ns3/tcp-recovery-ops.h"
#include "ns3/tcp-socket-base.h"
#include "ns3/test.h"

namespace ns3 {

class TcpSocketMsgBase : public ns3::TcpSocketBase {
public:
  static TypeId GetTypeId();

  TcpSocketMsgBase() : TcpSocketBase() {}

  TcpSocketMsgBase(const TcpSocketMsgBase &other) : TcpSocketBase(other) {
    m_rcvAckCb = other.m_rcvAckCb;
    m_processedAckCb = other.m_processedAckCb;
    m_beforeRetrCallback = other.m_beforeRetrCallback;
    m_afterRetrCallback = other.m_afterRetrCallback;
    m_forkCb = other.m_forkCb;
    m_updateRttCb = other.m_updateRttCb;
  }

  typedef Callback<void, Ptr<const Packet>, const TcpHeader &,
                   Ptr<const TcpSocketBase>>
      AckManagementCb;
  typedef Callback<void, Ptr<const TcpSocketState>, Ptr<const TcpSocketBase>>
      RetrCb;
  typedef Callback<void, Ptr<const TcpSocketBase>, const SequenceNumber32 &,
                   uint32_t, bool>
      UpdateRttCallback;

  void SetRcvAckCb(AckManagementCb cb);

  void SetProcessedAckCb(AckManagementCb cb);

  void SetAfterRetransmitCb(RetrCb cb);

  void SetBeforeRetransmitCb(RetrCb cb);

  void SetForkCb(Callback<void, Ptr<TcpSocketMsgBase>> cb);

  void SetUpdateRttHistoryCb(UpdateRttCallback cb);

protected:
  void ReceivedAck(Ptr<Packet> packet, const TcpHeader &tcpHeader) override;
  void ReTxTimeout() override;
  Ptr<TcpSocketBase> Fork() override;
  void CompleteFork(Ptr<Packet> p, const TcpHeader &tcpHeader,
                    const Address &fromAddress,
                    const Address &toAddress) override;
  void UpdateRttHistory(const SequenceNumber32 &seq, uint32_t sz,
                        bool isRetransmission) override;

private:
  AckManagementCb m_rcvAckCb;
  AckManagementCb m_processedAckCb;
  RetrCb m_beforeRetrCallback;
  RetrCb m_afterRetrCallback;
  Callback<void, Ptr<TcpSocketMsgBase>> m_forkCb;
  UpdateRttCallback m_updateRttCb;
};

class TcpSocketSmallAcks : public TcpSocketMsgBase {
public:
  static TypeId GetTypeId();

  TcpSocketSmallAcks()
      : TcpSocketMsgBase(), m_bytesToAck(125), m_bytesLeftToBeAcked(0),
        m_lastAckedSeq(1) {}

  TcpSocketSmallAcks(const TcpSocketSmallAcks &other)
      : TcpSocketMsgBase(other), m_bytesToAck(other.m_bytesToAck),
        m_bytesLeftToBeAcked(other.m_bytesLeftToBeAcked),
        m_lastAckedSeq(other.m_lastAckedSeq) {}

  void SetBytesToAck(uint32_t bytes) { m_bytesToAck = bytes; }

protected:
  void SendEmptyPacket(uint8_t flags) override;
  Ptr<TcpSocketBase> Fork() override;

  uint32_t m_bytesToAck;
  uint32_t m_bytesLeftToBeAcked;
  SequenceNumber32 m_lastAckedSeq;
};

class TcpGeneralTest : public TestCase {
public:
  TcpGeneralTest(const std::string &desc);
  ~TcpGeneralTest() override;

  enum SocketWho { SENDER, RECEIVER };

protected:
  virtual Ptr<SimpleChannel> CreateChannel();

  virtual Ptr<ErrorModel> CreateSenderErrorModel();

  virtual Ptr<ErrorModel> CreateReceiverErrorModel();

  virtual Ptr<TcpSocketMsgBase> CreateReceiverSocket(Ptr<Node> node);

  virtual Ptr<TcpSocketMsgBase> CreateSenderSocket(Ptr<Node> node);

  virtual Ptr<TcpSocketMsgBase> CreateSocket(Ptr<Node> node, TypeId socketType,
                                             TypeId congControl);

  virtual Ptr<TcpSocketMsgBase> CreateSocket(Ptr<Node> node, TypeId socketType,
                                             TypeId congControl,
                                             TypeId recoveryAlgorithm);

  Ptr<TcpSocketMsgBase> GetSenderSocket() { return m_senderSocket; }

  Ptr<TcpSocketMsgBase> GetReceiverSocket() { return m_receiverSocket; }

  void DoRun() override;

  virtual void ConfigureEnvironment();

  virtual void ConfigureProperties();

  void DoTeardown() override;

  void DoConnect();

  virtual void ReceivePacket(Ptr<Socket> socket);

  void SendPacket(Ptr<Socket> socket, uint32_t pktSize, uint32_t pktCount,
                  Time pktInterval);

  uint32_t GetSegSize(SocketWho who);

  SequenceNumber32 GetHighestTxMark(SocketWho who);

  uint32_t GetReTxThreshold(SocketWho who);

  uint32_t GetInitialSsThresh(SocketWho who);

  uint32_t GetInitialCwnd(SocketWho who);

  uint32_t GetDupAckCount(SocketWho who);

  uint32_t GetDelAckCount(SocketWho who);

  Time GetDelAckTimeout(SocketWho who);

  Time GetRto(SocketWho who);

  Time GetMinRto(SocketWho who);

  Time GetConnTimeout(SocketWho who);

  Ptr<RttEstimator> GetRttEstimator(SocketWho who);

  Time GetClockGranularity(SocketWho who);

  TcpSocket::TcpStates_t GetTcpState(SocketWho who);

  Ptr<TcpSocketState> GetTcb(SocketWho who);

  Ptr<TcpRxBuffer> GetRxBuffer(SocketWho who);

  Ptr<TcpTxBuffer> GetTxBuffer(SocketWho who);

  uint32_t GetRWnd(SocketWho who);

  EventId GetPersistentEvent(SocketWho who);

  Time GetPersistentTimeout(SocketWho who);

  void SetRcvBufSize(SocketWho who, uint32_t size);

  void SetSegmentSize(SocketWho who, uint32_t segmentSize);

  void SetInitialCwnd(SocketWho who, uint32_t initialCwnd);

  void SetDelAckMaxCount(SocketWho who, uint32_t count);

  void SetUseEcn(SocketWho who, TcpSocketState::UseEcn_t useEcn);

  void SetPacingStatus(SocketWho who, bool pacing);

  void SetPaceInitialWindow(SocketWho who, bool paceWindow);

  void SetInitialSsThresh(SocketWho who, uint32_t initialSsThresh);

  void SetAppPktSize(uint32_t pktSize) { m_pktSize = pktSize; }

  void SetAppPktCount(uint32_t pktCount) { m_pktCount = pktCount; }

  void SetAppPktInterval(Time pktInterval) {
    m_interPacketInterval = pktInterval;
  }

  void SetPropagationDelay(Time propDelay) { m_propagationDelay = propDelay; }

  void SetTransmitStart(Time startTime) { m_startTime = startTime; }

  void SetCongestionControl(TypeId congControl) {
    m_congControlTypeId = congControl;
  }

  void SetRecoveryAlgorithm(TypeId recovery) { m_recoveryTypeId = recovery; }

  void SetMTU(uint32_t mtu) { m_mtu = mtu; }

  virtual void CongStateTrace(const TcpSocketState::TcpCongState_t oldValue
                              [[maybe_unused]],
                              const TcpSocketState::TcpCongState_t newValue
                              [[maybe_unused]]) {}

  virtual void CWndTrace(uint32_t oldValue [[maybe_unused]],
                         uint32_t newValue [[maybe_unused]]) {}

  virtual void CWndInflTrace(uint32_t oldValue [[maybe_unused]],
                             uint32_t newValue [[maybe_unused]]) {}

  virtual void RttTrace(Time oldTime [[maybe_unused]],
                        Time newTime [[maybe_unused]]) {}

  virtual void SsThreshTrace(uint32_t oldValue [[maybe_unused]],
                             uint32_t newValue [[maybe_unused]]) {}

  virtual void BytesInFlightTrace(uint32_t oldValue [[maybe_unused]],
                                  uint32_t newValue [[maybe_unused]]) {}

  virtual void RtoTrace(Time oldValue [[maybe_unused]],
                        Time newValue [[maybe_unused]]) {}

  virtual void NextTxSeqTrace(SequenceNumber32 oldValue [[maybe_unused]],
                              SequenceNumber32 newValue [[maybe_unused]]) {}

  virtual void HighestTxSeqTrace(SequenceNumber32 oldValue [[maybe_unused]],
                                 SequenceNumber32 newValue [[maybe_unused]]) {}

  virtual void RateUpdatedTrace(const TcpRateLinux::TcpRateConnection &rate
                                [[maybe_unused]]) {}

  virtual void RateSampleUpdatedTrace(const TcpRateLinux::TcpRateSample &sample
                                      [[maybe_unused]]) {}

  virtual void NormalClose(SocketWho who [[maybe_unused]]) {}

  virtual void ErrorClose(SocketWho who [[maybe_unused]]) {}

  virtual void QueueDrop(SocketWho who [[maybe_unused]]) {}

  virtual void PhyDrop(SocketWho who [[maybe_unused]]) {}

  virtual void RcvAck(const Ptr<const TcpSocketState> tcb [[maybe_unused]],
                      const TcpHeader &h [[maybe_unused]],
                      SocketWho who [[maybe_unused]]) {}

  virtual void ProcessedAck(const Ptr<const TcpSocketState> tcb
                            [[maybe_unused]],
                            const TcpHeader &h [[maybe_unused]],
                            SocketWho who [[maybe_unused]]) {}

  virtual void Tx(const Ptr<const Packet> p, const TcpHeader &h, SocketWho who);

  virtual void Rx(const Ptr<const Packet> p, const TcpHeader &h, SocketWho who);

  virtual void AfterRTOExpired(const Ptr<const TcpSocketState> tcb
                               [[maybe_unused]],
                               SocketWho who [[maybe_unused]]) {}

  virtual void BeforeRTOExpired(const Ptr<const TcpSocketState> tcb
                                [[maybe_unused]],
                                SocketWho who [[maybe_unused]]) {}

  virtual void UpdatedRttHistory(const SequenceNumber32 &seq [[maybe_unused]],
                                 uint32_t sz [[maybe_unused]],
                                 bool isRetransmission [[maybe_unused]],
                                 SocketWho who [[maybe_unused]]) {}

  virtual void DataSent(uint32_t size [[maybe_unused]],
                        SocketWho who [[maybe_unused]]) {}

  virtual void FinalChecks() {}

  Time GetPropagationDelay() const { return m_propagationDelay; }

  Time GetStartTime() const { return m_startTime; }

  uint32_t GetMtu() const { return m_mtu; }

  uint32_t GetPktSize() const { return m_pktSize; }

  uint32_t GetPktCount() const { return m_pktCount; }

  Time GetPktInterval() const { return m_interPacketInterval; }

  TypeId m_congControlTypeId;
  TypeId m_recoveryTypeId;

private:
  Time m_propagationDelay;

  Time m_startTime;
  uint32_t m_mtu;

  uint32_t m_pktSize;
  uint32_t m_pktCount;
  Time m_interPacketInterval;

  Ptr<TcpSocketMsgBase> m_senderSocket;
  Ptr<TcpSocketMsgBase> m_receiverSocket;

private:
  void NormalCloseCb(Ptr<Socket> socket);
  void ErrorCloseCb(Ptr<Socket> socket);
  void QueueDropCb(std::string context, Ptr<const Packet> p);
  void PhyDropCb(std::string context, Ptr<const Packet> p);
  void RcvAckCb(Ptr<const Packet> p, const TcpHeader &h,
                Ptr<const TcpSocketBase> tcp);
  void ProcessedAckCb(Ptr<const Packet> p, const TcpHeader &h,
                      Ptr<const TcpSocketBase> tcp);
  void TxPacketCb(const Ptr<const Packet> p, const TcpHeader &h,
                  const Ptr<const TcpSocketBase> tcp);
  void RxPacketCb(const Ptr<const Packet> p, const TcpHeader &h,
                  const Ptr<const TcpSocketBase> tcp);
  void RtoExpiredCb(const Ptr<const TcpSocketState> tcb,
                    const Ptr<const TcpSocketBase> tcp);
  void UpdateRttHistoryCb(Ptr<const TcpSocketBase> tcp,
                          const SequenceNumber32 &seq, uint32_t sz,
                          bool isRetransmission);

  void AfterRetransmitCb(const Ptr<const TcpSocketState> tcb,
                         const Ptr<const TcpSocketBase> tcp);

  void BeforeRetransmitCb(const Ptr<const TcpSocketState> tcb,
                          const Ptr<const TcpSocketBase> tcp);

  void DataSentCb(Ptr<Socket> socket, uint32_t size);
  void ForkCb(Ptr<TcpSocketMsgBase> tcp);
  void HandleAccept(Ptr<Socket> socket, const Address &from);

  InetSocketAddress m_remoteAddr;
};

} // namespace ns3

#endif
