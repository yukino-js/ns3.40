
#ifndef ATHSTATS_HELPER_H
#define ATHSTATS_HELPER_H

#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/type-id.h"
#include "ns3/wifi-phy-common.h"
#include "ns3/wifi-phy-state.h"

#include <iosfwd>
#include <stdint.h>
#include <string>

namespace ns3 {

class NetDevice;
class NodeContainer;
class NetDeviceContainer;
class Packet;
class Mac48Address;
class WifiMode;

class AthstatsHelper {
public:
  AthstatsHelper();
  void EnableAthstats(std::string filename, uint32_t nodeid, uint32_t deviceid);
  void EnableAthstats(std::string filename, Ptr<NetDevice> nd);
  void EnableAthstats(std::string filename, NetDeviceContainer d);
  void EnableAthstats(std::string filename, NodeContainer n);

private:
  Time m_interval;
};

class AthstatsWifiTraceSink : public Object {
public:
  static TypeId GetTypeId();
  AthstatsWifiTraceSink();
  ~AthstatsWifiTraceSink() override;

  void DevTxTrace(std::string context, Ptr<const Packet> p);

  void DevRxTrace(std::string context, Ptr<const Packet> p);

  void TxRtsFailedTrace(std::string context, Mac48Address address);

  void TxDataFailedTrace(std::string context, Mac48Address address);

  void TxFinalRtsFailedTrace(std::string context, Mac48Address address);

  void TxFinalDataFailedTrace(std::string context, Mac48Address address);

  void PhyRxOkTrace(std::string context, Ptr<const Packet> packet, double snr,
                    WifiMode mode, WifiPreamble preamble);

  void PhyRxErrorTrace(std::string context, Ptr<const Packet> packet,
                       double snr);

  void PhyTxTrace(std::string context, Ptr<const Packet> packet, WifiMode mode,
                  WifiPreamble preamble, uint8_t txPower);

  void PhyStateTrace(std::string context, Time start, Time duration,
                     WifiPhyState state);

  void Open(const std::string &name);

private:
  void WriteStats();
  void ResetCounters();

  uint32_t m_txCount;
  uint32_t m_rxCount;
  uint32_t m_shortRetryCount;
  uint32_t m_longRetryCount;
  uint32_t m_exceededRetryCount;
  uint32_t m_phyRxOkCount;
  uint32_t m_phyRxErrorCount;
  uint32_t m_phyTxCount;

  std::ofstream *m_writer;

  Time m_interval;
};

} // namespace ns3

#endif
