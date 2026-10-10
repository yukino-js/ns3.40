
#ifndef LTE_TEST_RLC_AM_TRANSMITTER_H
#define LTE_TEST_RLC_AM_TRANSMITTER_H

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

class LteRlcAmTransmitterTestSuite : public TestSuite {
public:
  LteRlcAmTransmitterTestSuite();
};

class LteRlcAmTransmitterTestCase : public TestCase {
public:
  LteRlcAmTransmitterTestCase(std::string name);
  LteRlcAmTransmitterTestCase();
  ~LteRlcAmTransmitterTestCase() override;

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

class LteRlcAmTransmitterOneSduTestCase : public LteRlcAmTransmitterTestCase {
public:
  LteRlcAmTransmitterOneSduTestCase(std::string name);
  LteRlcAmTransmitterOneSduTestCase();
  ~LteRlcAmTransmitterOneSduTestCase() override;

private:
  void DoRun() override;
};

class LteRlcAmTransmitterSegmentationTestCase
    : public LteRlcAmTransmitterTestCase {
public:
  LteRlcAmTransmitterSegmentationTestCase(std::string name);
  LteRlcAmTransmitterSegmentationTestCase();
  ~LteRlcAmTransmitterSegmentationTestCase() override;

private:
  void DoRun() override;
};

class LteRlcAmTransmitterConcatenationTestCase
    : public LteRlcAmTransmitterTestCase {
public:
  LteRlcAmTransmitterConcatenationTestCase(std::string name);
  LteRlcAmTransmitterConcatenationTestCase();
  ~LteRlcAmTransmitterConcatenationTestCase() override;

private:
  void DoRun() override;
};

class LteRlcAmTransmitterReportBufferStatusTestCase
    : public LteRlcAmTransmitterTestCase {
public:
  LteRlcAmTransmitterReportBufferStatusTestCase(std::string name);
  LteRlcAmTransmitterReportBufferStatusTestCase();
  ~LteRlcAmTransmitterReportBufferStatusTestCase() override;

private:
  void DoRun() override;
};

#endif
