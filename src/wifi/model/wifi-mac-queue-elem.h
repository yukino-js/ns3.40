
#ifndef WIFI_MAC_QUEUE_ELEM_H
#define WIFI_MAC_QUEUE_ELEM_H

#include "qos-utils.h"

#include "ns3/callback.h"
#include "ns3/nstime.h"

#include <map>

namespace ns3 {

class WifiMpdu;

struct WifiMacQueueElem {
  Ptr<WifiMpdu> mpdu;
  Time expiryTime{0};
  AcIndex ac{AC_UNDEF};
  bool expired{false};
  std::map<uint8_t, Ptr<WifiMpdu>> inflights;
  Callback<void, Ptr<WifiMpdu>> deleter;

  WifiMacQueueElem(Ptr<WifiMpdu> item);

  ~WifiMacQueueElem();
};

} // namespace ns3

#endif
