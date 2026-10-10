
#ifndef APARF_WIFI_MANAGER_H
#define APARF_WIFI_MANAGER_H

#include "ns3/wifi-remote-station-manager.h"

namespace ns3 {

struct AparfWifiRemoteStation;

class AparfWifiManager : public WifiRemoteStationManager {
public:
  static TypeId GetTypeId();
  AparfWifiManager();
  ~AparfWifiManager() override;

  void SetupPhy(const Ptr<WifiPhy> phy) override;

  enum State { High, Low, Spread };

private:
  void DoInitialize() override;
  WifiRemoteStation *DoCreateStation() const override;
  void DoReportRxOk(WifiRemoteStation *station, double rxSnr,
                    WifiMode txMode) override;
  void DoReportRtsFailed(WifiRemoteStation *station) override;
  void DoReportDataFailed(WifiRemoteStation *station) override;
  void DoReportRtsOk(WifiRemoteStation *station, double ctsSnr,
                     WifiMode ctsMode, double rtsSnr) override;
  void DoReportDataOk(WifiRemoteStation *station, double ackSnr,
                      WifiMode ackMode, double dataSnr,
                      uint16_t dataChannelWidth, uint8_t dataNss) override;
  void DoReportFinalRtsFailed(WifiRemoteStation *station) override;
  void DoReportFinalDataFailed(WifiRemoteStation *station) override;
  WifiTxVector DoGetDataTxVector(WifiRemoteStation *station,
                                 uint16_t allowedWidth) override;
  WifiTxVector DoGetRtsTxVector(WifiRemoteStation *station) override;

  void CheckInit(AparfWifiRemoteStation *station);

  uint32_t m_successMax1;
  uint32_t m_successMax2;
  uint32_t m_failMax;
  uint32_t m_powerMax;
  uint8_t m_powerInc;
  uint8_t m_powerDec;
  uint8_t m_rateInc;
  uint8_t m_rateDec;

  uint8_t m_minPower;

  uint8_t m_maxPower;

  TracedCallback<double, double, Mac48Address> m_powerChange;
  TracedCallback<DataRate, DataRate, Mac48Address> m_rateChange;
};

} // namespace ns3

#endif
