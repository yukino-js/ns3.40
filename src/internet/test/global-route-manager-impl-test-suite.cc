
#include "ns3/candidate-queue.h"
#include "ns3/global-route-manager-impl.h"
#include "ns3/simulator.h"
#include "ns3/test.h"

#include <cstdlib>

using namespace ns3;

class GlobalRouteManagerImplTestCase : public TestCase {
public:
  GlobalRouteManagerImplTestCase();
  void DoRun() override;
};

GlobalRouteManagerImplTestCase::GlobalRouteManagerImplTestCase()
    : TestCase("GlobalRouteManagerImplTestCase") {}

void GlobalRouteManagerImplTestCase::DoRun() {
  CandidateQueue candidate;

  for (int i = 0; i < 100; ++i) {
    auto v = new SPFVertex;
    v->SetDistanceFromRoot(std::rand() % 100);
    candidate.Push(v);
  }

  for (int i = 0; i < 100; ++i) {
    SPFVertex *v = candidate.Pop();
    delete v;
    v = nullptr;
  }

  auto lr0 = new GlobalRoutingLinkRecord(GlobalRoutingLinkRecord::PointToPoint,
                                         "0.0.0.2", "10.1.1.1", 1);

  auto lr1 = new GlobalRoutingLinkRecord(GlobalRoutingLinkRecord::StubNetwork,
                                         "10.1.1.1", "255.255.255.252", 1);

  auto lsa0 = new GlobalRoutingLSA();
  lsa0->SetLSType(GlobalRoutingLSA::RouterLSA);
  lsa0->SetLinkStateId("0.0.0.0");
  lsa0->SetAdvertisingRouter("0.0.0.0");
  lsa0->AddLinkRecord(lr0);
  lsa0->AddLinkRecord(lr1);

  auto lr2 = new GlobalRoutingLinkRecord(GlobalRoutingLinkRecord::PointToPoint,
                                         "0.0.0.2", "10.1.2.1", 1);

  auto lr3 = new GlobalRoutingLinkRecord(GlobalRoutingLinkRecord::StubNetwork,
                                         "10.1.2.1", "255.255.255.252", 1);

  auto lsa1 = new GlobalRoutingLSA();
  lsa1->SetLSType(GlobalRoutingLSA::RouterLSA);
  lsa1->SetLinkStateId("0.0.0.1");
  lsa1->SetAdvertisingRouter("0.0.0.1");
  lsa1->AddLinkRecord(lr2);
  lsa1->AddLinkRecord(lr3);

  auto lr4 = new GlobalRoutingLinkRecord(GlobalRoutingLinkRecord::PointToPoint,
                                         "0.0.0.0", "10.1.1.2", 1);

  auto lr5 = new GlobalRoutingLinkRecord(GlobalRoutingLinkRecord::StubNetwork,
                                         "10.1.1.2", "255.255.255.252", 1);

  auto lr6 = new GlobalRoutingLinkRecord(GlobalRoutingLinkRecord::PointToPoint,
                                         "0.0.0.1", "10.1.2.2", 1);

  auto lr7 = new GlobalRoutingLinkRecord(GlobalRoutingLinkRecord::StubNetwork,
                                         "10.1.2.2", "255.255.255.252", 1);

  auto lr8 = new GlobalRoutingLinkRecord(GlobalRoutingLinkRecord::PointToPoint,
                                         "0.0.0.3", "10.1.3.2", 1);

  auto lr9 = new GlobalRoutingLinkRecord(GlobalRoutingLinkRecord::StubNetwork,
                                         "10.1.3.2", "255.255.255.252", 1);

  auto lsa2 = new GlobalRoutingLSA();
  lsa2->SetLSType(GlobalRoutingLSA::RouterLSA);
  lsa2->SetLinkStateId("0.0.0.2");
  lsa2->SetAdvertisingRouter("0.0.0.2");
  lsa2->AddLinkRecord(lr4);
  lsa2->AddLinkRecord(lr5);
  lsa2->AddLinkRecord(lr6);
  lsa2->AddLinkRecord(lr7);
  lsa2->AddLinkRecord(lr8);
  lsa2->AddLinkRecord(lr9);

  auto lr10 = new GlobalRoutingLinkRecord(GlobalRoutingLinkRecord::PointToPoint,
                                          "0.0.0.2", "10.1.2.1", 1);

  auto lr11 = new GlobalRoutingLinkRecord(GlobalRoutingLinkRecord::StubNetwork,
                                          "10.1.2.1", "255.255.255.252", 1);

  auto lsa3 = new GlobalRoutingLSA();
  lsa3->SetLSType(GlobalRoutingLSA::RouterLSA);
  lsa3->SetLinkStateId("0.0.0.3");
  lsa3->SetAdvertisingRouter("0.0.0.3");
  lsa3->AddLinkRecord(lr10);
  lsa3->AddLinkRecord(lr11);

  auto srmlsdb = new GlobalRouteManagerLSDB();
  srmlsdb->Insert(lsa0->GetLinkStateId(), lsa0);
  srmlsdb->Insert(lsa1->GetLinkStateId(), lsa1);
  srmlsdb->Insert(lsa2->GetLinkStateId(), lsa2);
  srmlsdb->Insert(lsa3->GetLinkStateId(), lsa3);
  NS_TEST_ASSERT_MSG_EQ(lsa2, srmlsdb->GetLSA(lsa2->GetLinkStateId()),
                        "The Ipv4Address is not stored as the link state ID");

  auto srm = new GlobalRouteManagerImpl();
  srm->DebugUseLsdb(srmlsdb);
  srm->DebugSPFCalculate(lsa0->GetLinkStateId());

  Simulator::Run();

  Simulator::Destroy();

  delete srm;
}

class GlobalRouteManagerImplTestSuite : public TestSuite {
public:
  GlobalRouteManagerImplTestSuite();

private:
};

GlobalRouteManagerImplTestSuite::GlobalRouteManagerImplTestSuite()
    : TestSuite("global-route-manager-impl", UNIT) {
  AddTestCase(new GlobalRouteManagerImplTestCase(), TestCase::QUICK);
}

static GlobalRouteManagerImplTestSuite g_globalRoutingManagerImplTestSuite;
