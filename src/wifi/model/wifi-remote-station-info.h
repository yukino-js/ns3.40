
#ifndef WIFI_REMOTE_STATION_INFO_H
#define WIFI_REMOTE_STATION_INFO_H

#include "ns3/nstime.h"
#include "ns3/uinteger.h"

namespace ns3 {

class WifiRemoteStationInfo {
public:
  WifiRemoteStationInfo();
  virtual ~WifiRemoteStationInfo();

  void NotifyTxSuccess(uint32_t retryCounter);
  void NotifyTxFailed();
  double GetFrameErrorRate() const;

private:
  double CalculateAveragingCoefficient();

  Time m_memoryTime;
  Time m_lastUpdate;
  double m_failAvg;
};

} // namespace ns3

#endif
