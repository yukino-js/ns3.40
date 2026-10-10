
#ifndef MULTI_USER_SCHEDULER_H
#define MULTI_USER_SCHEDULER_H

#include "he-ru.h"

#include "ns3/ap-wifi-mac.h"
#include "ns3/ctrl-headers.h"
#include "ns3/object.h"
#include "ns3/wifi-remote-station-manager.h"
#include "ns3/wifi-tx-parameters.h"

#include <unordered_map>

namespace ns3 {

class HeFrameExchangeManager;

typedef std::unordered_map<uint16_t, Ptr<WifiPsdu>> WifiPsduMap;

class MultiUserScheduler : public Object {
public:
  static TypeId GetTypeId();
  MultiUserScheduler();
  ~MultiUserScheduler() override;

  enum TxFormat { NO_TX = 0, SU_TX, DL_MU_TX, UL_MU_TX };

  struct DlMuInfo {
    WifiPsduMap psduMap;
    WifiTxParameters txParams;
  };

  struct UlMuInfo {
    CtrlTriggerHeader trigger;
    WifiMacHeader macHdr;
    WifiTxParameters txParams;
  };

  TxFormat NotifyAccessGranted(Ptr<QosTxop> edca, Time availableTime,
                               bool initialFrame, uint16_t allowedWidth,
                               uint8_t linkId);

  DlMuInfo &GetDlMuInfo(uint8_t linkId);

  UlMuInfo &GetUlMuInfo(uint8_t linkId);

  void SetAccessReqInterval(Time interval);

protected:
  Ptr<WifiRemoteStationManager>
  GetWifiRemoteStationManager(uint8_t linkId) const;

  Ptr<HeFrameExchangeManager> GetHeFem(uint8_t linkId) const;

  Ptr<WifiMpdu> GetTriggerFrame(const CtrlTriggerHeader &trigger,
                                uint8_t linkId) const;

  TxFormat GetLastTxFormat(uint8_t linkId);

  uint32_t GetMaxSizeOfQosNullAmpdu(const CtrlTriggerHeader &trigger) const;

  void DoDispose() override;
  void NotifyNewAggregate() override;
  void DoInitialize() override;

  Ptr<ApWifiMac> m_apMac;
  Ptr<QosTxop> m_edca;
  Time m_availableTime;
  bool m_initialFrame;
  uint16_t m_allowedWidth;
  uint8_t m_linkId;

private:
  void SetWifiMac(Ptr<ApWifiMac> mac);

  void AccessReqTimeout();

  virtual TxFormat SelectTxFormat() = 0;

  virtual DlMuInfo ComputeDlMuInfo() = 0;

  virtual UlMuInfo ComputeUlMuInfo() = 0;

  void CheckTriggerFrame();

  struct LastTxInfo {
    TxFormat lastTxFormat{NO_TX};
    DlMuInfo dlInfo;
    UlMuInfo ulInfo;
  };

  std::map<uint8_t, LastTxInfo> m_lastTxInfo;
  EventId m_accessReqTimer;
  Time m_accessReqInterval;
  AcIndex m_accessReqAc;
  bool m_restartTimerUponAccess;
};

} // namespace ns3

#endif
