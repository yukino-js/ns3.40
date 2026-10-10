
#ifndef UAN_TRANSDUCER_H
#define UAN_TRANSDUCER_H

#include "uan-prop-model.h"
#include "uan-tx-mode.h"

#include "ns3/object.h"
#include "ns3/packet.h"

#include <list>

namespace ns3 {

class UanPhy;
class UanChannel;

class UanPacketArrival {
public:
  UanPacketArrival() {}

  UanPacketArrival(Ptr<Packet> packet, double rxPowerDb, UanTxMode txMode,
                   UanPdp pdp, Time arrTime)
      : m_packet(packet), m_rxPowerDb(rxPowerDb), m_txMode(txMode), m_pdp(pdp),
        m_arrTime(arrTime) {}

  ~UanPacketArrival() { m_packet = nullptr; }

  inline Ptr<Packet> GetPacket() const { return m_packet; }

  inline double GetRxPowerDb() const { return m_rxPowerDb; }

  inline const UanTxMode &GetTxMode() const { return m_txMode; }

  inline Time GetArrivalTime() const { return m_arrTime; }

  inline UanPdp GetPdp() const { return m_pdp; }

private:
  Ptr<Packet> m_packet;
  double m_rxPowerDb;
  UanTxMode m_txMode;
  UanPdp m_pdp;
  Time m_arrTime;
};

class UanTransducer : public Object {
public:
  static TypeId GetTypeId();

  enum State { TX, RX };

  typedef std::list<UanPacketArrival> ArrivalList;
  typedef std::list<Ptr<UanPhy>> UanPhyList;

  virtual State GetState() const = 0;

  virtual bool IsRx() const = 0;
  virtual bool IsTx() const = 0;
  virtual const ArrivalList &GetArrivalList() const = 0;
  virtual void SetRxGainDb(double gainDb) = 0;
  virtual double GetRxGainDb() = 0;
  virtual double ApplyRxGainDb(double rxPowerDb, UanTxMode mode) = 0;
  virtual void Receive(Ptr<Packet> packet, double rxPowerDb, UanTxMode txMode,
                       UanPdp pdp) = 0;
  virtual void Transmit(Ptr<UanPhy> src, Ptr<Packet> packet, double txPowerDb,
                        UanTxMode txMode) = 0;
  virtual void SetChannel(Ptr<UanChannel> chan) = 0;
  virtual Ptr<UanChannel> GetChannel() const = 0;
  virtual void AddPhy(Ptr<UanPhy> phy) = 0;
  virtual const UanPhyList &GetPhyList() const = 0;
  virtual void Clear() = 0;
};

} // namespace ns3

#endif
