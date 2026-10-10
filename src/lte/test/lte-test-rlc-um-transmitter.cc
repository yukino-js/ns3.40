
#include "lte-test-rlc-um-transmitter.h"

#include "lte-test-entities.h"

#include "ns3/log.h"
#include "ns3/lte-rlc-header.h"
#include "ns3/lte-rlc-um.h"
#include "ns3/simulator.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LteRlcUmTransmitterTest");

LteRlcUmTransmitterTestSuite::LteRlcUmTransmitterTestSuite()
    : TestSuite("lte-rlc-um-transmitter", SYSTEM) {

  AddTestCase(new LteRlcUmTransmitterOneSduTestCase("One SDU, one PDU"),
              TestCase::QUICK);
  AddTestCase(new LteRlcUmTransmitterSegmentationTestCase("Segmentation"),
              TestCase::QUICK);
  AddTestCase(new LteRlcUmTransmitterConcatenationTestCase("Concatenation"),
              TestCase::QUICK);
  AddTestCase(new LteRlcUmTransmitterReportBufferStatusTestCase(
                  "ReportBufferStatus primitive"),
              TestCase::QUICK);
}

static LteRlcUmTransmitterTestSuite lteRlcUmTransmitterTestSuite;

LteRlcUmTransmitterTestCase::LteRlcUmTransmitterTestCase(std::string name)
    : TestCase(name) {}

LteRlcUmTransmitterTestCase::~LteRlcUmTransmitterTestCase() {}

void LteRlcUmTransmitterTestCase::DoRun() {

  uint16_t rnti = 1111;
  uint8_t lcid = 222;

  Packet::EnablePrinting();

  txPdcp = CreateObject<LteTestPdcp>();

  txRlc = CreateObject<LteRlcUm>();
  txRlc->SetRnti(rnti);
  txRlc->SetLcId(lcid);

  txMac = CreateObject<LteTestMac>();
  txMac->SetRlcHeaderType(LteTestMac::UM_RLC_HEADER);

  txPdcp->SetLteRlcSapProvider(txRlc->GetLteRlcSapProvider());
  txRlc->SetLteRlcSapUser(txPdcp->GetLteRlcSapUser());

  txRlc->SetLteMacSapProvider(txMac->GetLteMacSapProvider());
  txMac->SetLteMacSapUser(txRlc->GetLteMacSapUser());
}

void LteRlcUmTransmitterTestCase::CheckDataReceived(Time time,
                                                    std::string shouldReceived,
                                                    std::string assertMsg) {
  Simulator::Schedule(time, &LteRlcUmTransmitterTestCase::DoCheckDataReceived,
                      this, shouldReceived, assertMsg);
}

void LteRlcUmTransmitterTestCase::DoCheckDataReceived(
    std::string shouldReceived, std::string assertMsg) {
  NS_TEST_ASSERT_MSG_EQ(shouldReceived, txMac->GetDataReceived(), assertMsg);
}

LteRlcUmTransmitterOneSduTestCase::LteRlcUmTransmitterOneSduTestCase(
    std::string name)
    : LteRlcUmTransmitterTestCase(name) {}

LteRlcUmTransmitterOneSduTestCase::~LteRlcUmTransmitterOneSduTestCase() {}

