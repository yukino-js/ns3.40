
#include "wifi-mac-queue-elem.h"

#include "wifi-mpdu.h"

namespace ns3 {

WifiMacQueueElem::WifiMacQueueElem(Ptr<WifiMpdu> item)
    : mpdu(item), expiryTime(0), ac(AC_UNDEF), expired(false) {}

WifiMacQueueElem::~WifiMacQueueElem() {
  deleter(mpdu);
  inflights.clear();
}

} // namespace ns3
