
#ifndef AODV_REGRESSION_H
#define AODV_REGRESSION_H

#include "ns3/node-container.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "ns3/test.h"

using namespace ns3;

class ChainRegressionTest : public TestCase {
public:
  ChainRegressionTest(const char *const prefix, Time time = Seconds(10),
                      uint32_t size = 5, Time arpAliveTimeout = Seconds(120));
  ~ChainRegressionTest() override;

private:
  NodeContainer *m_nodes;

  const std::string m_prefix;
  const Time m_time;
  const uint32_t m_size;
  const double m_step;
  const Time m_arpAliveTimeout;
  Ptr<Socket> m_socket;
  uint16_t m_seq;

  void CreateNodes();
  void CreateDevices();
  void CheckResults();
  void DoRun() override;
  void SendPing();
};

#endif
