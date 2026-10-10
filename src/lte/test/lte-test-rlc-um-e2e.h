
#ifndef LTE_TEST_RLC_UM_E2E_H
#define LTE_TEST_RLC_UM_E2E_H

#include "ns3/ptr.h"
#include "ns3/test.h"

namespace ns3 {
class Packet;
}

using namespace ns3;

class LteRlcUmE2eTestSuite : public TestSuite {
public:
  LteRlcUmE2eTestSuite();
};

class LteRlcUmE2eTestCase : public TestCase {
public:
  LteRlcUmE2eTestCase(std::string name, uint32_t seed, double losses);
  LteRlcUmE2eTestCase();
  ~LteRlcUmE2eTestCase() override;

private:
  void DoRun() override;

  void DlDropEvent(Ptr<const Packet> p);
  void UlDropEvent(Ptr<const Packet> p);

  uint32_t m_dlDrops;
  uint32_t m_ulDrops;

  uint32_t m_seed;
  double m_losses;
};

#endif
