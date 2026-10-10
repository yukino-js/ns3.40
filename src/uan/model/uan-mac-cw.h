
#ifndef UAN_MAC_CW_H
#define UAN_MAC_CW_H

#include "uan-mac.h"
#include "uan-phy.h"
#include "uan-tx-mode.h"

#include "ns3/mac8-address.h"
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"

namespace ns3 {

class UanMacCw : public UanMac, public UanPhyListener {
public:
  UanMacCw();
  ~UanMacCw() override;
  static TypeId GetTypeId();

  virtual void SetCw(uint32_t cw);
  virtual void SetSlotTime(Time duration);
  virtual uint32_t GetCw();
  virtual Time GetSlotTime();

  bool Enqueue(Ptr<Packet> pkt, uint16_t protocolNumber,
               const Address &dest) override;
  void SetForwardUpCb(
      Callback<void, Ptr<Packet>, uint16_t, const Mac8Address &> cb) override;
  void AttachPhy(Ptr<UanPhy> phy) override;
  void Clear() override;
  int64_t AssignStreams(int64_t stream) override;

  void NotifyRxStart() override;
  void NotifyRxEndOk() override;
  void NotifyRxEndError() override;
  void NotifyCcaStart() override;
  void NotifyCcaEnd() override;
  void NotifyTxStart(Time duration) override;
  void NotifyTxEnd() override;

  typedef void (*QueueTracedCallback)(Ptr<const Packet> packet, uint16_t proto);

private:
  enum State { IDLE, CCABUSY, RUNNING, TX };

  Callback<void, Ptr<Packet>, uint16_t, const Mac8Address &> m_forwardUpCb;
  Ptr<UanPhy> m_phy;
  TracedCallback<Ptr<const Packet>, UanTxMode> m_rxLogger;
  TracedCallback<Ptr<const Packet>, uint16_t> m_enqueueLogger;
  TracedCallback<Ptr<const Packet>, uint16_t> m_dequeueLogger;

  uint32_t m_cw;
  Time m_slotTime;

  Time m_sendTime;
  Time m_savedDelayS;
  Ptr<Packet> m_pktTx;
  uint16_t m_pktTxProt;
  EventId m_sendEvent;
  bool m_txOngoing;
  State m_state;

  bool m_cleared;

  Ptr<UniformRandomVariable> m_rv;

  void PhyRxPacketGood(Ptr<Packet> packet, double sinr, UanTxMode mode);
  void PhyRxPacketError(Ptr<Packet> packet, double sinr);
  void SaveTimer();
  void StartTimer();
  void SendPacket();
  void EndTx();

protected:
  void DoDispose() override;
};

} // namespace ns3

#endif
