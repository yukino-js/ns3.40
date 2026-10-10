
#include "ns3/fcfs-wifi-queue-scheduler.h"
#include "ns3/simulator.h"
#include "ns3/test.h"
#include "ns3/wifi-mac-queue.h"

#include <algorithm>

using namespace ns3;

class WifiMacQueueDropOldestTest : public TestCase {
public:
  WifiMacQueueDropOldestTest();

  void DoRun() override;
};

WifiMacQueueDropOldestTest::WifiMacQueueDropOldestTest()
    : TestCase("Test DROP_OLDEST setting") {}

void WifiMacQueueDropOldestTest::DoRun() {
  auto wifiMacQueue = CreateObject<WifiMacQueue>(AC_BE);
  wifiMacQueue->SetMaxSize(QueueSize("5p"));
  auto wifiMacScheduler = CreateObject<FcfsWifiQueueScheduler>();
  wifiMacScheduler->SetAttribute(
      "DropPolicy", EnumValue(FcfsWifiQueueScheduler::DROP_OLDEST));
  wifiMacScheduler->m_perAcInfo[AC_BE].wifiMacQueue = wifiMacQueue;
  wifiMacQueue->SetScheduler(wifiMacScheduler);

  Mac48Address addr1 = Mac48Address::Allocate();

  std::list<uint64_t> packetUids;
  for (uint32_t i = 0; i < 5; i++) {
    WifiMacHeader header;
    header.SetType(WIFI_MAC_QOSDATA);
    header.SetAddr1(addr1);
    header.SetQosTid(0);
    auto packet = Create<Packet>();
    auto item = Create<WifiMpdu>(packet, header);
    wifiMacQueue->Enqueue(item);

    packetUids.push_back(packet->GetUid());
  }

  auto mpdu = wifiMacQueue->PeekByTidAndAddress(0, addr1);
  NS_TEST_EXPECT_MSG_EQ(wifiMacQueue->GetNPackets(), 5,
                        "Queue has unexpected number of elements");
  for (auto packetUid : packetUids) {
    NS_TEST_EXPECT_MSG_EQ(mpdu->GetPacket()->GetUid(), packetUid,
                          "Stored packet is not the expected one");
    mpdu = wifiMacQueue->PeekByTidAndAddress(0, addr1, mpdu);
  }

  WifiMacHeader header;
  header.SetType(WIFI_MAC_QOSDATA);
  header.SetAddr1(addr1);
  header.SetQosTid(0);
  auto packet = Create<Packet>();
  auto item = Create<WifiMpdu>(packet, header);
  wifiMacQueue->Enqueue(item);

  packetUids.pop_front();
  packetUids.push_back(packet->GetUid());

  mpdu = wifiMacQueue->PeekByTidAndAddress(0, addr1);
  NS_TEST_EXPECT_MSG_EQ(wifiMacQueue->GetNPackets(), 5,
                        "Queue has unexpected number of elements");
  for (auto packetUid : packetUids) {
    NS_TEST_EXPECT_MSG_EQ(mpdu->GetPacket()->GetUid(), packetUid,
                          "Stored packet is not the expected one");
    mpdu = wifiMacQueue->PeekByTidAndAddress(0, addr1, mpdu);
  }

  wifiMacScheduler->Dispose();
  Simulator::Destroy();
}

class WifiExtractExpiredMpdusTest : public TestCase {
public:
  WifiExtractExpiredMpdusTest();

private:
  void DoRun() override;

  void Enqueue(Mac48Address rxAddr, bool inflight, Time expiryTime);

  WifiMacQueueContainer m_container;
  uint16_t m_currentSeqNo{0};
  Mac48Address m_txAddr;
};

WifiExtractExpiredMpdusTest::WifiExtractExpiredMpdusTest()
    : TestCase("Test extraction of expired MPDUs from MAC queue container") {}

void WifiExtractExpiredMpdusTest::Enqueue(Mac48Address rxAddr, bool inflight,
                                          Time expiryTime) {
  WifiMacHeader header(WIFI_MAC_QOSDATA);
  header.SetAddr1(rxAddr);
  header.SetAddr2(m_txAddr);
  header.SetQosTid(0);
  header.SetSequenceNumber(m_currentSeqNo++);
  auto mpdu = Create<WifiMpdu>(Create<Packet>(), header);

  auto queueId = WifiMacQueueContainer::GetQueueId(mpdu);
  auto elemIt = m_container.insert(m_container.GetQueue(queueId).cend(), mpdu);
  elemIt->expiryTime = expiryTime;
  if (inflight) {
    elemIt->inflights.emplace(0, mpdu);
  }
  elemIt->deleter = [](auto mpdu) {};
}

