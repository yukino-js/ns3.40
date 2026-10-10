
#ifndef LTE_TEST_RLC_UM_TRANSMITTER_H
#define LTE_TEST_RLC_UM_TRANSMITTER_H

#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/test.h"

namespace ns3 {

class LteTestRrc;
class LteTestMac;
class LteTestPdcp;
class LteRlc;

} // namespace ns3

using namespace ns3;

class LteRlcUmTransmitterTestSuite : public TestSuite {
public:
  LteRlcUmTransmitterTestSuite();
};

class LteRlcUmTransmitterTestCase : public TestCase {
public:
  LteRlcUmTransmitterTestCase(std::string name);
  LteRlcUmTransmitterTestCase();
  ~LteRlcUmTransmitterTestCase() override;

  void CheckDataReceived(Time time, std::string shouldReceived,
                         std::string assertMsg);

protected:
  void DoRun() override;

  Ptr<LteTestPdcp> txPdcp;
  Ptr<LteRlc> txRlc;
  Ptr<LteTestMac> txMac;

private:
  void DoCheckDataReceived(std::string shouldReceived, std::string assertMsg);
};

class LteRlcUmTransmitterOneSduTestCase : public LteRlcUmTransmitterTestCase {
public:
  LteRlcUmTransmitterOneSduTestCase(std::string name);
  LteRlcUmTransmitterOneSduTestCase();
  ~LteRlcUmTransmitterOneSduTestCase() override;

private:
  void DoRun() override;
};

class LteRlcUmTransmitterSegmentationTestCase
    : public LteRlcUmTransmitterTestCase {
public:
  LteRlcUmTransmitterSegmentationTestCase(std::string name);
  LteRlcUmTransmitterSegmentationTestCase();
  ~LteRlcUmTransmitterSegmentationTestCase() override;

private:
  void DoRun() override;
};

class LteRlcUmTransmitterConcatenationTestCase
    : public LteRlcUmTransmitterTestCase {
public:
  LteRlcUmTransmitterConcatenationTestCase(std::string name);
  LteRlcUmTransmitterConcatenationTestCase();
  ~LteRlcUmTransmitterConcatenationTestCase() override;

private:
  void DoRun() override;
};

class LteRlcUmTransmitterReportBufferStatusTestCase
    : public LteRlcUmTransmitterTestCase {
public:
  LteRlcUmTransmitterReportBufferStatusTestCase(std::string name);
  LteRlcUmTransmitterReportBufferStatusTestCase();
  ~LteRlcUmTransmitterReportBufferStatusTestCase() override;

private:
  void DoRun() override;
};

#endif
