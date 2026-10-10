#include "ns3/cid.h"
#include "ns3/log.h"
#include "ns3/mac-messages.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "ns3/simulator.h"
#include "ns3/test.h"
#include "ns3/wimax-connection.h"
#include "ns3/wimax-mac-header.h"

using namespace ns3;

class Ns3WimaxFragmentationTestCase : public TestCase {
public:
  Ns3WimaxFragmentationTestCase();
  ~Ns3WimaxFragmentationTestCase() override;

private:
  void DoRun() override;
};

Ns3WimaxFragmentationTestCase::Ns3WimaxFragmentationTestCase()
    : TestCase("Test the packet fragmentation and defragmentation.") {}

Ns3WimaxFragmentationTestCase::~Ns3WimaxFragmentationTestCase() {}

void Ns3WimaxFragmentationTestCase::DoRun() {
  GenericMacHeader gnrcMacHdr;
  ManagementMessageType msgType;
  FragmentationSubheader fragSubhdr;
  GenericMacHeader hdr;

  Cid cid;
  auto connectionTx = new WimaxConnection(cid, Cid::TRANSPORT);
  auto connectionRx = new WimaxConnection(cid, Cid::TRANSPORT);

  Ptr<Packet> packet = Create<Packet>(1000);
  Ptr<Packet> fragment;
  Ptr<Packet> fullPacket = Create<Packet>();

  hdr.SetLen(packet->GetSize() + hdr.GetSerializedSize());
  hdr.SetCid(connectionTx->GetCid());
  MacHeaderType::HeaderType packetType = MacHeaderType::HEADER_TYPE_GENERIC;

  connectionTx->Enqueue(packet, packetType, hdr);

  uint32_t availableByteForFragment = 280;
  for (int i = 0; i < 4; i++) {
    if (connectionTx->GetQueue()->GetFirstPacketRequiredByte(packetType) >
        availableByteForFragment) {
      fragment = connectionTx->Dequeue(packetType, availableByteForFragment);
    } else {
      fragment = connectionTx->Dequeue(packetType);
    }

    fragment->RemoveHeader(gnrcMacHdr);
    uint8_t type = gnrcMacHdr.GetType();
    if (type) {
      NS_TEST_EXPECT_MSG_EQ(((type >> 2) & 1), 1,
                            "The packet is not a fragment");
    }

    fragment->RemoveHeader(fragSubhdr);
    uint32_t fc = fragSubhdr.GetFc();

    NS_TEST_EXPECT_MSG_EQ((fc == 1 && i != 0), false,
                          "The fragment in not the first one");
    NS_TEST_EXPECT_MSG_EQ((fc == 2 && i != 3), false,
                          "The fragment in not the latest one");
    NS_TEST_EXPECT_MSG_EQ(((fc == 3 && i != 1) && (fc == 3 && i != 2)), false,
                          "The fragment in not the middle one");

    if (fc != 2) {
      connectionRx->FragmentEnqueue(fragment);
    } else {
      connectionRx->FragmentEnqueue(fragment);
      WimaxConnection::FragmentsQueue fragmentsQueue =
          connectionRx->GetFragmentsQueue();

      for (auto iter = fragmentsQueue.begin(); iter != fragmentsQueue.end();
           ++iter) {
        fullPacket->AddAtEnd(*iter);
      }
      connectionRx->ClearFragmentsQueue();

      NS_TEST_EXPECT_MSG_EQ(fullPacket->GetSize(), 1000,
                            "The defragmentation is incorrect");
    }
  }
  delete connectionTx;
  delete connectionRx;
  Simulator::Destroy();
}

class Ns3WimaxFragmentationTestSuite : public TestSuite {
public:
  Ns3WimaxFragmentationTestSuite();
};

Ns3WimaxFragmentationTestSuite::Ns3WimaxFragmentationTestSuite()
    : TestSuite("wimax-fragmentation", UNIT) {
  AddTestCase(new Ns3WimaxFragmentationTestCase, TestCase::QUICK);
}

static Ns3WimaxFragmentationTestSuite ns3WimaxFragmentationTestSuite;
