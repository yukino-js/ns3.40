
#include "ns3/ipv4-interface-container.h"
#include "ns3/node-container.h"
#include "ns3/nstime.h"
#include "ns3/pcap-file.h"
#include "ns3/test.h"

using namespace ns3;

class HwmpDoRfRegressionTest : public TestCase {
public:
  HwmpDoRfRegressionTest();
  ~HwmpDoRfRegressionTest() override;

  void DoRun() override;
  void CheckResults();

private:
  NodeContainer *m_nodes;
  Time m_time;
  Ipv4InterfaceContainer m_interfaces;

  void CreateNodes();
  void CreateDevices();
  void InstallApplications();
  void ResetPosition();

  Ptr<Socket> m_serverSocketA;
  Ptr<Socket> m_serverSocketB;
  Ptr<Socket> m_clientSocketA;
  Ptr<Socket> m_clientSocketB;
  Ptr<Socket> m_clientSocketC;

  uint32_t m_sentPktsCounterA;
  uint32_t m_sentPktsCounterB;
  uint32_t m_sentPktsCounterC;

  void SendDataA(Ptr<Socket> socket);

  void SendDataB(Ptr<Socket> socket);

  void SendDataC(Ptr<Socket> socket);

  void HandleReadServer(Ptr<Socket> socket);

  void HandleReadClient(Ptr<Socket> socket);
};
