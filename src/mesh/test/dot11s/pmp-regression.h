
#ifndef PMP_REGRESSION_H
#define PMP_REGRESSION_H
#include "ns3/node-container.h"
#include "ns3/nstime.h"
#include "ns3/test.h"

using namespace ns3;

class PeerManagementProtocolRegressionTest : public TestCase {
public:
  PeerManagementProtocolRegressionTest();
  ~PeerManagementProtocolRegressionTest() override;

private:
  NodeContainer *m_nodes;
  Time m_time;

  void CreateNodes();
  void CreateDevices();
  void CheckResults();
  void DoRun() override;
};
#endif
