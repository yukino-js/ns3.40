
#include "lte-test-rlc-am-transmitter.h"

#include "lte-test-entities.h"

#include "ns3/log.h"
#include "ns3/lte-rlc-am.h"
#include "ns3/lte-rlc-header.h"
#include "ns3/simulator.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LteRlcAmTransmitterTest");

LteRlcAmTransmitterTestSuite::LteRlcAmTransmitterTestSuite()
    : TestSuite("lte-rlc-am-transmitter", SYSTEM) {

  AddTestCase(new LteRlcAmTransmitterOneSduTestCase("One SDU, one PDU"),
              TestCase::QUICK);
  AddTestCase(new LteRlcAmTransmitterSegmentationTestCase("Segmentation"),
              TestCase::QUICK);
  AddTestCase(new LteRlcAmTransmitterConcatenationTestCase("Concatenation"),
              TestCase::QUICK);
  AddTestCase(new LteRlcAmTransmitterReportBufferStatusTestCase(
                  "ReportBufferStatus primitive"),
              TestCase::QUICK);
}

static LteRlcAmTransmitterTestSuite lteRlcAmTransmitterTestSuite;

LteRlcAmTransmitterTestCase::LteRlcAmTransmitterTestCase(std::string name)
    : TestCase(name) {}

LteRlcAmTransmitterTestCase::~LteRlcAmTransmitterTestCase() {}

void LteRlcAmTransmitterTestCase::DoRun() {

  uint16_t rnti = 1111;
  uint8_t lcid = 222;

  Packet::EnablePrinting();

  txPdcp = CreateObject<LteTestPdcp>();

  txRlc = CreateObject<LteRlcAm>();
  txRlc->SetRnti(rnti);
  txRlc->SetLcId(lcid);

  txMac = CreateObject<LteTestMac>();
  txMac->SetRlcHeaderType(LteTestMac::AM_RLC_HEADER);

  txPdcp->SetLteRlcSapProvider(txRlc->GetLteRlcSapProvider());
  txRlc->SetLteRlcSapUser(txPdcp->GetLteRlcSapUser());

  txRlc->SetLteMacSapProvider(txMac->GetLteMacSapProvider());
  txMac->SetLteMacSapUser(txRlc->GetLteMacSapUser());
}

void LteRlcAmTransmitterTestCase::CheckDataReceived(Time time,
                                                    std::string shouldReceived,
                                                    std::string assertMsg) {
  Simulator::Schedule(time, &LteRlcAmTransmitterTestCase::DoCheckDataReceived,
                      this, shouldReceived, assertMsg);
}

void LteRlcAmTransmitterTestCase::DoCheckDataReceived(
    std::string shouldReceived, std::string assertMsg) {
  NS_TEST_ASSERT_MSG_EQ(shouldReceived, txMac->GetDataReceived(), assertMsg);
}

LteRlcAmTransmitterOneSduTestCase::LteRlcAmTransmitterOneSduTestCase(
    std::string name)
    : LteRlcAmTransmitterTestCase(name) {}

LteRlcAmTransmitterOneSduTestCase::~LteRlcAmTransmitterOneSduTestCase() {}

