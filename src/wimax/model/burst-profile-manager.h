
#ifndef BURST_PROFILE_MANAGER_H
#define BURST_PROFILE_MANAGER_H

#include "cid.h"
#include "wimax-net-device.h"
#include "wimax-phy.h"

#include <stdint.h>

namespace ns3 {

class SSRecord;
class RngReq;

class BurstProfileManager : public Object {
public:
  static TypeId GetTypeId();
  BurstProfileManager(Ptr<WimaxNetDevice> device);
  ~BurstProfileManager() override;

  BurstProfileManager(const BurstProfileManager &) = delete;
  BurstProfileManager &operator=(const BurstProfileManager &) = delete;

  void DoDispose() override;
  uint16_t GetNrBurstProfilesToDefine();

  WimaxPhy::ModulationType
  GetModulationType(uint8_t iuc, WimaxNetDevice::Direction direction) const;

  uint8_t GetBurstProfile(WimaxPhy::ModulationType modulationType,
                          WimaxNetDevice::Direction direction) const;

  uint8_t GetBurstProfileForSS(const SSRecord *ssRecord, const RngReq *rngreq,
                               WimaxPhy::ModulationType &modulationType) const;
  WimaxPhy::ModulationType GetModulationTypeForSS(const SSRecord *ssRecord,
                                                  const RngReq *rngreq) const;
  uint8_t GetBurstProfileToRequest();

private:
  Ptr<WimaxNetDevice> m_device;
};

} // namespace ns3

#endif
