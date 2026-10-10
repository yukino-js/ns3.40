
#ifndef NS3_SOCKET_H
#define NS3_SOCKET_H

#include "address.h"
#include "net-device.h"
#include "tag.h"

#include "ns3/callback.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/object.h"
#include "ns3/ptr.h"

#include <stdint.h>

namespace ns3 {

class Node;
class Packet;

class Socket : public Object {
public:
  static TypeId GetTypeId();

  Socket();
  ~Socket() override;

  enum SocketErrno {
    ERROR_NOTERROR,
    ERROR_ISCONN,
    ERROR_NOTCONN,
    ERROR_MSGSIZE,
    ERROR_AGAIN,
    ERROR_SHUTDOWN,
    ERROR_OPNOTSUPP,
    ERROR_AFNOSUPPORT,
    ERROR_INVAL,
    ERROR_BADF,
    ERROR_NOROUTETOHOST,
    ERROR_NODEV,
    ERROR_ADDRNOTAVAIL,
    ERROR_ADDRINUSE,
    SOCKET_ERRNO_LAST
  };

  enum SocketType {
    NS3_SOCK_STREAM,
    NS3_SOCK_SEQPACKET,
    NS3_SOCK_DGRAM,
    NS3_SOCK_RAW
  };

  enum SocketPriority {
    NS3_PRIO_BESTEFFORT = 0,
    NS3_PRIO_FILLER = 1,
    NS3_PRIO_BULK = 2,
    NS3_PRIO_INTERACTIVE_BULK = 4,
    NS3_PRIO_INTERACTIVE = 6,
    NS3_PRIO_CONTROL = 7
  };

  enum Ipv6MulticastFilterMode { INCLUDE = 1, EXCLUDE };

  static Ptr<Socket> CreateSocket(Ptr<Node> node, TypeId tid);
  virtual Socket::SocketErrno GetErrno() const = 0;
  virtual Socket::SocketType GetSocketType() const = 0;
  virtual Ptr<Node> GetNode() const = 0;
  void SetConnectCallback(Callback<void, Ptr<Socket>> connectionSucceeded,
                          Callback<void, Ptr<Socket>> connectionFailed);
  void SetCloseCallbacks(Callback<void, Ptr<Socket>> normalClose,
                         Callback<void, Ptr<Socket>> errorClose);
  void SetAcceptCallback(
      Callback<bool, Ptr<Socket>, const Address &> connectionRequest,
      Callback<void, Ptr<Socket>, const Address &> newConnectionCreated);
  void SetDataSentCallback(Callback<void, Ptr<Socket>, uint32_t> dataSent);
  void SetSendCallback(Callback<void, Ptr<Socket>, uint32_t> sendCb);
  void SetRecvCallback(Callback<void, Ptr<Socket>> receivedData);
  virtual int Bind(const Address &address) = 0;

  virtual int Bind() = 0;

  virtual int Bind6() = 0;

  virtual int Close() = 0;

  virtual int ShutdownSend() = 0;

  virtual int ShutdownRecv() = 0;

  virtual int Connect(const Address &address) = 0;

  virtual int Listen() = 0;

  virtual uint32_t GetTxAvailable() const = 0;

  virtual int Send(Ptr<Packet> p, uint32_t flags) = 0;

  virtual int SendTo(Ptr<Packet> p, uint32_t flags,
                     const Address &toAddress) = 0;

  virtual uint32_t GetRxAvailable() const = 0;

  virtual Ptr<Packet> Recv(uint32_t maxSize, uint32_t flags) = 0;

  virtual Ptr<Packet> RecvFrom(uint32_t maxSize, uint32_t flags,
                               Address &fromAddress) = 0;

  int Send(Ptr<Packet> p);

  int Send(const uint8_t *buf, uint32_t size, uint32_t flags);

  int SendTo(const uint8_t *buf, uint32_t size, uint32_t flags,
             const Address &address);

  Ptr<Packet> Recv();

  int Recv(uint8_t *buf, uint32_t size, uint32_t flags);

  Ptr<Packet> RecvFrom(Address &fromAddress);

  int RecvFrom(uint8_t *buf, uint32_t size, uint32_t flags,
               Address &fromAddress);
  virtual int GetSockName(Address &address) const = 0;

  virtual int GetPeerName(Address &address) const = 0;

  virtual void BindToNetDevice(Ptr<NetDevice> netdevice);

  Ptr<NetDevice> GetBoundNetDevice();

  virtual bool SetAllowBroadcast(bool allowBroadcast) = 0;

  virtual bool GetAllowBroadcast() const = 0;

  void SetRecvPktInfo(bool flag);

  bool IsRecvPktInfo() const;

  void SetPriority(uint8_t priority);

  uint8_t GetPriority() const;

  static uint8_t IpTos2Priority(uint8_t ipTos);

  void SetIpTos(uint8_t ipTos);

  uint8_t GetIpTos() const;

  void SetIpRecvTos(bool ipv4RecvTos);

  bool IsIpRecvTos() const;

