
#include "ns3/internet-stack-helper.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/test.h"

#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("InternetStackHelperTestSuite");

class InternetStackHelperTestCase : public TestCase {
public:
  InternetStackHelperTestCase();

private:
  void DoRun() override;
  void DoTeardown() override;
};

InternetStackHelperTestCase::InternetStackHelperTestCase()
    : TestCase("InternetStackHelperTestCase") {}

void InternetStackHelperTestCase::DoRun() {

  Ptr<Node> nodeIpv4Only = CreateObject<Node>();
  Ptr<Node> nodeIpv6Only = CreateObject<Node>();
  Ptr<Node> nodeIpv46 = CreateObject<Node>();

  InternetStackHelper internet;

  internet.SetIpv4StackInstall(true);
  internet.SetIpv6StackInstall(false);
  internet.Install(nodeIpv4Only);

  internet.SetIpv4StackInstall(false);
  internet.SetIpv6StackInstall(true);
  internet.Install(nodeIpv6Only);

  internet.SetIpv4StackInstall(true);
  internet.SetIpv6StackInstall(true);
  internet.Install(nodeIpv46);

  NS_TEST_EXPECT_MSG_NE(
      nodeIpv4Only->GetObject<Ipv4>(), nullptr,
      "IPv4 not found on IPv4-only node (should have been there)");
  NS_TEST_EXPECT_MSG_EQ(
      nodeIpv4Only->GetObject<Ipv6>(), nullptr,
      "IPv6 found on IPv4-only node (should not have been there)");

  NS_TEST_EXPECT_MSG_EQ(
      nodeIpv6Only->GetObject<Ipv4>(), nullptr,
      "IPv4 found on IPv6-only node (should not have been there)");
  NS_TEST_EXPECT_MSG_NE(
      nodeIpv6Only->GetObject<Ipv6>(), nullptr,
      "IPv6 not found on IPv6-only node (should have been there)");

  NS_TEST_EXPECT_MSG_NE(
      nodeIpv46->GetObject<Ipv4>(), nullptr,
      "IPv4 not found on dual stack node (should have been there)");
  NS_TEST_EXPECT_MSG_NE(
      nodeIpv46->GetObject<Ipv6>(), nullptr,
      "IPv6 not found on dual stack node (should have been there)");

  internet.Install(nodeIpv4Only);
  internet.Install(nodeIpv6Only);
  internet.Install(nodeIpv46);

  NS_TEST_EXPECT_MSG_NE(nodeIpv4Only->GetObject<Ipv4>(), nullptr,
                        "IPv4 not found on IPv4-only, now dual stack node "
                        "(should have been there)");
  NS_TEST_EXPECT_MSG_NE(nodeIpv4Only->GetObject<Ipv6>(), nullptr,
                        "IPv6 not found on IPv4-only, now dual stack node "
                        "(should have been there)");

  NS_TEST_EXPECT_MSG_NE(nodeIpv6Only->GetObject<Ipv4>(), nullptr,
                        "IPv4 not found on IPv6-only, now dual stack node "
                        "(should have been there)");
  NS_TEST_EXPECT_MSG_NE(nodeIpv6Only->GetObject<Ipv6>(), nullptr,
                        "IPv6 not found on IPv6-only, now dual stack node "
                        "(should have been there)");

  NS_TEST_EXPECT_MSG_NE(
      nodeIpv46->GetObject<Ipv4>(), nullptr,
      "IPv4 not found on dual stack node (should have been there)");
  NS_TEST_EXPECT_MSG_NE(
      nodeIpv46->GetObject<Ipv6>(), nullptr,
      "IPv6 not found on dual stack node (should have been there)");
}

void InternetStackHelperTestCase::DoTeardown() { Simulator::Destroy(); }

class InternetStackHelperTestSuite : public TestSuite {
public:
  InternetStackHelperTestSuite() : TestSuite("internet-stack-helper", UNIT) {
    AddTestCase(new InternetStackHelperTestCase(), TestCase::QUICK);
  }
};

static InternetStackHelperTestSuite g_internetStackHelperTestSuite;
