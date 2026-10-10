
#ifndef AODVNEIGHBOR_H
#define AODVNEIGHBOR_H

#include "ns3/arp-cache.h"
#include "ns3/callback.h"
#include "ns3/ipv4-address.h"
#include "ns3/simulator.h"
#include "ns3/timer.h"

#include <vector>

namespace ns3 {

class WifiMacHeader;

namespace aodv {

class RoutingProtocol;

class Neighbors {
public:
  Neighbors(Time delay);

  struct Neighbor {
    Ipv4Address m_neighborAddress;
    Mac48Address m_hardwareAddress;
    Time m_expireTime;
    bool close;

    Neighbor(Ipv4Address ip, Mac48Address mac, Time t)
        : m_neighborAddress(ip), m_hardwareAddress(mac), m_expireTime(t),
          close(false) {}
  };

  Time GetExpireTime(Ipv4Address addr);
  bool IsNeighbor(Ipv4Address addr);
  void Update(Ipv4Address addr, Time expire);
  void Purge();
  void ScheduleTimer();

  void Clear() { m_nb.clear(); }

  void AddArpCache(Ptr<ArpCache> a);
  void DelArpCache(Ptr<ArpCache> a);

  Callback<void, const WifiMacHeader &> GetTxErrorCallback() const {
    return m_txErrorCallback;
  }

  void SetCallback(Callback<void, Ipv4Address> cb) { m_handleLinkFailure = cb; }

  Callback<void, Ipv4Address> GetCallback() const {
    return m_handleLinkFailure;
  }

private:
  Callback<void, Ipv4Address> m_handleLinkFailure;
  Callback<void, const WifiMacHeader &> m_txErrorCallback;
  Timer m_ntimer;
  std::vector<Neighbor> m_nb;
  std::vector<Ptr<ArpCache>> m_arp;

  Mac48Address LookupMacAddress(Ipv4Address addr);
  void ProcessTxError(const WifiMacHeader &hdr);
};

} // namespace aodv
} // namespace ns3

#endif
