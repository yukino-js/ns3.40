
#ifndef LTE_SIMPLE_HELPER_H
#define LTE_SIMPLE_HELPER_H

#include "ns3/lte-pdcp.h"
#include "ns3/lte-rlc-am.h"
#include "ns3/lte-rlc-um.h"
#include "ns3/lte-rlc.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/radio-bearer-stats-calculator.h"
#include "ns3/simple-channel.h"

namespace ns3 {

class LteTestRrc;
class LteTestMac;

class LteSimpleHelper : public Object {
public:
  LteSimpleHelper();
  ~LteSimpleHelper() override;

  static TypeId GetTypeId();
  void DoDispose() override;

  NetDeviceContainer InstallEnbDevice(NodeContainer c);

  NetDeviceContainer InstallUeDevice(NodeContainer c);

  void EnableLogComponents();

  void EnableTraces();

  void EnableRlcTraces();

  void EnableDlRlcTraces();

  void EnableUlRlcTraces();

  void EnablePdcpTraces();

  void EnableDlPdcpTraces();

  void EnableUlPdcpTraces();

protected:
  void DoInitialize() override;

private:
  Ptr<NetDevice> InstallSingleEnbDevice(Ptr<Node> n);
  Ptr<NetDevice> InstallSingleUeDevice(Ptr<Node> n);

  Ptr<SimpleChannel> m_phyChannel;

public:
  Ptr<LteTestRrc> m_enbRrc;
  Ptr<LteTestRrc> m_ueRrc;

  Ptr<LteTestMac> m_enbMac;
  Ptr<LteTestMac> m_ueMac;

private:
  Ptr<LtePdcp> m_enbPdcp;
  Ptr<LteRlc> m_enbRlc;

  Ptr<LtePdcp> m_uePdcp;
  Ptr<LteRlc> m_ueRlc;

  ObjectFactory m_enbDeviceFactory;
  ObjectFactory m_ueDeviceFactory;

  enum LteRlcEntityType_t { RLC_UM = 1, RLC_AM = 2 } m_lteRlcEntityType;
};

} // namespace ns3

#endif
