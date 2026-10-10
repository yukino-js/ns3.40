#ifndef TRAFFICCONTROLLAYER_H
#define TRAFFICCONTROLLAYER_H

#include "ns3/address.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/object.h"
#include "ns3/queue-item.h"
#include "ns3/traced-callback.h"

#include <map>
#include <vector>

namespace ns3 {

class Packet;
class QueueDisc;
class NetDeviceQueueInterface;

class TrafficControlLayer : public Object {
public:
  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  TrafficControlLayer();

  ~TrafficControlLayer() override;

  TrafficControlLayer(const TrafficControlLayer &) = delete;
  TrafficControlLayer &operator=(const TrafficControlLayer &) = delete;

  void RegisterProtocolHandler(Node::ProtocolHandler handler,
                               uint16_t protocolType, Ptr<NetDevice> device);

  typedef std::vector<Ptr<QueueDisc>> QueueDiscVector;

  virtual void ScanDevices();

  virtual void SetRootQueueDiscOnDevice(Ptr<NetDevice> device,
                                        Ptr<QueueDisc> qDisc);

  virtual Ptr<QueueDisc> GetRootQueueDiscOnDevice(Ptr<NetDevice> device) const;

  virtual void DeleteRootQueueDiscOnDevice(Ptr<NetDevice> device);

  void SetNode(Ptr<Node> node);

  virtual void Receive(Ptr<NetDevice> device, Ptr<const Packet> p,
                       uint16_t protocol, const Address &from,
                       const Address &to, NetDevice::PacketType packetType);
  virtual void Send(Ptr<NetDevice> device, Ptr<QueueDiscItem> item);

protected:
  void DoDispose() override;
  void DoInitialize() override;
  void NotifyNewAggregate() override;

private:
  struct ProtocolHandlerEntry {
    Node::ProtocolHandler handler;
    Ptr<NetDevice> device;
    uint16_t protocol;
    bool promiscuous;
  };

  struct NetDeviceInfo {
    Ptr<QueueDisc> m_rootQueueDisc;
    Ptr<NetDeviceQueueInterface> m_ndqi;
    QueueDiscVector m_queueDiscsToWake;
  };

  typedef std::vector<ProtocolHandlerEntry> ProtocolHandlerList;

  uint32_t GetNDevices() const;
  Ptr<QueueDisc> GetRootQueueDiscOnDeviceByIndex(uint32_t index) const;

  Ptr<Node> m_node;
  std::map<Ptr<NetDevice>, NetDeviceInfo> m_netDevices;
  ProtocolHandlerList m_handlers;

  TracedCallback<Ptr<const Packet>> m_dropped;
};

} // namespace ns3

#endif
