
#include "ns3/ipv4-interface-container.h"
#include "ns3/node-container.h"
#include "ns3/nstime.h"
#include "ns3/pcap-file.h"
#include "ns3/test.h"

using namespace ns3;

class FlameRegressionTest : public TestCase {
public:
  FlameRegressionTest();
  ~FlameRegressionTest() override;

  void DoRun() override;
  void CheckResults();

private:
  NodeContainer *m_nodes;
  Time m_time;
  Ipv4InterfaceContainer m_interfaces;

  void CreateNodes();
  void CreateDevices();
  void InstallApplications();

  Ptr<Socket> m_serverSocket;
  Ptr<Socket> m_clientSocket;

  uint32_t m_sentPktsCounter;

  void SendData(Ptr<Socket> socket);

  void HandleReadServer(Ptr<Socket> socket);

  void HandleReadClient(Ptr<Socket> socket);
};
