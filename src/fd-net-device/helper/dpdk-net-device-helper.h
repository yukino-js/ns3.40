
#ifndef DPDK_NET_DEVICE_HELPER_H
#define DPDK_NET_DEVICE_HELPER_H

#include "emu-fd-net-device-helper.h"

namespace ns3 {

class DpdkNetDeviceHelper : public EmuFdNetDeviceHelper {
public:
  DpdkNetDeviceHelper();

  ~DpdkNetDeviceHelper() override {}

  void SetLCoreList(std::string lCoreList);

  void SetPmdLibrary(std::string pmdLibrary);

  void SetDpdkDriver(std::string dpdkDriver);

protected:
  Ptr<NetDevice> InstallPriv(Ptr<Node> node) const override;

  std::string m_lCoreList;

  std::string m_pmdLibrary;

  std::string m_dpdkDriver;
};

} // namespace ns3

#endif
