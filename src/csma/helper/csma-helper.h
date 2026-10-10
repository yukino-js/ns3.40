#ifndef CSMA_HELPER_H
#define CSMA_HELPER_H

#include "ns3/attribute.h"
#include "ns3/csma-channel.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/queue.h"
#include "ns3/trace-helper.h"

#include <string>

namespace ns3 {

class Packet;

class CsmaHelper : public PcapHelperForDevice,
                   public AsciiTraceHelperForDevice {
public:
  CsmaHelper();

  ~CsmaHelper() override {}

  template <typename... Ts> void SetQueue(std::string type, Ts &&...args);

  void SetDeviceAttribute(std::string n1, const AttributeValue &v1);

  void SetChannelAttribute(std::string n1, const AttributeValue &v1);

  void DisableFlowControl();

  NetDeviceContainer Install(Ptr<Node> node) const;

  NetDeviceContainer Install(std::string name) const;

  NetDeviceContainer Install(Ptr<Node> node, Ptr<CsmaChannel> channel) const;

  NetDeviceContainer Install(Ptr<Node> node, std::string channelName) const;

  NetDeviceContainer Install(std::string nodeName,
                             Ptr<CsmaChannel> channel) const;

  NetDeviceContainer Install(std::string nodeName,
                             std::string channelName) const;

  NetDeviceContainer Install(const NodeContainer &c) const;

  NetDeviceContainer Install(const NodeContainer &c,
                             Ptr<CsmaChannel> channel) const;

  NetDeviceContainer Install(const NodeContainer &c,
                             std::string channelName) const;

  int64_t AssignStreams(NetDeviceContainer c, int64_t stream);

private:
  Ptr<NetDevice> InstallPriv(Ptr<Node> node, Ptr<CsmaChannel> channel) const;

  void EnablePcapInternal(std::string prefix, Ptr<NetDevice> nd,
                          bool promiscuous, bool explicitFilename) override;

  void EnableAsciiInternal(Ptr<OutputStreamWrapper> stream, std::string prefix,
                           Ptr<NetDevice> nd, bool explicitFilename) override;

  ObjectFactory m_queueFactory;
  ObjectFactory m_deviceFactory;
  ObjectFactory m_channelFactory;
  bool m_enableFlowControl;
};

template <typename... Ts>
void CsmaHelper::SetQueue(std::string type, Ts &&...args) {
  QueueBase::AppendItemTypeIfNotPresent(type, "Packet");

  m_queueFactory.SetTypeId(type);
  m_queueFactory.Set(std::forward<Ts>(args)...);
}

} // namespace ns3

#endif
