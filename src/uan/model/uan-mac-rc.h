
#ifndef UAN_MAC_RC_H
#define UAN_MAC_RC_H

#include "uan-mac.h"

#include "ns3/event-id.h"
#include "ns3/mac8-address.h"
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/traced-callback.h"

#include <list>
#include <utility>
#include <vector>

namespace ns3 {

class Address;
class UanTxMode;
class UanHeaderRcRts;
class UanHeaderRcCts;
class UanHeaderRcCtsGlobal;
class UanPhy;

class Reservation {
public:
  Reservation();
  Reservation(std::list<std::pair<Ptr<Packet>, Mac8Address>> &list,
              uint8_t frameNo, uint32_t maxPkts = 0);
  ~Reservation();
  uint32_t GetNoFrames() const;
  uint32_t GetLength() const;
  const std::list<std::pair<Ptr<Packet>, Mac8Address>> &GetPktList() const;
  uint8_t GetFrameNo() const;
  uint8_t GetRetryNo() const;
  Time GetTimestamp(uint8_t n) const;

  bool IsTransmitted() const;
  void SetFrameNo(uint8_t fn);
  void AddTimestamp(Time t);
  void IncrementRetry();
  void SetTransmitted(bool t = true);

private:
  std::list<std::pair<Ptr<Packet>, Mac8Address>> m_pktList;
  uint32_t m_length;
  uint8_t m_frameNo;
  std::vector<Time> m_timestamp;
  uint8_t m_retryNo;
  bool m_transmitted;
};

class UanMacRc : public UanMac {
public:
  enum { TYPE_DATA, TYPE_GWPING, TYPE_RTS, TYPE_CTS, TYPE_ACK };

  UanMacRc();
  ~UanMacRc() override;

  static TypeId GetTypeId();

  bool Enqueue(Ptr<Packet> pkt, uint16_t protocolNumber,
               const Address &dest) override;
  void SetForwardUpCb(
      Callback<void, Ptr<Packet>, uint16_t, const Mac8Address &> cb) override;
  void AttachPhy(Ptr<UanPhy> phy) override;
  void Clear() override;
  int64_t AssignStreams(int64_t stream) override;

  typedef void (*QueueTracedCallback)(Ptr<const Packet> packet, uint32_t proto);

private:
  enum State { UNASSOCIATED, GWPSENT, IDLE, RTSSENT, DATATX };

  State m_state;
  bool m_rtsBlocked;

  EventId m_startAgain;
  double m_retryRate;
  Mac8Address m_assocAddr;
  Ptr<UanPhy> m_phy;
  uint32_t m_numRates;
  uint32_t m_currentRate;
  uint32_t m_maxFrames;
  uint32_t m_queueLimit;
  uint8_t m_frameNo;
  Time m_sifs;
  Time m_learnedProp;

  double m_minRetryRate;
  double m_retryStep;

  uint32_t m_ctsSizeN;
  uint32_t m_ctsSizeG;

  bool m_cleared;

  std::list<std::pair<Ptr<Packet>, Mac8Address>> m_pktQueue;
  std::list<Reservation> m_resList;

  Callback<void, Ptr<Packet>, uint16_t, const Mac8Address &> m_forwardUpCb;

  TracedCallback<Ptr<const Packet>, UanTxMode> m_rxLogger;
  TracedCallback<Ptr<const Packet>, uint32_t> m_enqueueLogger;
  TracedCallback<Ptr<const Packet>, uint32_t> m_dequeueLogger;

  EventId m_rtsEvent;
  void ReceiveOkFromPhy(Ptr<Packet> pkt, double sinr, UanTxMode mode);
  void Associate();
  void AssociateTimeout();
  void SendRts();
  void RtsTimeout();
  UanHeaderRcRts CreateRtsHeader(const Reservation &res);
  void ScheduleData(const UanHeaderRcCts &ctsh,
                    const UanHeaderRcCtsGlobal &ctsg, uint32_t ctsBytes);
  void ProcessAck(Ptr<Packet> ack);
  void SendPacket(Ptr<Packet> pkt, uint32_t rate);
  bool IsPhy1Ok();
  void BlockRtsing();

  static uint32_t m_cntrlSends;

  Ptr<ExponentialRandomVariable> m_ev;

protected:
  void DoDispose() override;
};

} // namespace ns3

#endif