void WifiExtractExpiredMpdusTest::DoRun() {
  m_txAddr = Mac48Address::Allocate();
  auto rxAddr1 = Mac48Address::Allocate();
  auto rxAddr2 = Mac48Address::Allocate();

  Enqueue(rxAddr1, true, MilliSeconds(10));
  Enqueue(rxAddr1, false, MilliSeconds(10));
  Enqueue(rxAddr1, true, MilliSeconds(12));
  Enqueue(rxAddr1, false, MilliSeconds(15));
  Enqueue(rxAddr1, true, MilliSeconds(30));
  Enqueue(rxAddr1, false, MilliSeconds(30));
  Enqueue(rxAddr1, true, MilliSeconds(35));
  Enqueue(rxAddr1, false, MilliSeconds(35));
  Enqueue(rxAddr1, false, MilliSeconds(40));
  Enqueue(rxAddr1, false, MilliSeconds(75));
  Enqueue(rxAddr1, false, MilliSeconds(75));

  Enqueue(rxAddr2, false, MilliSeconds(11));
  Enqueue(rxAddr2, true, MilliSeconds(11));
  Enqueue(rxAddr2, true, MilliSeconds(13));
  Enqueue(rxAddr2, false, MilliSeconds(30));
  Enqueue(rxAddr2, true, MilliSeconds(35));
  Enqueue(rxAddr2, true, MilliSeconds(40));
  Enqueue(rxAddr2, false, MilliSeconds(40));
  Enqueue(rxAddr2, false, MilliSeconds(70));
  Enqueue(rxAddr2, false, MilliSeconds(75));

  WifiContainerQueueId queueId1{WIFI_QOSDATA_QUEUE, WIFI_UNICAST, rxAddr1, 0};
  WifiContainerQueueId queueId2{WIFI_QOSDATA_QUEUE, WIFI_UNICAST, rxAddr2, 0};

  Simulator::Schedule(MilliSeconds(25), [&]() {
    auto [first1, last1] = m_container.ExtractExpiredMpdus(queueId1);
    NS_TEST_EXPECT_MSG_EQ((first1 != last1), true,
                          "Expected one MPDU extracted");
    NS_TEST_EXPECT_MSG_EQ(first1->mpdu->GetHeader().GetSequenceNumber(), 1,
                          "Unexpected extracted MPDU");
    first1++;
    NS_TEST_EXPECT_MSG_EQ((first1 != last1), true,
                          "Expected two MPDUs extracted");
    NS_TEST_EXPECT_MSG_EQ(first1->mpdu->GetHeader().GetSequenceNumber(), 3,
                          "Unexpected extracted MPDU");
    first1++;
    NS_TEST_EXPECT_MSG_EQ((first1 == last1), true,
                          "Did not expect other expired MPDUs");

    {
      auto [first, last] = m_container.ExtractExpiredMpdus(queueId1);
      NS_TEST_EXPECT_MSG_EQ((first == last), true,
                            "Did not expect other expired MPDUs");
    }

    auto [first2, last2] = m_container.ExtractExpiredMpdus(queueId2);
    NS_TEST_EXPECT_MSG_EQ((first2 != last2), true,
                          "Expected one MPDU extracted");
    NS_TEST_EXPECT_MSG_EQ(first2->mpdu->GetHeader().GetSequenceNumber(), 11,
                          "Unexpected extracted MPDU");
    first2++;
    NS_TEST_EXPECT_MSG_EQ((first2 == last2), true,
                          "Did not expect other expired MPDUs");

    {
      auto [first, last] = m_container.ExtractExpiredMpdus(queueId2);
      NS_TEST_EXPECT_MSG_EQ((first == last), true,
                            "Did not expect other expired MPDUs");
    }
  });

  Simulator::Schedule(MilliSeconds(50), [&]() {
    auto [first, last] = m_container.ExtractAllExpiredMpdus();

    std::set<uint16_t> expectedSeqNo{5, 7, 8, 14, 17};
    std::set<uint16_t> actualSeqNo;

    std::transform(
        first, last, std::inserter(actualSeqNo, actualSeqNo.end()),
        [](auto &elem) { return elem.mpdu->GetHeader().GetSequenceNumber(); });

    NS_TEST_EXPECT_MSG_EQ(expectedSeqNo.size(), actualSeqNo.size(),
                          "Unexpected number of MPDUs extracted");

    for (auto expectedIt = expectedSeqNo.begin(),
              actualIt = actualSeqNo.begin();
         expectedIt != expectedSeqNo.end(); ++expectedIt, ++actualIt) {
      NS_TEST_EXPECT_MSG_EQ(*expectedIt, *actualIt,
                            "Unexpected extracted MPDU");
    }

    {
      auto [first, last] = m_container.ExtractAllExpiredMpdus();
      NS_TEST_EXPECT_MSG_EQ((first == last), true,
                            "Did not expect other expired MPDUs");
    }

    auto elemIt = m_container.GetQueue(queueId1).begin();
    auto endIt = m_container.GetQueue(queueId1).end();
    NS_TEST_EXPECT_MSG_EQ((elemIt != endIt), true,
                          "There should be other MPDU(s) in container queue 1");
    NS_TEST_EXPECT_MSG_EQ(elemIt->mpdu->GetHeader().GetSequenceNumber(), 0,
                          "Unexpected queued MPDU");
    elemIt++;
    NS_TEST_EXPECT_MSG_EQ((elemIt != endIt), true,
                          "There should be other MPDU(s) in container queue 1");
    NS_TEST_EXPECT_MSG_EQ(elemIt->mpdu->GetHeader().GetSequenceNumber(), 2,
                          "Unexpected queued MPDU");
    elemIt++;
    NS_TEST_EXPECT_MSG_EQ((elemIt != endIt), true,
                          "There should be other MPDU(s) in container queue 1");
    NS_TEST_EXPECT_MSG_EQ(elemIt->mpdu->GetHeader().GetSequenceNumber(), 4,
                          "Unexpected queued MPDU");
    elemIt++;
    NS_TEST_EXPECT_MSG_EQ((elemIt != endIt), true,
                          "There should be other MPDU(s) in container queue 1");
    NS_TEST_EXPECT_MSG_EQ(elemIt->mpdu->GetHeader().GetSequenceNumber(), 6,
                          "Unexpected queued MPDU");
    elemIt++;
    NS_TEST_EXPECT_MSG_EQ((elemIt != endIt), true,
                          "There should be other MPDU(s) in container queue 1");
    NS_TEST_EXPECT_MSG_EQ(elemIt->mpdu->GetHeader().GetSequenceNumber(), 9,
                          "Unexpected queued MPDU");
    elemIt++;
    NS_TEST_EXPECT_MSG_EQ((elemIt != endIt), true,
                          "There should be other MPDU(s) in container queue 1");
    NS_TEST_EXPECT_MSG_EQ(elemIt->mpdu->GetHeader().GetSequenceNumber(), 10,
                          "Unexpected queued MPDU");
    elemIt++;
    NS_TEST_EXPECT_MSG_EQ((elemIt == endIt), true,
                          "There should be no other MPDU in container queue 1");

    elemIt = m_container.GetQueue(queueId2).begin();
    endIt = m_container.GetQueue(queueId2).end();
    NS_TEST_EXPECT_MSG_EQ((elemIt != endIt), true,
                          "There should be other MPDU(s) in container queue 2");
    NS_TEST_EXPECT_MSG_EQ(elemIt->mpdu->GetHeader().GetSequenceNumber(), 12,
                          "Unexpected queued MPDU");
    elemIt++;
    NS_TEST_EXPECT_MSG_EQ((elemIt != endIt), true,
                          "There should be other MPDU(s) in container queue 2");
    NS_TEST_EXPECT_MSG_EQ(elemIt->mpdu->GetHeader().GetSequenceNumber(), 13,
                          "Unexpected queued MPDU");
    elemIt++;
    NS_TEST_EXPECT_MSG_EQ((elemIt != endIt), true,
                          "There should be other MPDU(s) in container queue 2");
    NS_TEST_EXPECT_MSG_EQ(elemIt->mpdu->GetHeader().GetSequenceNumber(), 15,
                          "Unexpected queued MPDU");
    elemIt++;
    NS_TEST_EXPECT_MSG_EQ((elemIt != endIt), true,
                          "There should be other MPDU(s) in container queue 2");
    NS_TEST_EXPECT_MSG_EQ(elemIt->mpdu->GetHeader().GetSequenceNumber(), 16,
                          "Unexpected queued MPDU");
    elemIt++;
    NS_TEST_EXPECT_MSG_EQ((elemIt != endIt), true,
                          "There should be other MPDU(s) in container queue 2");
    NS_TEST_EXPECT_MSG_EQ(elemIt->mpdu->GetHeader().GetSequenceNumber(), 18,
                          "Unexpected queued MPDU");
    elemIt++;
    NS_TEST_EXPECT_MSG_EQ((elemIt != endIt), true,
                          "There should be other MPDU(s) in container queue 2");
    NS_TEST_EXPECT_MSG_EQ(elemIt->mpdu->GetHeader().GetSequenceNumber(), 19,
                          "Unexpected queued MPDU");
    elemIt++;
    NS_TEST_EXPECT_MSG_EQ((elemIt == endIt), true,
                          "There should be no other MPDU in container queue 2");
  });

  Simulator::Run();
  Simulator::Destroy();
}

class WifiMacQueueTestSuite : public TestSuite {
public:
  WifiMacQueueTestSuite();
};

WifiMacQueueTestSuite::WifiMacQueueTestSuite()
    : TestSuite("wifi-mac-queue", UNIT) {
  AddTestCase(new WifiMacQueueDropOldestTest, TestCase::QUICK);
  AddTestCase(new WifiExtractExpiredMpdusTest, TestCase::QUICK);
}

static WifiMacQueueTestSuite g_wifiMacQueueTestSuite;
