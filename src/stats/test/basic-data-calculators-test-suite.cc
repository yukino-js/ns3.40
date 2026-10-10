
#include "ns3/basic-data-calculators.h"
#include "ns3/test.h"

#include <cmath>

using namespace ns3;

const double TOLERANCE = 1e-13;

class OneIntegerTestCase : public TestCase {
public:
  OneIntegerTestCase();
  ~OneIntegerTestCase() override;

private:
  void DoRun() override;
};

OneIntegerTestCase::OneIntegerTestCase()
    : TestCase("Basic Statistical Functions using One Integer")

{}

OneIntegerTestCase::~OneIntegerTestCase() {}

void OneIntegerTestCase::DoRun() {
  MinMaxAvgTotalCalculator<int> calculator;

  long count = 1;

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
  variance = 0;
  stddev = std::sqrt(variance);

  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.getCount(), count, TOLERANCE,
                            "Count value wrong");
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.getSum(), sum, TOLERANCE,
                            "Sum value wrong");
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.getMin(), min, TOLERANCE,
                            "Min value wrong");
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.getMax(), max, TOLERANCE,
                            "Max value wrong");
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.getMean(), mean, TOLERANCE,
                            "Mean value wrong");
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.getStddev(), stddev, TOLERANCE,
                            "Stddev value wrong");
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.getVariance(), variance, TOLERANCE,
                            "Variance value wrong");
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.getSqrSum(), sqrSum, TOLERANCE,
                            "SqrSum value wrong");
}

class FiveIntegersTestCase : public TestCase {
public:
  FiveIntegersTestCase();
  ~FiveIntegersTestCase() override;

private:
  void DoRun() override;
};

FiveIntegersTestCase::FiveIntegersTestCase()
    : TestCase("Basic Statistical Functions using Five Integers")

{}

FiveIntegersTestCase::~FiveIntegersTestCase() {}

void FiveIntegersTestCase::DoRun() {
  MinMaxAvgTotalCalculator<int> calculator;

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

  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.getCount(), count, TOLERANCE,
                            "Count value wrong");
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.getSum(), sum, TOLERANCE,
                            "Sum value wrong");
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.getMin(), min, TOLERANCE,
                            "Min value wrong");
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.getMax(), max, TOLERANCE,
                            "Max value wrong");
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.getMean(), mean, TOLERANCE,
                            "Mean value wrong");
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.getStddev(), stddev, TOLERANCE,
                            "Stddev value wrong");
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.getVariance(), variance, TOLERANCE,
                            "Variance value wrong");
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.getSqrSum(), sqrSum, TOLERANCE,
                            "SqrSum value wrong");
}

class FiveDoublesTestCase : public TestCase {
public:
  FiveDoublesTestCase();
  ~FiveDoublesTestCase() override;

private:
  void DoRun() override;
};

FiveDoublesTestCase::FiveDoublesTestCase()
    : TestCase("Basic Statistical Functions using Five Double Values")

{}

FiveDoublesTestCase::~FiveDoublesTestCase() {}

void FiveDoublesTestCase::DoRun() {
  MinMaxAvgTotalCalculator<double> calculator;

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

  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.getCount(), count, TOLERANCE,
                            "Count value wrong");
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.getSum(), sum, TOLERANCE,
                            "Sum value wrong");
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.getMin(), min, TOLERANCE,
                            "Min value wrong");
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.getMax(), max, TOLERANCE,
                            "Max value wrong");
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.getMean(), mean, TOLERANCE,
                            "Mean value wrong");
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.getStddev(), stddev, TOLERANCE,
                            "Stddev value wrong");
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.getVariance(), variance, TOLERANCE,
                            "Variance value wrong");
  NS_TEST_ASSERT_MSG_EQ_TOL(calculator.getSqrSum(), sqrSum, TOLERANCE,
                            "SqrSum value wrong");
}

class BasicDataCalculatorsTestSuite : public TestSuite {
public:
  BasicDataCalculatorsTestSuite();
};

BasicDataCalculatorsTestSuite::BasicDataCalculatorsTestSuite()
    : TestSuite("basic-data-calculators", UNIT) {
  AddTestCase(new OneIntegerTestCase, TestCase::QUICK);
  AddTestCase(new FiveIntegersTestCase, TestCase::QUICK);
  AddTestCase(new FiveDoublesTestCase, TestCase::QUICK);
}

static BasicDataCalculatorsTestSuite basicDataCalculatorsTestSuite;
