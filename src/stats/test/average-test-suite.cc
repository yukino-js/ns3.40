
#include "ns3/average.h"
#include "ns3/test.h"

#include <cmath>

using namespace ns3;

const double TOLERANCE = 2e-14;

class OneIntegerAverageTestCase : public TestCase {
public:
  OneIntegerAverageTestCase();
  ~OneIntegerAverageTestCase() override;

private:
  void DoRun() override;
};

OneIntegerAverageTestCase::OneIntegerAverageTestCase()
    : TestCase("Average Object Test using One Integer")

{}

OneIntegerAverageTestCase::~OneIntegerAverageTestCase() {}

void OneIntegerAverageTestCase::DoRun() {
  Average<int> calculator;

  long count = 1;

  double sum = 0;
  double min;
  double max;
  double mean;
  double stddev;
  double variance;

  int multiple = 5;
  int value;
  for (long i = 0; i < count; i++) {
    value = multiple * (i + 1);

    calculator.Update(value);

    sum += value;
  }

  min = multiple;
  max = multiple * count;
  mean = sum / count;
  variance = 0;
  stddev = std::sqrt(variance);

  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.Count(), count, TOLERANCE,
                            "Count value outside of tolerance "
                                << TOLERANCE << "; difference: "
                                << calculator.Count() - count);
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.Min(), min, TOLERANCE,
                            "Min value outside of tolerance "
                                << TOLERANCE
                                << "; difference: " << calculator.Min() - min);
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.Max(), max, TOLERANCE,
                            "Max value outside of tolerance "
                                << TOLERANCE
                                << "; difference: " << calculator.Max() - max);
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.Mean(), mean, TOLERANCE,
                            "Mean value outside of tolerance "
                                << TOLERANCE << "; difference: "
                                << calculator.Mean() - mean);
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.Stddev(), stddev, TOLERANCE,
                            "Stddev value outside of tolerance "
                                << TOLERANCE << "; difference: "
                                << calculator.Stddev() - stddev);
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.Var(), variance, TOLERANCE,
                            "Variance value outside of tolerance "
                                << TOLERANCE << "; difference: "
                                << calculator.Var() - variance);
}

class FiveIntegersAverageTestCase : public TestCase {
public:
  FiveIntegersAverageTestCase();
  ~FiveIntegersAverageTestCase() override;

private:
  void DoRun() override;
};

FiveIntegersAverageTestCase::FiveIntegersAverageTestCase()
    : TestCase("Average Object Test using Five Integers")

{}

FiveIntegersAverageTestCase::~FiveIntegersAverageTestCase() {}

void FiveIntegersAverageTestCase::DoRun() {
  Average<int> calculator;

  long count = 5;

  double sum = 0;
  double sqrSum = 0;
  double min;
  double max;
  double mean;
  double stddev;
  double variance;

  int multiple = 5;
  int value;
  for (long i = 0; i < count; i++) {
    value = multiple * (i + 1);

    calculator.Update(value);

    sum += value;
    sqrSum += value * value;
  }

  min = multiple;
  max = multiple * count;
  mean = sum / count;
  variance = (count * sqrSum - sum * sum) / (count * (count - 1));
  stddev = std::sqrt(variance);

  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.Count(), count, TOLERANCE,
                            "Count value outside of tolerance "
                                << TOLERANCE << "; difference: "
                                << calculator.Count() - count);
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.Min(), min, TOLERANCE,
                            "Min value outside of tolerance "
                                << TOLERANCE
                                << "; difference: " << calculator.Min() - min);
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.Max(), max, TOLERANCE,
                            "Max value outside of tolerance "
                                << TOLERANCE
                                << "; difference: " << calculator.Max() - max);
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.Mean(), mean, TOLERANCE,
                            "Mean value outside of tolerance "
                                << TOLERANCE << "; difference: "
                                << calculator.Mean() - mean);
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.Stddev(), stddev, TOLERANCE,
                            "Stddev value outside of tolerance "
                                << TOLERANCE << "; difference: "
                                << calculator.Stddev() - stddev);
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.Var(), variance, TOLERANCE,
                            "Variance value outside of tolerance "
                                << TOLERANCE << "; difference: "
                                << calculator.Var() - variance);
}

class FiveDoublesAverageTestCase : public TestCase {
public:
  FiveDoublesAverageTestCase();
  ~FiveDoublesAverageTestCase() override;

private:
  void DoRun() override;
};

FiveDoublesAverageTestCase::FiveDoublesAverageTestCase()
    : TestCase("Average Object Test using Five Double Values")

{}

FiveDoublesAverageTestCase::~FiveDoublesAverageTestCase() {}

void FiveDoublesAverageTestCase::DoRun() {
  Average<double> calculator;

  long count = 5;

  double sum = 0;
  double sqrSum = 0;
  double min;
  double max;
  double mean;
  double stddev;
  double variance;

  double multiple = 3.14;
  double value;
  for (long i = 0; i < count; i++) {
    value = multiple * (i + 1);

    calculator.Update(value);

    sum += value;
    sqrSum += value * value;
  }

  min = multiple;
  max = multiple * count;
  mean = sum / count;
  variance = (count * sqrSum - sum * sum) / (count * (count - 1));
  stddev = std::sqrt(variance);

  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.Count(), count, TOLERANCE,
                            "Count value outside of tolerance "
                                << TOLERANCE << "; difference: "
                                << calculator.Count() - count);
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.Min(), min, TOLERANCE,
                            "Min value outside of tolerance "
                                << TOLERANCE
                                << "; difference: " << calculator.Min() - min);
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.Max(), max, TOLERANCE,
                            "Max value outside of tolerance "
                                << TOLERANCE
                                << "; difference: " << calculator.Max() - max);
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.Mean(), mean, TOLERANCE,
                            "Mean value outside of tolerance "
                                << TOLERANCE << "; difference: "
                                << calculator.Mean() - mean);
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.Stddev(), stddev, TOLERANCE,
                            "Stddev value outside of tolerance "
                                << TOLERANCE << "; difference: "
                                << calculator.Stddev() - stddev);
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.Var(), variance, TOLERANCE,
                            "Variance value outside of tolerance "
                                << TOLERANCE << "; difference: "
                                << calculator.Var() - variance);
}

class AverageTestSuite : public TestSuite {
public:
  AverageTestSuite();
};

AverageTestSuite::AverageTestSuite() : TestSuite("average", UNIT) {
  AddTestCase(new OneIntegerAverageTestCase, TestCase::QUICK);
  AddTestCase(new FiveIntegersAverageTestCase, TestCase::QUICK);
  AddTestCase(new FiveDoublesAverageTestCase, TestCase::QUICK);
}

static AverageTestSuite averageTestSuite;
