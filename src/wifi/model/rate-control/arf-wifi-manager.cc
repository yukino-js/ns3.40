
#include "arf-wifi-manager.h"

#include "ns3/log.h"
#include "ns3/wifi-tx-vector.h"

#define Min(a, b) ((a < b) ? a : b)

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("ArfWifiManager");

struct ArfWifiRemoteStation : public WifiRemoteStation {
  uint32_t m_timer;
  uint32_t m_success;
  uint32_t m_failed;
  bool m_recovery;
  uint32_t m_timerTimeout;
  uint32_t m_successThreshold;
  uint8_t m_rate;
};

NS_OBJECT_ENSURE_REGISTERED(ArfWifiManager);

TypeId ArfWifiManager::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::ArfWifiManager")
          .SetParent<WifiRemoteStationManager>()
          .SetGroupName("Wifi")
          .AddConstructor<ArfWifiManager>()
          .AddAttribute("TimerThreshold",
                        "The 'timer' threshold in the ARF algorithm.",
                        UintegerValue(15),
                        MakeUintegerAccessor(&ArfWifiManager::m_timerThreshold),
                        MakeUintegerChecker<uint32_t>())
          .AddAttribute(
              "SuccessThreshold",
              "The minimum number of successful transmissions to try a new "
              "rate.",
              UintegerValue(10),
              MakeUintegerAccessor(&ArfWifiManager::m_successThreshold),
              MakeUintegerChecker<uint32_t>())
          .AddTraceSource(
              "Rate", "Traced value for rate changes (b/s)",
              MakeTraceSourceAccessor(&ArfWifiManager::m_currentRate),
              "ns3::TracedValueCallback::Uint64");
  return tid;
}

ArfWifiManager::ArfWifiManager()
    : WifiRemoteStationManager(), m_currentRate(0) {
  NS_LOG_FUNCTION(this);
}

ArfWifiManager::~ArfWifiManager() { NS_LOG_FUNCTION(this); }

void ArfWifiManager::DoInitialize() {
  NS_LOG_FUNCTION(this);
  if (GetHtSupported()) {
    NS_FATAL_ERROR(
        "WifiRemoteStationManager selected does not support HT rates");
  }
  if (GetVhtSupported()) {
    NS_FATAL_ERROR(
        "WifiRemoteStationManager selected does not support VHT rates");
  }
  if (GetHeSupported()) {
    NS_FATAL_ERROR(
        "WifiRemoteStationManager selected does not support HE rates");
  }
}

WifiRemoteStation *ArfWifiManager::DoCreateStation() const {
  NS_LOG_FUNCTION(this);
  auto station = new ArfWifiRemoteStation();

  station->m_successThreshold = m_successThreshold;
  station->m_timerTimeout = m_timerThreshold;
  station->m_rate = 0;
  station->m_success = 0;
  station->m_failed = 0;
  station->m_recovery = false;
  station->m_timer = 0;

  return station;
}

void ArfWifiManager::DoReportRtsFailed(WifiRemoteStation *station) {
  NS_LOG_FUNCTION(this << station);
}

void ArfWifiManager::DoReportDataFailed(WifiRemoteStation *st) {
  NS_LOG_FUNCTION(this << st);
  auto station = static_cast<ArfWifiRemoteStation *>(st);
  station->m_timer++;
  station->m_failed++;
  station->m_success = 0;

  if (station->m_recovery) {
    NS_ASSERT(station->m_failed >= 1);
    if (station->m_failed == 1) {
      if (station->m_rate != 0) {
        station->m_rate--;
      }
    }
    station->m_timer = 0;
  } else {
    NS_ASSERT(station->m_failed >= 1);
    if (((station->m_failed - 1) % 2) == 1) {
      if (station->m_rate != 0) {
        station->m_rate--;
      }
    }
    if (station->m_failed >= 2) {
      station->m_timer = 0;
    }
  }
}

void ArfWifiManager::DoReportRxOk(WifiRemoteStation *station, double rxSnr,
                                  WifiMode txMode) {
  NS_LOG_FUNCTION(this << station << rxSnr << txMode);
}

void ArfWifiManager::DoReportRtsOk(WifiRemoteStation *station, double ctsSnr,
                                   WifiMode ctsMode, double rtsSnr) {
  NS_LOG_FUNCTION(this << station << ctsSnr << ctsMode << rtsSnr);
  NS_LOG_DEBUG("station=" << station << " rts ok");
}

void ArfWifiManager::DoReportDataOk(WifiRemoteStation *st, double ackSnr,
                                    WifiMode ackMode, double dataSnr,
                                    uint16_t dataChannelWidth,
                                    uint8_t dataNss) {
  NS_LOG_FUNCTION(this << st << ackSnr << ackMode << dataSnr << dataChannelWidth
                       << +dataNss);
  auto station = static_cast<ArfWifiRemoteStation *>(st);
  station->m_timer++;
  station->m_success++;
  station->m_failed = 0;
  station->m_recovery = false;
  NS_LOG_DEBUG("station=" << station
                          << " data ok success=" << station->m_success
                          << ", timer=" << station->m_timer);
  if ((station->m_success == m_successThreshold ||
       station->m_timer == m_timerThreshold) &&
      (station->m_rate < (station->m_state->m_operationalRateSet.size() - 1))) {
    NS_LOG_DEBUG("station=" << station << " inc rate");
    station->m_rate++;
    station->m_timer = 0;
    station->m_success = 0;
    station->m_recovery = true;
  }
}

void ArfWifiManager::DoReportFinalRtsFailed(WifiRemoteStation *station) {
  NS_LOG_FUNCTION(this << station);
}

void ArfWifiManager::DoReportFinalDataFailed(WifiRemoteStation *station) {
  NS_LOG_FUNCTION(this << station);
}

WifiTxVector ArfWifiManager::DoGetDataTxVector(WifiRemoteStation *st,
                                               uint16_t allowedWidth) {
  NS_LOG_FUNCTION(this << st << allowedWidth);
  auto station = static_cast<ArfWifiRemoteStation *>(st);
  uint16_t channelWidth = GetChannelWidth(station);
  if (channelWidth > 20 && channelWidth != 22) {
    channelWidth = 20;
  }
  WifiMode mode = GetSupported(station, station->m_rate);
  uint64_t rate = mode.GetDataRate(channelWidth);
  if (m_currentRate != rate) {
    NS_LOG_DEBUG("New datarate: " << rate);
    m_currentRate = rate;
  }
  return WifiTxVector(mode, GetDefaultTxPowerLevel(),
                      GetPreambleForTransmission(mode.GetModulationClass(),
                                                 GetShortPreambleEnabled()),
                      800, 1, 1, 0, channelWidth, GetAggregation(station));
}

WifiTxVector ArfWifiManager::DoGetRtsTxVector(WifiRemoteStation *st) {
  NS_LOG_FUNCTION(this << st);
  auto station = static_cast<ArfWifiRemoteStation *>(st);
  uint16_t channelWidth = GetChannelWidth(station);
  if (channelWidth > 20 && channelWidth != 22) {
    channelWidth = 20;
  }
  WifiMode mode;
  if (!GetUseNonErpProtection()) {
    mode = GetSupported(station, 0);
  } else {
    mode = GetNonErpSupported(station, 0);
  }
  return WifiTxVector(mode, GetDefaultTxPowerLevel(),
                      GetPreambleForTransmission(mode.GetModulationClass(),
                                                 GetShortPreambleEnabled()),
                      800, 1, 1, 0, channelWidth, GetAggregation(station));
}

} // namespace ns3
