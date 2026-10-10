
#ifndef DEFAULT_EMLSR_MANAGER_H
#define DEFAULT_EMLSR_MANAGER_H

#include "emlsr-manager.h"

#include <optional>

namespace ns3 {

class DefaultEmlsrManager : public EmlsrManager {
public:
  static TypeId GetTypeId();

  DefaultEmlsrManager();
  ~DefaultEmlsrManager() override;

protected:
  uint8_t GetLinkToSendEmlOmn() override;
  std::optional<uint8_t> ResendNotification(Ptr<const WifiMpdu> mpdu) override;

private:
  void DoNotifyMgtFrameReceived(Ptr<const WifiMpdu> mpdu,
                                uint8_t linkId) override;
  void NotifyEmlsrModeChanged() override;
  void NotifyMainPhySwitch(uint8_t currLinkId, uint8_t nextLinkId) override;

  bool m_switchAuxPhy;
};

} // namespace ns3

#endif
