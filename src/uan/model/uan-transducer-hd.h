
#ifndef UAN_TRANSDUCER_HD_H
#define UAN_TRANSDUCER_HD_H

#include "uan-transducer.h"

#include "ns3/simulator.h"

namespace ns3 {

class UanTransducerHd : public UanTransducer {
public:
  UanTransducerHd();
  ~UanTransducerHd() override;

  static TypeId GetTypeId();

  State GetState() const override;
  bool IsRx() const override;
  bool IsTx() const override;
  const ArrivalList &GetArrivalList() const override;
  double ApplyRxGainDb(double rxPowerDb, UanTxMode mode) override;
  void SetRxGainDb(double gainDb) override;
  double GetRxGainDb() override;
  void Receive(Ptr<Packet> packet, double rxPowerDb, UanTxMode txMode,
               UanPdp pdp) override;
  void Transmit(Ptr<UanPhy> src, Ptr<Packet> packet, double txPowerDb,
                UanTxMode txMode) override;
  void SetChannel(Ptr<UanChannel> chan) override;
  Ptr<UanChannel> GetChannel() const override;
  void AddPhy(Ptr<UanPhy>) override;
  const UanPhyList &GetPhyList() const override;
  void Clear() override;

private:
  State m_state;
  ArrivalList m_arrivalList;
  UanPhyList m_phyList;
  Ptr<UanChannel> m_channel;
  EventId m_endTxEvent;
  Time m_endTxTime;
  bool m_cleared;
  double m_rxGainDb;

  void RemoveArrival(UanPacketArrival arrival);
  void EndTx();

protected:
  void DoDispose() override;
};

} // namespace ns3

#endif
