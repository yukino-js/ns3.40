#ifndef PACKET_SOCKET_H
#define PACKET_SOCKET_H

#include "ns3/callback.h"
#include "ns3/net-device.h"
#include "ns3/ptr.h"
#include "ns3/socket.h"
#include "ns3/traced-callback.h"

#include <queue>
#include <stdint.h>

namespace ns3 {

class Node;
class Packet;
class NetDevice;
class PacketSocketAddress;

class PacketSocket : public Socket {
public:
  static TypeId GetTypeId();

  PacketSocket();
  ~PacketSocket() override;

  void SetNode(Ptr<Node> node);

  SocketErrno GetErrno() const override;
  SocketType GetSocketType() const override;
  Ptr<Node> GetNode() const override;
  int Bind() override;
  int Bind6() override;
  int Bind(const Address &address) override;
  int Close() override;
  int ShutdownSend() override;
  int ShutdownRecv() override;
  int Connect(const Address &address) override;
  int Listen() override;
  uint32_t GetTxAvailable() const override;
  int Send(Ptr<Packet> p, uint32_t flags) override;
  int SendTo(Ptr<Packet> p, uint32_t flags, const Address &toAddress) override;
  uint32_t GetRxAvailable() const override;
  Ptr<Packet> Recv(uint32_t maxSize, uint32_t flags) override;
  Ptr<Packet> RecvFrom(uint32_t maxSize, uint32_t flags,
                       Address &fromAddress) override;
  int GetSockName(Address &address) const override;
  int GetPeerName(Address &address) const override;
  bool SetAllowBroadcast(bool allowBroadcast) override;
  bool GetAllowBroadcast() const override;

private:
  void ForwardUp(Ptr<NetDevice> device, Ptr<const Packet> packet,
                 uint16_t protocol, const Address &from, const Address &to,
                 NetDevice::PacketType packetType);
  int DoBind(const PacketSocketAddress &address);

  uint32_t GetMinMtu(PacketSocketAddress ad) const;
  void DoDispose() override;

  enum State { STATE_OPEN, STATE_BOUND, STATE_CONNECTED, STATE_CLOSED };

  Ptr<Node> m_node;
  mutable SocketErrno m_errno;
  bool m_shutdownSend;
  bool m_shutdownRecv;
  State m_state;
  uint16_t m_protocol;
  bool m_isSingleDevice;
  uint32_t m_device;
  Address m_destAddr;

  std::queue<std::pair<Ptr<Packet>, Address>> m_deliveryQueue;
  uint32_t m_rxAvailable;

  TracedCallback<Ptr<const Packet>> m_dropTrace;

  uint32_t m_rcvBufSize;
};

class PacketSocketTag : public Tag {
public:
  PacketSocketTag();
  void SetPacketType(NetDevice::PacketType t);
  NetDevice::PacketType GetPacketType() const;
  void SetDestAddress(Address a);
  Address GetDestAddress() const;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(TagBuffer i) const override;
  void Deserialize(TagBuffer i) override;
  void Print(std::ostream &os) const override;

private:
  NetDevice::PacketType m_packetType;
  Address m_destAddr;
};

class DeviceNameTag : public Tag {
public:
  DeviceNameTag();
  void SetDeviceName(std::string n);
  std::string GetDeviceName() const;
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(TagBuffer i) const override;
  void Deserialize(TagBuffer i) override;
  void Print(std::ostream &os) const override;

private:
  std::string m_deviceName;
};

} // namespace ns3

#endif
