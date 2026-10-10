
#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include "ns3/callback.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/socket.h"
#include "ns3/traced-callback.h"

namespace ns3 {

class Node;
class Packet;

class TcpSocket : public Socket {
public:
  static TypeId GetTypeId();

  TcpSocket();
  ~TcpSocket() override;

  enum TcpStates_t {
    CLOSED = 0,
    LISTEN,
    SYN_SENT,
    SYN_RCVD,
    ESTABLISHED,
    CLOSE_WAIT,
    LAST_ACK,
    FIN_WAIT_1,
    FIN_WAIT_2,
    CLOSING,
    TIME_WAIT,
    LAST_STATE
  };

  static const char *const TcpStateName[TcpSocket::LAST_STATE];

private:
  virtual void SetSndBufSize(uint32_t size) = 0;

  virtual uint32_t GetSndBufSize() const = 0;

  virtual void SetRcvBufSize(uint32_t size) = 0;

  virtual uint32_t GetRcvBufSize() const = 0;

  virtual void SetSegSize(uint32_t size) = 0;

  virtual uint32_t GetSegSize() const = 0;

  virtual void SetInitialSSThresh(uint32_t threshold) = 0;

  virtual uint32_t GetInitialSSThresh() const = 0;

  virtual void SetInitialCwnd(uint32_t cwnd) = 0;

  virtual uint32_t GetInitialCwnd() const = 0;

  virtual void SetConnTimeout(Time timeout) = 0;

  virtual Time GetConnTimeout() const = 0;

  virtual void SetSynRetries(uint32_t count) = 0;

  virtual uint32_t GetSynRetries() const = 0;

  virtual void SetDataRetries(uint32_t retries) = 0;

  virtual uint32_t GetDataRetries() const = 0;

  virtual void SetDelAckTimeout(Time timeout) = 0;

  virtual Time GetDelAckTimeout() const = 0;

  virtual void SetDelAckMaxCount(uint32_t count) = 0;

  virtual uint32_t GetDelAckMaxCount() const = 0;

  virtual void SetTcpNoDelay(bool noDelay) = 0;

  virtual bool GetTcpNoDelay() const = 0;

  virtual void SetPersistTimeout(Time timeout) = 0;

  virtual Time GetPersistTimeout() const = 0;
};

typedef void (*TcpStatesTracedValueCallback)(
    const TcpSocket::TcpStates_t oldValue,
    const TcpSocket::TcpStates_t newValue);

} // namespace ns3

#endif
