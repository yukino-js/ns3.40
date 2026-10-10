
#ifndef UAN_PHY_H
#define UAN_PHY_H

#include "uan-mac.h"
#include "uan-prop-model.h"
#include "uan-transducer.h"
#include "uan-tx-mode.h"

#include "ns3/device-energy-model.h"
#include "ns3/object.h"

namespace ns3 {

class UanPhyCalcSinr : public Object {
public:
  virtual double
  CalcSinrDb(Ptr<Packet> pkt, Time arrTime, double rxPowerDb, double ambNoiseDb,
             UanTxMode mode, UanPdp pdp,
             const UanTransducer::ArrivalList &arrivalList) const = 0;
  static TypeId GetTypeId();

  virtual void Clear();

  inline double DbToKp(double db) const { return std::pow(10, db / 10.0); }

  inline double KpToDb(double kp) const { return 10 * std::log10(kp); }

protected:
  void DoDispose() override;
};

class UanPhyPer : public Object {
public:
  virtual double CalcPer(Ptr<Packet> pkt, double sinrDb, UanTxMode mode) = 0;

  static TypeId GetTypeId();
  virtual void Clear();

protected:
  void DoDispose() override;
};

class UanPhyListener {
public:
  virtual ~UanPhyListener() {}

  virtual void NotifyRxStart() = 0;
  virtual void NotifyRxEndOk() = 0;
  virtual void NotifyRxEndError() = 0;
  virtual void NotifyCcaStart() = 0;
  virtual void NotifyCcaEnd() = 0;
  virtual void NotifyTxStart(Time duration) = 0;
  virtual void NotifyTxEnd() = 0;
};

class UanPhy : public Object {
public:
  enum State { IDLE, CCABUSY, RX, TX, SLEEP, DISABLED };

  typedef Callback<void, Ptr<Packet>, double, UanTxMode> RxOkCallback;

  typedef Callback<void, Ptr<Packet>, double> RxErrCallback;

  typedef void (*TracedCallback)(Ptr<const Packet> pkt, double sinr,
                                 UanTxMode mode);

  virtual void
  SetEnergyModelCallback(DeviceEnergyModel::ChangeStateCallback callback) = 0;
  virtual void EnergyDepletionHandler() = 0;
  virtual void EnergyRechargeHandler() = 0;
  virtual void SendPacket(Ptr<Packet> pkt, uint32_t modeNum) = 0;

  virtual void RegisterListener(UanPhyListener *listener) = 0;

  virtual void StartRxPacket(Ptr<Packet> pkt, double rxPowerDb,
                             UanTxMode txMode, UanPdp pdp) = 0;

  virtual void SetReceiveOkCallback(RxOkCallback cb) = 0;

  virtual void SetReceiveErrorCallback(RxErrCallback cb) = 0;

  virtual void SetTxPowerDb(double txpwr) = 0;

  virtual void SetRxThresholdDb(double thresh) = 0;

  virtual void SetCcaThresholdDb(double thresh) = 0;

  virtual double GetTxPowerDb() = 0;

  virtual double GetRxThresholdDb() = 0;

  virtual double GetCcaThresholdDb() = 0;
  virtual bool IsStateSleep() = 0;
  virtual bool IsStateIdle() = 0;
  virtual bool IsStateBusy() = 0;
  virtual bool IsStateRx() = 0;
  virtual bool IsStateTx() = 0;
  virtual bool IsStateCcaBusy() = 0;

  virtual Ptr<UanChannel> GetChannel() const = 0;

  virtual Ptr<UanNetDevice> GetDevice() const = 0;

  virtual void SetChannel(Ptr<UanChannel> channel) = 0;

  virtual void SetDevice(Ptr<UanNetDevice> device) = 0;

  virtual void SetMac(Ptr<UanMac> mac) = 0;

  virtual void NotifyTransStartTx(Ptr<Packet> packet, double txPowerDb,
                                  UanTxMode txMode) = 0;

  virtual void NotifyIntChange() = 0;

  virtual void SetTransducer(Ptr<UanTransducer> trans) = 0;

  virtual Ptr<UanTransducer> GetTransducer() = 0;

  virtual uint32_t GetNModes() = 0;

  virtual UanTxMode GetMode(uint32_t n) = 0;

  virtual Ptr<Packet> GetPacketRx() const = 0;

  virtual void Clear() = 0;

  virtual void SetSleepMode(bool sleep) = 0;

  void NotifyTxBegin(Ptr<const Packet> packet);

  void NotifyTxEnd(Ptr<const Packet> packet);

  void NotifyTxDrop(Ptr<const Packet> packet);

  void NotifyRxBegin(Ptr<const Packet> packet);

  void NotifyRxEnd(Ptr<const Packet> packet);

  void NotifyRxDrop(Ptr<const Packet> packet);

  virtual int64_t AssignStreams(int64_t stream) = 0;

  static TypeId GetTypeId();

private:
  ns3::TracedCallback<Ptr<const Packet>> m_phyTxBeginTrace;

  ns3::TracedCallback<Ptr<const Packet>> m_phyTxEndTrace;

  ns3::TracedCallback<Ptr<const Packet>> m_phyTxDropTrace;

  ns3::TracedCallback<Ptr<const Packet>> m_phyRxBeginTrace;

  ns3::TracedCallback<Ptr<const Packet>> m_phyRxEndTrace;

  ns3::TracedCallback<Ptr<const Packet>> m_phyRxDropTrace;
};

} // namespace ns3

#endif
