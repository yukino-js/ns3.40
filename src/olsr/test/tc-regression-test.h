
#ifndef TC_REGRESSION_TEST_H
#define TC_REGRESSION_TEST_H

#include "ns3/ipv4-raw-socket-impl.h"
#include "ns3/node-container.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "ns3/test.h"

namespace ns3 {
namespace olsr {
class TcRegressionTest : public TestCase {
public:
  TcRegressionTest();
  ~TcRegressionTest() override;

private:
  const Time m_time;
  void CreateNodes();
  void DoRun() override;

  void ReceivePktProbeA(Ptr<Socket> socket);
  uint8_t m_countA;
  Ptr<Ipv4RawSocketImpl> m_rxSocketA;

  void ReceivePktProbeB(Ptr<Socket> socket);
  uint8_t m_countB;
  Ptr<Ipv4RawSocketImpl> m_rxSocketB;

  void ReceivePktProbeC(Ptr<Socket> socket);
  uint8_t m_countC;
  Ptr<Ipv4RawSocketImpl> m_rxSocketC;
};

} // namespace olsr
} // namespace ns3

#endif