  void SetIpv6Tclass(int ipTclass);

  uint8_t GetIpv6Tclass() const;

  void SetIpv6RecvTclass(bool ipv6RecvTclass);

  bool IsIpv6RecvTclass() const;

  virtual void SetIpTtl(uint8_t ipTtl);

  virtual uint8_t GetIpTtl() const;

  void SetIpRecvTtl(bool ipv4RecvTtl);

  bool IsIpRecvTtl() const;

  virtual void SetIpv6HopLimit(uint8_t ipHopLimit);

  virtual uint8_t GetIpv6HopLimit() const;

  void SetIpv6RecvHopLimit(bool ipv6RecvHopLimit);

  bool IsIpv6RecvHopLimit() const;

  virtual void Ipv6JoinGroup(Ipv6Address address,
                             Ipv6MulticastFilterMode filterMode,
                             std::vector<Ipv6Address> sourceAddresses);

  virtual void Ipv6JoinGroup(Ipv6Address address);

  virtual void Ipv6LeaveGroup();

protected:
  void NotifyConnectionSucceeded();

  void NotifyConnectionFailed();

  void NotifyNormalClose();

  void NotifyErrorClose();

  bool NotifyConnectionRequest(const Address &from);

  void NotifyNewConnectionCreated(Ptr<Socket> socket, const Address &from);

  void NotifyDataSent(uint32_t size);

  void NotifySend(uint32_t spaceAvailable);

  void NotifyDataRecv();

  void DoDispose() override;

  bool IsManualIpv6Tclass() const;

  bool IsManualIpTtl() const;

  bool IsManualIpv6HopLimit() const;

  Ptr<NetDevice> m_boundnetdevice;
  bool m_recvPktInfo;
  Ipv6Address m_ipv6MulticastGroupAddress;

private:
  Callback<void, Ptr<Socket>> m_connectionSucceeded;
  Callback<void, Ptr<Socket>> m_connectionFailed;
  Callback<void, Ptr<Socket>> m_normalClose;
  Callback<void, Ptr<Socket>> m_errorClose;
  Callback<bool, Ptr<Socket>, const Address &> m_connectionRequest;
  Callback<void, Ptr<Socket>, const Address &> m_newConnectionCreated;
  Callback<void, Ptr<Socket>, uint32_t> m_dataSent;
  Callback<void, Ptr<Socket>, uint32_t> m_sendCb;
  Callback<void, Ptr<Socket>> m_receivedData;

  uint8_t m_priority;

  bool m_manualIpTtl;
  bool m_ipRecvTos;
  bool m_ipRecvTtl;

  uint8_t m_ipTos;
  uint8_t m_ipTtl;

  bool m_manualIpv6Tclass;
  bool m_manualIpv6HopLimit;
  bool m_ipv6RecvTclass;
  bool m_ipv6RecvHopLimit;

  uint8_t m_ipv6Tclass;
  uint8_t m_ipv6HopLimit;
};

class SocketIpTtlTag : public Tag {
public:
  SocketIpTtlTag();

  void SetTtl(uint8_t ttl);

  uint8_t GetTtl() const;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(TagBuffer i) const override;

  void Deserialize(TagBuffer i) override;

  void Print(std::ostream &os) const override;

private:
  uint8_t m_ttl;
};

class SocketIpv6HopLimitTag : public Tag {
public:
  SocketIpv6HopLimitTag();

  void SetHopLimit(uint8_t hopLimit);

  uint8_t GetHopLimit() const;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(TagBuffer i) const override;

  void Deserialize(TagBuffer i) override;

  void Print(std::ostream &os) const override;

private:
  uint8_t m_hopLimit;
};

class SocketSetDontFragmentTag : public Tag {
public:
  SocketSetDontFragmentTag();

  void Enable();

  void Disable();

  bool IsEnabled() const;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(TagBuffer i) const override;

  void Deserialize(TagBuffer i) override;

  void Print(std::ostream &os) const override;

private:
  bool m_dontFragment;
};

class SocketIpTosTag : public Tag {
public:
  SocketIpTosTag();

  void SetTos(uint8_t tos);

  uint8_t GetTos() const;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(TagBuffer i) const override;

  void Deserialize(TagBuffer i) override;

  void Print(std::ostream &os) const override;

private:
  uint8_t m_ipTos;
};

class SocketPriorityTag : public Tag {
public:
  SocketPriorityTag();

  void SetPriority(uint8_t priority);

  uint8_t GetPriority() const;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(TagBuffer i) const override;

  void Deserialize(TagBuffer i) override;

  void Print(std::ostream &os) const override;

private:
  uint8_t m_priority;
};

class SocketIpv6TclassTag : public Tag {
public:
  SocketIpv6TclassTag();

  void SetTclass(uint8_t tclass);

  uint8_t GetTclass() const;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(TagBuffer i) const override;

  void Deserialize(TagBuffer i) override;

  void Print(std::ostream &os) const override;

private:
  uint8_t m_ipv6Tclass;
};

} // namespace ns3

#endif
