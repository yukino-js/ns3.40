

#ifndef BUG780_TEST_H
#define BUG780_TEST_H

#include "ns3/node-container.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/test.h"

namespace ns3 {

class Socket;

namespace olsr {

class Bug780Test : public TestCase {
public:
  Bug780Test();
  ~Bug780Test() override;

private:
  const Time m_time;
  void CreateNodes();
  void DoRun() override;
  void SendPing();
  void Receive(Ptr<Socket> socket);
  Ptr<Socket> m_socket;
  uint16_t m_seq;
  uint16_t m_recvCount;
};

} // namespace olsr
} // namespace ns3

#endif
