#include "tcp-recovery-ops.h"

#include "tcp-socket-state.h"

#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("TcpRecoveryOps");

NS_OBJECT_ENSURE_REGISTERED(TcpRecoveryOps);

TypeId TcpRecoveryOps::GetTypeId() {
  static TypeId tid = TypeId("ns3::TcpRecoveryOps")
                          .SetParent<Object>()
                          .SetGroupName("Internet");
  return tid;
}

TcpRecoveryOps::TcpRecoveryOps() : Object() { NS_LOG_FUNCTION(this); }

TcpRecoveryOps::TcpRecoveryOps(const TcpRecoveryOps &other) : Object(other) {
  NS_LOG_FUNCTION(this);
}

TcpRecoveryOps::~TcpRecoveryOps() { NS_LOG_FUNCTION(this); }

void TcpRecoveryOps::UpdateBytesSent(uint32_t bytesSent) {
  NS_LOG_FUNCTION(this << bytesSent);
}

NS_OBJECT_ENSURE_REGISTERED(TcpClassicRecovery);

TypeId TcpClassicRecovery::GetTypeId() {
  static TypeId tid = TypeId("ns3::TcpClassicRecovery")
                          .SetParent<TcpRecoveryOps>()
                          .SetGroupName("Internet")
                          .AddConstructor<TcpClassicRecovery>();
  return tid;
}

TcpClassicRecovery::TcpClassicRecovery() : TcpRecoveryOps() {
  NS_LOG_FUNCTION(this);
}

TcpClassicRecovery::TcpClassicRecovery(const TcpClassicRecovery &sock)
    : TcpRecoveryOps(sock) {
  NS_LOG_FUNCTION(this);
}

TcpClassicRecovery::~TcpClassicRecovery() { NS_LOG_FUNCTION(this); }

void TcpClassicRecovery::EnterRecovery(Ptr<TcpSocketState> tcb,
                                       uint32_t dupAckCount,
                                       uint32_t unAckDataCount [[maybe_unused]],
                                       uint32_t deliveredBytes
                                       [[maybe_unused]]) {
  NS_LOG_FUNCTION(this << tcb << dupAckCount << unAckDataCount);
  tcb->m_cWnd = tcb->m_ssThresh;
  tcb->m_cWndInfl = tcb->m_ssThresh + (dupAckCount * tcb->m_segmentSize);
}

void TcpClassicRecovery::DoRecovery(Ptr<TcpSocketState> tcb,
                                    uint32_t deliveredBytes [[maybe_unused]]) {
  NS_LOG_FUNCTION(this << tcb << deliveredBytes);
  tcb->m_cWndInfl += tcb->m_segmentSize;
}

void TcpClassicRecovery::ExitRecovery(Ptr<TcpSocketState> tcb) {
  NS_LOG_FUNCTION(this << tcb);
  tcb->m_cWndInfl = tcb->m_ssThresh.Get();
}

std::string TcpClassicRecovery::GetName() const { return "TcpClassicRecovery"; }

Ptr<TcpRecoveryOps> TcpClassicRecovery::Fork() {
  return CopyObject<TcpClassicRecovery>(this);
}

} // namespace ns3
