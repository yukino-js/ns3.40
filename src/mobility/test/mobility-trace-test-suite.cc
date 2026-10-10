
#include "ns3/ascii-test.h"
#include "ns3/double.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/node-container.h"
#include "ns3/rectangle.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/trace-helper.h"
#include "ns3/uinteger.h"

#include <string>

using namespace ns3;

class MobilityTraceTestCase : public TestCase {
public:
  MobilityTraceTestCase();
  ~MobilityTraceTestCase() override;

private:
  void DoRun() override;
};

MobilityTraceTestCase::MobilityTraceTestCase()
    : TestCase("Mobility Trace Test Case") {}

MobilityTraceTestCase::~MobilityTraceTestCase() {}

void MobilityTraceTestCase::DoRun() {

  NodeContainer sta;
  sta.Create(4);
  MobilityHelper mobility;
  mobility.SetPositionAllocator(
      "ns3::GridPositionAllocator", "MinX", DoubleValue(1.0), "MinY",
      DoubleValue(1.0), "DeltaX", DoubleValue(5.0), "DeltaY", DoubleValue(5.0),
      "GridWidth", UintegerValue(3), "LayoutType", StringValue("RowFirst"));
  mobility.SetMobilityModel(
      "ns3::RandomWalk2dMobilityModel", "Mode", StringValue("Time"), "Time",
      StringValue("2s"), "Speed",
      StringValue("ns3::ConstantRandomVariable[Constant=1.0]"), "Bounds",
      RectangleValue(Rectangle(0.0, 20.0, 0.0, 20.0)));
  mobility.Install(sta);
  mobility.AssignStreams(sta, 0);

  SetDataDir(NS_TEST_SOURCEDIR);
  std::string referenceMobilityFilePath =
      CreateDataDirFilename("mobility-trace-example.mob");
  std::string testMobilityFilePath =
      CreateTempDirFilename("mobility-trace-test.mob");

  AsciiTraceHelper ascii;
  MobilityHelper::EnableAsciiAll(ascii.CreateFileStream(testMobilityFilePath));
  Simulator::Stop(Seconds(5.0));
  Simulator::Run();
  Simulator::Destroy();

  NS_ASCII_TEST_EXPECT_EQ(testMobilityFilePath, referenceMobilityFilePath);
}

class MobilityTraceTestSuite : public TestSuite {
public:
  MobilityTraceTestSuite();
};

MobilityTraceTestSuite::MobilityTraceTestSuite()
    : TestSuite("mobility-trace", UNIT) {
  AddTestCase(new MobilityTraceTestCase, TestCase::QUICK);
}

static MobilityTraceTestSuite mobilityTraceTestSuite;
