
#ifndef UAN_PHY_DUAL_H
#define UAN_PHY_DUAL_H

#include "uan-phy.h"

namespace ns3 {

class UanTxMode;
class UanModesList;

class UanPhyCalcSinrDual : public UanPhyCalcSinr {
public:
  UanPhyCalcSinrDual();
  ~UanPhyCalcSinrDual() override;

  static TypeId GetTypeId();

  double
  CalcSinrDb(Ptr<Packet> pkt, Time arrTime, double rxPowerDb, double ambNoiseDb,
             UanTxMode mode, UanPdp pdp,
             const UanTransducer::ArrivalList &arrivalList) const override;
};

class UanPhyDual : public UanPhy {
public:
  UanPhyDual();
  ~UanPhyDual() override;

  static TypeId GetTypeId();

  void SetEnergyModelCallback(
      DeviceEnergyModel::ChangeStateCallback callback) override;
  void EnergyDepletionHandler() override;
  void EnergyRechargeHandler() override;
  void SendPacket(Ptr<Packet> pkt, uint32_t modeNum) override;

  void RegisterListener(UanPhyListener *listener) override;
  void StartRxPacket(Ptr<Packet> pkt, double rxPowerDb, UanTxMode txMode,
                     UanPdp pdp) override;
  void SetReceiveOkCallback(RxOkCallback cb) override;
  void SetReceiveErrorCallback(RxErrCallback cb) override;
  void SetTxPowerDb(double txpwr) override;
  void SetRxThresholdDb(double thresh) override;
  void SetCcaThresholdDb(double thresh) override;
  double GetTxPowerDb() override;
  double GetRxThresholdDb() override;
  double GetCcaThresholdDb() override;
  bool IsStateSleep() override;
  bool IsStateIdle() override;
  bool IsStateBusy() override;
  bool IsStateRx() override;
  bool IsStateTx() override;
  bool IsStateCcaBusy() override;
  Ptr<UanChannel> GetChannel() const override;
  Ptr<UanNetDevice> GetDevice() const override;
  void SetChannel(Ptr<UanChannel> channel) override;
  void SetDevice(Ptr<UanNetDevice> device) override;
  void SetMac(Ptr<UanMac> mac) override;
  void NotifyTransStartTx(Ptr<Packet> packet, double txPowerDb,
                          UanTxMode txMode) override;
  void NotifyIntChange() override;
  void SetTransducer(Ptr<UanTransducer> trans) override;
  Ptr<UanTransducer> GetTransducer() override;
  uint32_t GetNModes() override;
  UanTxMode GetMode(uint32_t n) override;
  void Clear() override;

  void SetSleepMode(bool) override {}

  int64_t AssignStreams(int64_t stream) override;
  Ptr<Packet> GetPacketRx() const override;

  bool IsPhy1Idle();
  bool IsPhy2Idle();
  bool IsPhy1Rx();
  bool IsPhy2Rx();
  bool IsPhy1Tx();
  bool IsPhy2Tx();

  double GetCcaThresholdPhy1() const;
  double GetCcaThresholdPhy2() const;
  void SetCcaThresholdPhy1(double thresh);
  void SetCcaThresholdPhy2(double thresh);

  double GetTxPowerDbPhy1() const;
  double GetTxPowerDbPhy2() const;
  void SetTxPowerDbPhy1(double txpwr);
  void SetTxPowerDbPhy2(double txpwr);

  UanModesList GetModesPhy1() const;
  UanModesList GetModesPhy2() const;

  void SetModesPhy1(UanModesList modes);
  void SetModesPhy2(UanModesList modes);

  Ptr<UanPhyPer> GetPerModelPhy1() const;
  Ptr<UanPhyPer> GetPerModelPhy2() const;

  void SetPerModelPhy1(Ptr<UanPhyPer> per);
  void SetPerModelPhy2(Ptr<UanPhyPer> per);

  Ptr<UanPhyCalcSinr> GetSinrModelPhy1() const;
  Ptr<UanPhyCalcSinr> GetSinrModelPhy2() const;

  void SetSinrModelPhy1(Ptr<UanPhyCalcSinr> calcSinr);
  void SetSinrModelPhy2(Ptr<UanPhyCalcSinr> calcSinr);

  Ptr<Packet> GetPhy1PacketRx() const;
  Ptr<Packet> GetPhy2PacketRx() const;

private:
  Ptr<UanPhy> m_phy1;
  Ptr<UanPhy> m_phy2;

  ns3::TracedCallback<Ptr<const Packet>, double, UanTxMode> m_rxOkLogger;
  ns3::TracedCallback<Ptr<const Packet>, double, UanTxMode> m_rxErrLogger;
  ns3::TracedCallback<Ptr<const Packet>, double, UanTxMode> m_txLogger;
  RxOkCallback m_recOkCb;
  RxErrCallback m_recErrCb;

  void RxOkFromSubPhy(Ptr<Packet> pkt, double sinr, UanTxMode mode);
  void RxErrFromSubPhy(Ptr<Packet> pkt, double sinr);

protected:
  void DoDispose() override;
};

} // namespace ns3

#endif
