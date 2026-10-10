
#include "ns3/data-rate.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/test.h"

using namespace ns3;

class DataRateTestCase : public TestCase {
public:
  DataRateTestCase(std::string name);
  ~DataRateTestCase() override;

  void CheckTimesEqual(Time t1, Time t2, const std::string msg);
  void CheckDataRateEqual(DataRate d1, DataRate d2, const std::string msg);

protected:
  void DoRun() override = 0;
};

DataRateTestCase::DataRateTestCase(std::string name) : TestCase(name) {}

DataRateTestCase::~DataRateTestCase() {}

void DataRateTestCase::CheckTimesEqual(Time actual, Time correct,
                                       const std::string msg) {
  int64x64_t actualFemtos = actual.GetFemtoSeconds();
  int64x64_t correctFemtos = correct.GetFemtoSeconds();
  NS_TEST_EXPECT_MSG_EQ(actualFemtos, correctFemtos, msg);
}

void DataRateTestCase::CheckDataRateEqual(DataRate d1, DataRate d2,
                                          const std::string msg) {
  NS_TEST_EXPECT_MSG_EQ(d1, d2, msg);
}

class DataRateTestCase1 : public DataRateTestCase {
public:
  DataRateTestCase1();

  void SingleTest(std::string rate, size_t nBits, Time correctTime);

private:
  void DoRun() override;
};

DataRateTestCase1::DataRateTestCase1()
    : DataRateTestCase("Test rounding of conversion from DataRate to time") {}

void DataRateTestCase1::SingleTest(std::string rate, size_t nBits,
                                   Time correctTime) {
  DataRate dr(rate);
  Time bitsTime = dr.CalculateBitsTxTime(nBits);
  CheckTimesEqual(bitsTime, correctTime,
                  "CalculateBitsTxTime returned incorrect value");
  if ((nBits % 8) == 0) {
    Time bytesTime = dr.CalculateBytesTxTime(nBits / 8);
    CheckTimesEqual(bytesTime, correctTime,
                    "CalculateBytesTxTime returned incorrect value");
  }
}

void DataRateTestCase1::DoRun() {
  if (Time::GetResolution() != Time::FS) {
    Time::SetResolution(Time::FS);
  }
  SingleTest("1GB/s", 512, Time(NanoSeconds(64)));
  SingleTest("8Gb/s", 512, Time(NanoSeconds(64)));
  SingleTest("1Gb/s", 512, Time(NanoSeconds(512)));
  SingleTest("8GB/s", 512, Time(NanoSeconds(8)));
  size_t nBits;
  for (nBits = 0; nBits <= 512; nBits++) {
    SingleTest("1Mb/s", nBits, Time(MicroSeconds(nBits)));
    SingleTest("10Mb/s", nBits, Time(NanoSeconds(nBits * 100)));
    SingleTest("100Mb/s", nBits, Time(NanoSeconds(nBits * 10)));
    SingleTest("1Gb/s", nBits, Time(NanoSeconds(nBits)));
    SingleTest("10Gb/s", nBits, Time(PicoSeconds(nBits * 100)));
    SingleTest("25Gb/s", nBits, Time(PicoSeconds(nBits * 40)));
    SingleTest("40Gb/s", nBits, Time(PicoSeconds(nBits * 25)));
    SingleTest("100Gb/s", nBits, Time(PicoSeconds(nBits * 10)));
    SingleTest("200Gb/s", nBits, Time(PicoSeconds(nBits * 5)));
    SingleTest("400Gb/s", nBits, Time(FemtoSeconds(nBits * 2500)));
  }
}

class DataRateTestCase2 : public DataRateTestCase {
public:
  DataRateTestCase2();
  void AdditionTest(std::string rate1, std::string rate2, std::string rate3);
  void SubtractionTest(std::string rate1, std::string rate2, std::string rate3);
  void MultiplicationIntTest(std::string rate1, uint64_t factor,
                             std::string rate2);
  void MultiplicationDoubleTest(std::string rate1, double factor,
                                std::string rate2);

private:
  void DoRun() override;
};

DataRateTestCase2::DataRateTestCase2()
    : DataRateTestCase("Test arithmetic on DateRate") {}

void DataRateTestCase2::AdditionTest(std::string rate1, std::string rate2,
                                     std::string rate3) {
  DataRate dr1(rate1);
  DataRate dr2(rate2);
  DataRate dr3(rate3);

  CheckDataRateEqual(dr1 + dr2, dr3,
                     "DataRate Addition returned incorrect value");

  dr1 += dr2;
  CheckDataRateEqual(dr1, dr3, "DataRate Addition returned incorrect value");
}

void DataRateTestCase2::SubtractionTest(std::string rate1, std::string rate2,
                                        std::string rate3) {
  DataRate dr1(rate1);
  DataRate dr2(rate2);
  DataRate dr3(rate3);

  CheckDataRateEqual(dr1 - dr2, dr3,
                     "DataRate Subtraction returned incorrect value");

  dr1 -= dr2;
  CheckDataRateEqual(dr1, dr3, "DataRate Subtraction returned incorrect value");
}

void DataRateTestCase2::MultiplicationIntTest(std::string rate1,
                                              uint64_t factor,
                                              std::string rate2) {
  DataRate dr1(rate1);
  DataRate dr2(rate2);

  CheckDataRateEqual(
      dr1 * factor, dr2,
      "DataRate Multiplication with Int returned incorrect value");

  dr1 *= factor;
  CheckDataRateEqual(
      dr1, dr2, "DataRate Multiplication with Int returned incorrect value");
}

void DataRateTestCase2::MultiplicationDoubleTest(std::string rate1,
                                                 double factor,
                                                 std::string rate2) {
  DataRate dr1(rate1);
  DataRate dr2(rate2);

  CheckDataRateEqual(
      dr1 * factor, dr2,
      "DataRate Multiplication with Double returned incorrect value");

  dr1 *= factor;
  CheckDataRateEqual(
      dr1, dr2, "DataRate Multiplication with Double returned incorrect value");
}

void DataRateTestCase2::DoRun() {
  AdditionTest("1Mb/s", "3Mb/s", "4Mb/s");
  AdditionTest("1Gb/s", "1b/s", "1000000001b/s");
  SubtractionTest("1Mb/s", "1b/s", "999999b/s");
  SubtractionTest("2Gb/s", "2Gb/s", "0Gb/s");
  MultiplicationIntTest("5Gb/s", 2, "10Gb/s");
  MultiplicationIntTest("4Mb/s", 1000, "4Gb/s");
  MultiplicationDoubleTest("1Gb/s", 0.001, "1Mb/s");
  MultiplicationDoubleTest("6Gb/s", 1.0 / 7.0, "857142857.14b/s");
}

class DataRateTestSuite : public TestSuite {
public:
  DataRateTestSuite();
};

DataRateTestSuite::DataRateTestSuite() : TestSuite("data-rate", UNIT) {
  AddTestCase(new DataRateTestCase1(), TestCase::QUICK);
  AddTestCase(new DataRateTestCase2(), TestCase::QUICK);
}

static DataRateTestSuite sDataRateTestSuite;