void LteRlcUmTransmitterOneSduTestCase::DoRun() {
  LteRlcUmTransmitterTestCase::DoRun();

  txPdcp->SendData(Seconds(0.100), "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

  txMac->SendTxOpportunity(Seconds(0.150), 28);
  CheckDataReceived(Seconds(0.200), "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
                    "SDU is not OK");

  Simulator::Run();
  Simulator::Destroy();
}

LteRlcUmTransmitterSegmentationTestCase::
    LteRlcUmTransmitterSegmentationTestCase(std::string name)
    : LteRlcUmTransmitterTestCase(name) {}

LteRlcUmTransmitterSegmentationTestCase::
    ~LteRlcUmTransmitterSegmentationTestCase() {}

void LteRlcUmTransmitterSegmentationTestCase::DoRun() {
  LteRlcUmTransmitterTestCase::DoRun();

  txPdcp->SendData(Seconds(0.100), "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

  txMac->SendTxOpportunity(Seconds(0.150), 10);
  CheckDataReceived(Seconds(0.200), "ABCDEFGH", "Segment #1 is not OK");

  txMac->SendTxOpportunity(Seconds(0.200), 10);
  CheckDataReceived(Seconds(0.250), "IJKLMNOP", "Segment #2 is not OK");

  txMac->SendTxOpportunity(Seconds(0.300), 10);
  CheckDataReceived(Seconds(0.350), "QRSTUVWX", "Segment #3 is not OK");

  txMac->SendTxOpportunity(Seconds(0.400), 4);
  CheckDataReceived(Seconds(0.450), "YZ", "Segment #4 is not OK");

  Simulator::Run();
  Simulator::Destroy();
}

LteRlcUmTransmitterConcatenationTestCase::
    LteRlcUmTransmitterConcatenationTestCase(std::string name)
    : LteRlcUmTransmitterTestCase(name) {}

LteRlcUmTransmitterConcatenationTestCase::
    ~LteRlcUmTransmitterConcatenationTestCase() {}

void LteRlcUmTransmitterConcatenationTestCase::DoRun() {
  LteRlcUmTransmitterTestCase::DoRun();

  txPdcp->SendData(Seconds(0.100), "ABCDEFGH");
  txPdcp->SendData(Seconds(0.150), "IJKLMNOPQR");
  txPdcp->SendData(Seconds(0.200), "STUVWXYZ");

  txMac->SendTxOpportunity(Seconds(0.250), 31);
  CheckDataReceived(Seconds(0.300), "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
                    "Concatenation is not OK");

  Simulator::Run();
  Simulator::Destroy();
}

LteRlcUmTransmitterReportBufferStatusTestCase::
    LteRlcUmTransmitterReportBufferStatusTestCase(std::string name)
    : LteRlcUmTransmitterTestCase(name) {}

LteRlcUmTransmitterReportBufferStatusTestCase::
    ~LteRlcUmTransmitterReportBufferStatusTestCase() {}

void LteRlcUmTransmitterReportBufferStatusTestCase::DoRun() {
  LteRlcUmTransmitterTestCase::DoRun();

  txPdcp->SendData(Seconds(0.100), "ABCDEFGHIJ");
  txPdcp->SendData(Seconds(0.150), "KLMNOPQRS");
  txPdcp->SendData(Seconds(0.200), "TUVWXYZ");

  txMac->SendTxOpportunity(Seconds(0.250), (2 + 2) + (10 + 6));
  CheckDataReceived(Seconds(0.300), "ABCDEFGHIJKLMNOP", "SDU is not OK");

  txPdcp->SendData(Seconds(0.350), "ABCDEFGH");
  txPdcp->SendData(Seconds(0.400), "IJKLMNOPQRST");
  txPdcp->SendData(Seconds(0.450), "UVWXYZ");

  txMac->SendTxOpportunity(Seconds(0.500), 2 + 3);
  CheckDataReceived(Seconds(0.550), "QRS", "SDU is not OK");

  txPdcp->SendData(Seconds(0.600), "ABCDEFGH");
  txPdcp->SendData(Seconds(0.650), "IJKLMNOPQRST");
  txPdcp->SendData(Seconds(0.700), "UVWXYZ");

  txPdcp->SendData(Seconds(0.750), "ABCDEFGHIJ");
  txPdcp->SendData(Seconds(0.800), "KLMNOPQRST");
  txPdcp->SendData(Seconds(0.850), "UVWXYZ");

  txMac->SendTxOpportunity(Seconds(0.900), 2 + 7);
  CheckDataReceived(Seconds(0.950), "TUVWXYZ", "SDU is not OK");

  txMac->SendTxOpportunity(Seconds(1.000), (2 + 2) + (8 + 2));
  CheckDataReceived(Seconds(1.050), "ABCDEFGHIJ", "SDU is not OK");

  txPdcp->SendData(Seconds(1.100), "ABCDEFGHIJ");
  txPdcp->SendData(Seconds(1.150), "KLMNOPQRST");
  txPdcp->SendData(Seconds(1.200), "UVWXYZ");

  txMac->SendTxOpportunity(Seconds(1.250), 2 + 2);
  CheckDataReceived(Seconds(1.300), "KL", "SDU is not OK");

  txMac->SendTxOpportunity(Seconds(1.350), 2 + 3);
  CheckDataReceived(Seconds(1.400), "MNO", "SDU is not OK");

  txMac->SendTxOpportunity(Seconds(1.450), 2 + 5);
  CheckDataReceived(Seconds(1.500), "PQRST", "SDU is not OK");

  txMac->SendTxOpportunity(Seconds(1.550), (2 + 2 + 1 + 2 + 1 + 2 + 1) +
                                               (6 + 8 + 12 + 6 + 10 + 10 + 3));
  CheckDataReceived(Seconds(1.600),
                    "UVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVW",
                    "SDU is not OK");

  txMac->SendTxOpportunity(Seconds(1.650), (2 + 2 + 1 + 2) + (3 + 10 + 10 + 6));
  CheckDataReceived(Seconds(1.700), "XYZABCDEFGHIJKLMNOPQRSTUVWXYZ",
                    "SDU is not OK");

  Simulator::Run();
  Simulator::Destroy();
}
