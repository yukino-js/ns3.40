
#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include "ns3/callback.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/socket.h"
#include "ns3/traced-callback.h"

namespace ns3 {

class Node;
class Packet;

class UdpSocket : public Socket {
public:
  static TypeId GetTypeId();

  UdpSocket();
  ~UdpSocket() override;

  virtual int MulticastJoinGroup(uint32_t interface,
                                 const Address &groupAddress) = 0;

  virtual int MulticastLeaveGroup(uint32_t interface,
                                  const Address &groupAddress) = 0;

private:
  virtual void SetRcvBufSize(uint32_t size) = 0;
  virtual uint32_t GetRcvBufSize() const = 0;
  virtual void SetIpMulticastTtl(uint8_t ipTtl) = 0;
  virtual uint8_t GetIpMulticastTtl() const = 0;
  virtual void SetIpMulticastIf(int32_t ipIf) = 0;
  virtual int32_t GetIpMulticastIf() const = 0;
  virtual void SetIpMulticastLoop(bool loop) = 0;
  virtual bool GetIpMulticastLoop() const = 0;
  virtual void SetMtuDiscover(bool discover) = 0;
  virtual bool GetMtuDiscover() const = 0;
};

} // namespace ns3

#endif
