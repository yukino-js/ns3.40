
#ifndef BUG_772_H
#define BUG_772_H

#include "ns3/node-container.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "ns3/test.h"

using namespace ns3;

class Bug772ChainTest : public TestCase {
public:
  Bug772ChainTest(const char *const prefix, const char *const proto, Time time,
                  uint32_t size);
  ~Bug772ChainTest() override;

private:
  NodeContainer *m_nodes;

  const std::string m_prefix;
  const std::string m_proto;
  const Time m_time;
  const uint32_t m_size;
  const double m_step;
  const uint16_t m_port;

  void CreateNodes();
  void CreateDevices();
  void CheckResults();
  void DoRun() override;
  void HandleRead(Ptr<Socket> socket);

  Ptr<Socket> m_recvSocket;
  Ptr<Socket> m_sendSocket;

  uint32_t m_receivedPackets;

  void SendData(Ptr<Socket> socket);
};

#endif
