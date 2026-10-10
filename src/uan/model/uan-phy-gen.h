
#ifndef UAN_PHY_GEN_H
#define UAN_PHY_GEN_H

#include "uan-phy.h"

#include "ns3/device-energy-model.h"
#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"
#include "ns3/traced-callback.h"

#include <list>

namespace ns3 {

class UanPhyPerGenDefault : public UanPhyPer {
public:
  UanPhyPerGenDefault();
  ~UanPhyPerGenDefault() override;

  static TypeId GetTypeId();

  double CalcPer(Ptr<Packet> pkt, double sinrDb, UanTxMode mode) override;

private:
  double m_thresh;
};

class UanPhyPerUmodem : public UanPhyPer {
public:
  UanPhyPerUmodem();
  ~UanPhyPerUmodem() override;

  static TypeId GetTypeId();

  double CalcPer(Ptr<Packet> pkt, double sinrDb, UanTxMode mode) override;

private:
  double NChooseK(uint32_t n, uint32_t k);
};

class UanPhyPerCommonModes : public UanPhyPer {
public:
  UanPhyPerCommonModes();
  ~UanPhyPerCommonModes() override;

  static TypeId GetTypeId();

  double CalcPer(Ptr<Packet> pkt, double sinrDb, UanTxMode mode) override;
};

class UanPhyCalcSinrDefault : public UanPhyCalcSinr {
public:
  UanPhyCalcSinrDefault();
  ~UanPhyCalcSinrDefault() override;

  static TypeId GetTypeId();

  double
  CalcSinrDb(Ptr<Packet> pkt, Time arrTime, double rxPowerDb, double ambNoiseDb,
             UanTxMode mode, UanPdp pdp,
             const UanTransducer::ArrivalList &arrivalList) const override;
};

class UanPhyCalcSinrFhFsk : public UanPhyCalcSinr {
public:
  UanPhyCalcSinrFhFsk();
  ~UanPhyCalcSinrFhFsk() override;

  static TypeId GetTypeId();

  double
  CalcSinrDb(Ptr<Packet> pkt, Time arrTime, double rxPowerDb, double ambNoiseDb,
             UanTxMode mode, UanPdp pdp,
             const UanTransducer::ArrivalList &arrivalList) const override;

private:
  uint32_t m_hops;
};

class UanPhyGen : public UanPhy {
public:
  UanPhyGen();
  ~UanPhyGen() override;
  static UanModesList GetDefaultModes();

  static TypeId GetTypeId();

  void
  SetEnergyModelCallback(DeviceEnergyModel::ChangeStateCallback cb) override;
  void EnergyDepletionHandler() override;
  void EnergyRechargeHandler() override;
  void SendPacket(Ptr<Packet> pkt, uint32_t modeNum) override;
  void RegisterListener(UanPhyListener *listener) override;
  void StartRxPacket(Ptr<Packet> pkt, double rxPowerDb, UanTxMode txMode,
                     UanPdp pdp) override;
  void SetReceiveOkCallback(RxOkCallback cb) override;
  void SetReceiveErrorCallback(RxErrCallback cb) override;
  bool IsStateSleep() override;
  bool IsStateIdle() override;
  bool IsStateBusy() override;
  bool IsStateRx() override;
  bool IsStateTx() override;
  bool IsStateCcaBusy() override;
  void SetTxPowerDb(double txpwr) override;
  void SetRxThresholdDb(double thresh) override;
  void SetCcaThresholdDb(double thresh) override;
  double GetTxPowerDb() override;
  double GetRxThresholdDb() override;
  double GetCcaThresholdDb() override;
  Ptr<UanChannel> GetChannel() const override;
  Ptr<UanNetDevice> GetDevice() const override;
  Ptr<UanTransducer> GetTransducer() override;
  void SetChannel(Ptr<UanChannel> channel) override;
  void SetDevice(Ptr<UanNetDevice> device) override;
  void SetMac(Ptr<UanMac> mac) override;
  void SetTransducer(Ptr<UanTransducer> trans) override;
  void NotifyTransStartTx(Ptr<Packet> packet, double txPowerDb,
                          UanTxMode txMode) override;
  void NotifyIntChange() override;
  uint32_t GetNModes() override;
  UanTxMode GetMode(uint32_t n) override;
  Ptr<Packet> GetPacketRx() const override;
  void Clear() override;
  void SetSleepMode(bool sleep) override;
  int64_t AssignStreams(int64_t stream) override;

private:
  typedef std::list<UanPhyListener *> ListenerList;

  UanModesList m_modes;

  State m_state;
  ListenerList m_listeners;
  RxOkCallback m_recOkCb;
  RxErrCallback m_recErrCb;
  Ptr<UanChannel> m_channel;
  Ptr<UanTransducer> m_transducer;
  Ptr<UanNetDevice> m_device;
  Ptr<UanMac> m_mac;
  Ptr<UanPhyPer> m_per;
  Ptr<UanPhyCalcSinr> m_sinr;

  double m_txPwrDb;
  double m_rxThreshDb;
  double m_ccaThreshDb;

  Ptr<Packet> m_pktRx;
  Ptr<Packet> m_pktTx;
  double m_minRxSinrDb;
  double m_rxRecvPwrDb;
  Time m_pktRxArrTime;
  UanPdp m_pktRxPdp;
  UanTxMode m_pktRxMode;

  bool m_cleared;

  EventId m_txEndEvent;
  EventId m_rxEndEvent;

  Ptr<UniformRandomVariable> m_pg;

  DeviceEnergyModel::ChangeStateCallback m_energyCallback;
  ns3::TracedCallback<Ptr<const Packet>, double, UanTxMode> m_rxOkLogger;
  ns3::TracedCallback<Ptr<const Packet>, double, UanTxMode> m_rxErrLogger;
  ns3::TracedCallback<Ptr<const Packet>, double, UanTxMode> m_txLogger;

  double CalculateSinrDb(Ptr<Packet> pkt, Time arrTime, double rxPowerDb,
                         UanTxMode mode, UanPdp pdp);

  double GetInterferenceDb(Ptr<Packet> pkt);
  double DbToKp(double db);
  double KpToDb(double kp);
  void RxEndEvent(Ptr<Packet> pkt, double rxPowerDb, UanTxMode txMode);
  void TxEndEvent();
  void UpdatePowerConsumption(const State state);

  void NotifyListenersRxStart();
  void NotifyListenersRxGood();
  void NotifyListenersRxBad();
  void NotifyListenersCcaStart();
  void NotifyListenersCcaEnd();
  void NotifyListenersTxStart(Time duration);
  void NotifyListenersTxEnd();

protected:
  void DoDispose() override;
};

} // namespace ns3

#endif
