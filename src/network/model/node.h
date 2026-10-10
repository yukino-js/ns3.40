#ifndef NODE_H
#define NODE_H

#include "net-device.h"

#include "ns3/callback.h"
#include "ns3/object.h"
#include "ns3/ptr.h"

#include <vector>

namespace ns3 {

class Application;
class Packet;
class Address;
class Time;

class Node : public Object {
public:
  static TypeId GetTypeId();

  Node();
  Node(uint32_t systemId);

  ~Node() override;

  uint32_t GetId() const;

  Time GetLocalTime() const;

  uint32_t GetSystemId() const;

  void SetSystemId(uint32_t systemId);

  uint32_t AddDevice(Ptr<NetDevice> device);
  Ptr<NetDevice> GetDevice(uint32_t index) const;
  uint32_t GetNDevices() const;

  uint32_t AddApplication(Ptr<Application> application);
  Ptr<Application> GetApplication(uint32_t index) const;

  uint32_t GetNApplications() const;

  typedef Callback<void, Ptr<NetDevice>, Ptr<const Packet>, uint16_t,
                   const Address &, const Address &, NetDevice::PacketType>
      ProtocolHandler;
  void RegisterProtocolHandler(ProtocolHandler handler, uint16_t protocolType,
                               Ptr<NetDevice> device, bool promiscuous = false);
  void UnregisterProtocolHandler(ProtocolHandler handler);

  typedef Callback<void, Ptr<NetDevice>> DeviceAdditionListener;
  void RegisterDeviceAdditionListener(DeviceAdditionListener listener);
  void UnregisterDeviceAdditionListener(DeviceAdditionListener listener);

  static bool ChecksumEnabled();

protected:
  void DoDispose() override;
  void DoInitialize() override;

private:
  void NotifyDeviceAdded(Ptr<NetDevice> device);

  bool NonPromiscReceiveFromDevice(Ptr<NetDevice> device,
                                   Ptr<const Packet> packet, uint16_t protocol,
                                   const Address &from);
  bool PromiscReceiveFromDevice(Ptr<NetDevice> device, Ptr<const Packet> packet,
                                uint16_t protocol, const Address &from,
                                const Address &to,
                                NetDevice::PacketType packetType);
  bool ReceiveFromDevice(Ptr<NetDevice> device, Ptr<const Packet>,
                         uint16_t protocol, const Address &from,
                         const Address &to, NetDevice::PacketType packetType,
                         bool promisc);

  void Construct();

  struct ProtocolHandlerEntry {
    ProtocolHandler handler;
    Ptr<NetDevice> device;
    uint16_t protocol;
    bool promiscuous;
  };

  typedef std::vector<Node::ProtocolHandlerEntry> ProtocolHandlerList;
  typedef std::vector<DeviceAdditionListener> DeviceAdditionListenerList;

  uint32_t m_id;
  uint32_t m_sid;
  std::vector<Ptr<NetDevice>> m_devices;
  std::vector<Ptr<Application>> m_applications;
  ProtocolHandlerList m_handlers;
  DeviceAdditionListenerList m_deviceAdditionListeners;
};

} // namespace ns3

#endif
