
#include "burst-profile-manager.h"

#include "bs-net-device.h"
#include "mac-messages.h"
#include "ss-manager.h"
#include "ss-net-device.h"
#include "ss-record.h"

#include "ns3/log.h"

#include <stdint.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("BurstProfileManager");

NS_OBJECT_ENSURE_REGISTERED(BurstProfileManager);

TypeId BurstProfileManager::GetTypeId() {
  static TypeId tid = TypeId("ns3::BurstProfileManager")
                          .SetParent<Object>()
                          .SetGroupName("Wimax");
  return tid;
}

BurstProfileManager::BurstProfileManager(Ptr<WimaxNetDevice> device)
    : m_device(device) {}

BurstProfileManager::~BurstProfileManager() { m_device = nullptr; }

void BurstProfileManager::DoDispose() { m_device = nullptr; }

uint16_t BurstProfileManager::GetNrBurstProfilesToDefine() { return 7; }

WimaxPhy::ModulationType BurstProfileManager::GetModulationType(
    uint8_t iuc, WimaxNetDevice::Direction direction) const {
  if (direction == WimaxNetDevice::DIRECTION_DOWNLINK) {
    std::vector<OfdmDlBurstProfile> dlBurstProfiles =
        m_device->GetCurrentDcd().GetDlBurstProfiles();
    for (auto iter = dlBurstProfiles.begin(); iter != dlBurstProfiles.end();
         ++iter) {
      if (iter->GetDiuc() == iuc) {
        return (WimaxPhy::ModulationType)iter->GetFecCodeType();
      }
    }
  } else {
    std::vector<OfdmUlBurstProfile> ulBurstProfiles =
        m_device->GetCurrentUcd().GetUlBurstProfiles();
    for (auto iter = ulBurstProfiles.begin(); iter != ulBurstProfiles.end();
         ++iter) {
      if (iter->GetUiuc() == iuc) {
        return (WimaxPhy::ModulationType)iter->GetFecCodeType();
      }
    }
  }

  NS_FATAL_ERROR("burst profile got to be there in DCD/UCD");

  return (WimaxPhy::ModulationType)-1;
}

uint8_t BurstProfileManager::GetBurstProfile(
    WimaxPhy::ModulationType modulationType,
    WimaxNetDevice::Direction direction) const {
  if (direction == WimaxNetDevice::DIRECTION_DOWNLINK) {
    std::vector<OfdmDlBurstProfile> dlBurstProfiles =
        m_device->GetCurrentDcd().GetDlBurstProfiles();
    for (auto iter = dlBurstProfiles.begin(); iter != dlBurstProfiles.end();
         ++iter) {
      if (iter->GetFecCodeType() == modulationType) {
        return iter->GetDiuc();
      }
    }
  } else {
    std::vector<OfdmUlBurstProfile> ulBurstProfiles =
        m_device->GetCurrentUcd().GetUlBurstProfiles();
    for (auto iter = ulBurstProfiles.begin(); iter != ulBurstProfiles.end();
         ++iter) {
      if (iter->GetFecCodeType() == modulationType) {
        return iter->GetUiuc();
      }
    }
  }

  NS_FATAL_ERROR("burst profile got to be there in DCD/UCD");

  return ~0;
}

uint8_t BurstProfileManager::GetBurstProfileForSS(
    const SSRecord *ssRecord, const RngReq *rngreq,
    WimaxPhy::ModulationType &modulationType) const {

  modulationType = GetModulationTypeForSS(ssRecord, rngreq);
  return GetBurstProfile(modulationType, WimaxNetDevice::DIRECTION_DOWNLINK);
}

WimaxPhy::ModulationType
BurstProfileManager::GetModulationTypeForSS(const SSRecord *ssRecord,
                                            const RngReq *rngreq) const {
  return GetModulationType(rngreq->GetReqDlBurstProfile(),
                           WimaxNetDevice::DIRECTION_DOWNLINK);
}

uint8_t BurstProfileManager::GetBurstProfileToRequest() {

  return GetBurstProfile(
      m_device->GetObject<SubscriberStationNetDevice>()->GetModulationType(),
      WimaxNetDevice::DIRECTION_DOWNLINK);
}

} // namespace ns3