void LteRlcAmTransmitterOneSduTestCase::DoRun() {
  LteRlcAmTransmitterTestCase::DoRun();

  txPdcp->SendData(Seconds(0.100), "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

  txMac->SendTxOpportunity(Seconds(0.150), 30);
  CheckDataReceived(Seconds(0.200), "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
                    "SDU is not OK");

  Simulator::Stop(Seconds(0.3));
  Simulator::Run();
  Simulator::Destroy();
}

LteRlcAmTransmitterSegmentationTestCase::
    LteRlcAmTransmitterSegmentationTestCase(std::string name)
    : LteRlcAmTransmitterTestCase(name) {}

LteRlcAmTransmitterSegmentationTestCase::
    ~LteRlcAmTransmitterSegmentationTestCase() {}

void LteRlcAmTransmitterSegmentationTestCase::DoRun() {
  LteRlcAmTransmitterTestCase::DoRun();

  txPdcp->SendData(Seconds(0.100), "ABCDEFGHIJKLMNOPQRSTUVWXYZZ");

  txMac->SendTxOpportunity(Seconds(0.150), 12);
  CheckDataReceived(Seconds(0.200), "ABCDEFGH", "Segment #1 is not OK");

  txMac->SendTxOpportunity(Seconds(0.250), 12);
  CheckDataReceived(Seconds(0.300), "IJKLMNOP", "Segment #2 is not OK");

  txMac->SendTxOpportunity(Seconds(0.350), 12);
  CheckDataReceived(Seconds(0.400), "QRSTUVWX", "Segment #3 is not OK");

  txMac->SendTxOpportunity(Seconds(0.450), 7);
  CheckDataReceived(Seconds(0.500), "YZZ", "Segment #4 is not OK");

  Simulator::Stop(Seconds(0.6));
  Simulator::Run();
  Simulator::Destroy();
}

LteRlcAmTransmitterConcatenationTestCase::
    LteRlcAmTransmitterConcatenationTestCase(std::string name)
    : LteRlcAmTransmitterTestCase(name) {}

LteRlcAmTransmitterConcatenationTestCase::
    ~LteRlcAmTransmitterConcatenationTestCase() {}

void LteRlcAmTransmitterConcatenationTestCase::DoRun() {
  LteRlcAmTransmitterTestCase::DoRun();

  txPdcp->SendData(Seconds(0.100), "ABCDEFGH");
  txPdcp->SendData(Seconds(0.150), "IJKLMNOPQR");
  txPdcp->SendData(Seconds(0.200), "STUVWXYZ");

  txMac->SendTxOpportunity(Seconds(0.250), 33);
  CheckDataReceived(Seconds(0.300), "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
                    "Concatenation is not OK");

  Simulator::Stop(Seconds(0.4));
  Simulator::Run();
  Simulator::Destroy();
}

LteRlcAmTransmitterReportBufferStatusTestCase::
    LteRlcAmTransmitterReportBufferStatusTestCase(std::string name)
    : LteRlcAmTransmitterTestCase(name) {}

LteRlcAmTransmitterReportBufferStatusTestCase::
    ~LteRlcAmTransmitterReportBufferStatusTestCase() {}

void LteRlcAmTransmitterReportBufferStatusTestCase::DoRun() {
  LteRlcAmTransmitterTestCase::DoRun();

  txPdcp->SendData(Seconds(0.100), "ABCDEFGHIJ");
  txPdcp->SendData(Seconds(0.150), "KLMNOPQRS");
  txPdcp->SendData(Seconds(0.200), "TUVWXYZ");

  txMac->SendTxOpportunity(Seconds(0.250), (4 + 2) + (10 + 6));
  CheckDataReceived(Seconds(0.300), "ABCDEFGHIJKLMNOP", "SDU #1 is not OK");

  txPdcp->SendData(Seconds(0.350), "ABCDEFGH");
  txPdcp->SendData(Seconds(0.400), "IJKLMNOPQRST");
  txPdcp->SendData(Seconds(0.450), "UVWXYZ");

  txMac->SendTxOpportunity(Seconds(0.500), 4 + 3);
  CheckDataReceived(Seconds(0.550), "QRS", "SDU #2 is not OK");

  txPdcp->SendData(Seconds(0.600), "ABCDEFGH");
  txPdcp->SendData(Seconds(0.650), "IJKLMNOPQRST");
  txPdcp->SendData(Seconds(0.700), "UVWXYZ");

  txPdcp->SendData(Seconds(0.750), "ABCDEFGHIJ");
  txPdcp->SendData(Seconds(0.800), "KLMNOPQRST");
  txPdcp->SendData(Seconds(0.850), "UVWXYZ");

  txMac->SendTxOpportunity(Seconds(0.900), 4 + 7);
  CheckDataReceived(Seconds(0.950), "TUVWXYZ", "SDU #3 is not OK");

  txMac->SendTxOpportunity(Seconds(1.000), (4 + 2) + (8 + 2));
  CheckDataReceived(Seconds(1.050), "ABCDEFGHIJ", "SDU #4 is not OK");

  txPdcp->SendData(Seconds(1.100), "ABCDEFGHIJ");
  txPdcp->SendData(Seconds(1.150), "KLMNOPQRSTU");
  txPdcp->SendData(Seconds(1.200), "VWXYZ");

  txMac->SendTxOpportunity(Seconds(1.250), 4 + 3);
  CheckDataReceived(Seconds(1.300), "KLM", "SDU #5 is not OK");

  txMac->SendTxOpportunity(Seconds(1.350), 4 + 3);
  CheckDataReceived(Seconds(1.400), "NOP", "SDU #6 is not OK");

  txMac->SendTxOpportunity(Seconds(1.450), 4 + 4);
  CheckDataReceived(Seconds(1.500), "QRST", "SDU #7 is not OK");

  txMac->SendTxOpportunity(Seconds(1.550), (4 + 2 + 1 + 2 + 1 + 2 + 1) +
                                               (6 + 8 + 12 + 6 + 10 + 10 + 3));
  CheckDataReceived(Seconds(1.600),
                    "UVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVW",
                    "SDU #8 is not OK");

  txMac->SendTxOpportunity(Seconds(1.650), (4 + 2 + 1 + 2) + (3 + 10 + 10 + 7));
  CheckDataReceived(Seconds(1.700), "XYZABCDEFGHIJKLMNOPQRSTUVWXYZ",
                    "SDU #9 is not OK");

  Simulator::Stop(Seconds(2));
  Simulator::Run();
  Simulator::Destroy();
}
