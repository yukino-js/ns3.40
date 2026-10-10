
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/integer.h"
#include "ns3/log.h"
#include "ns3/random-variable-stream.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/string.h"
#include "ns3/test.h"

#include <cmath>
#include <ctime>
#include <fstream>
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_histogram.h>
#include <gsl/gsl_sf_zeta.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("RandomVariableStreamGenerators");

namespace ns3 {

namespace test {

namespace RandomVariable {

class TestCaseBase : public TestCase {
public:
  static const uint32_t N_BINS{50};
  static const uint32_t N_MEASUREMENTS{1000000};
  static const uint32_t N_RUNS{5};

  TestCaseBase(std::string name) : TestCase(name) {}

  std::vector<double> UniformHistogramBins(gsl_histogram *h, double start,
                                           double end, bool underflow = true,
                                           bool overflow = true) const {
    NS_LOG_FUNCTION(this << h << start << end);
    std::size_t nBins = gsl_histogram_bins(h);
    double increment = (end - start) / (nBins - 1.);
    double d = start;

    std::vector<double> range(nBins + 1);

    for (auto &r : range) {
      r = d;
      d += increment;
    }
    if (underflow) {
      range[0] = -std::numeric_limits<double>::max();
    }
    if (overflow) {
      range[nBins] = std::numeric_limits<double>::max();
    }

    gsl_histogram_set_ranges(h, range.data(), nBins + 1);
    return range;
  }

  double Average(Ptr<RandomVariableStream> rng) const {
    NS_LOG_FUNCTION(this << rng);
    double sum = 0.0;
    for (uint32_t i = 0; i < N_MEASUREMENTS; ++i) {
      double value = rng->GetValue();
      sum += value;
    }
    double valueMean = sum / N_MEASUREMENTS;
    return valueMean;
  }

  class RngGeneratorBase {
  public:
    virtual Ptr<RandomVariableStream> Create() const = 0;
  };

  template <typename RNG> class RngGenerator : public RngGeneratorBase {
  public:
    RngGenerator(bool anti = false) : m_anti(anti) {}

    Ptr<RandomVariableStream> Create() const override {
      auto rng = CreateObject<RNG>();
      rng->SetAttribute("Antithetic", BooleanValue(m_anti));
      return rng;
    }

  private:
    bool m_anti;
  };

  double ChiSquared(gsl_histogram *h, const std::vector<double> &expected,
                    Ptr<RandomVariableStream> rng) const {
    NS_LOG_FUNCTION(this << h << expected.size() << rng);
    NS_ASSERT_MSG(gsl_histogram_bins(h) == expected.size(),
                  "Histogram and expected vector have different sizes.");

    for (std::size_t i = 0; i < N_MEASUREMENTS; ++i) {
      double value = rng->GetValue();
      gsl_histogram_increment(h, value);
    }

    double chiSquared = 0;
    std::size_t nBins = gsl_histogram_bins(h);
    for (std::size_t i = 0; i < nBins; ++i) {
      double hbin = gsl_histogram_get(h, i);
      double tmp = hbin - expected[i];
      tmp *= tmp;
      tmp /= expected[i];
      chiSquared += tmp;
    }

    return chiSquared;
  }

  virtual double ChiSquaredTest(Ptr<RandomVariableStream> rng) const {
    return 0;
  }

  double ChiSquaredsAverage(const RngGeneratorBase *generator,
                            std::size_t nRuns) const {
    NS_LOG_FUNCTION(this << generator << nRuns);

    double sum = 0.;
    for (std::size_t i = 0; i < nRuns; ++i) {
      auto rng = generator->Create();
      double result = ChiSquaredTest(rng);
      sum += result;
    }
    sum /= (double)nRuns;
    return sum;
  }

  void SetTestSuiteSeed() {
    if (!m_seedSet) {
      uint32_t seed;
      if (RngSeedManager::GetRun() == 0) {
        seed = static_cast<uint32_t>(time(nullptr));
        m_seedSet = true;
        NS_LOG_DEBUG(
            "Special run number value of zero; seeding with time of day: "
            << seed);
      } else {
        seed = RngSeedManager::GetSeed();
        m_seedSet = true;
        NS_LOG_DEBUG("Using the values seed: " << seed << " and run: "
                                               << RngSeedManager::GetRun());
      }
      SeedManager::SetSeed(seed);
    }
  }

private:
  bool m_seedSet = false;
};

class UniformTestCase : public TestCaseBase {
public:
  UniformTestCase();

  double ChiSquaredTest(Ptr<RandomVariableStream> rng) const override;

private:
  void DoRun() override;
};

UniformTestCase::UniformTestCase()
    : TestCaseBase("Uniform Random Variable Stream Generator") {}

double UniformTestCase::ChiSquaredTest(Ptr<RandomVariableStream> rng) const {
  gsl_histogram *h = gsl_histogram_alloc(N_BINS);

  gsl_histogram_set_ranges_uniform(h, 0., 1.);

  std::vector<double> expected(N_BINS,
                               ((double)N_MEASUREMENTS / (double)N_BINS));

  double chiSquared = ChiSquared(h, expected, rng);
  gsl_histogram_free(h);
  return chiSquared;
}

void UniformTestCase::DoRun() {
  NS_LOG_FUNCTION(this);
  SetTestSuiteSeed();

  double confidence = 0.99;
  double maxStatistic = gsl_cdf_chisq_Pinv(confidence, (N_BINS - 1));
  NS_LOG_DEBUG("Chi square required at " << confidence << " confidence for "
                                         << N_BINS << " bins is "
                                         << maxStatistic);

  double result = maxStatistic;
  for (uint32_t i = 0; i < N_RUNS; ++i) {
    Ptr<UniformRandomVariable> rng = CreateObject<UniformRandomVariable>();
    result = ChiSquaredTest(rng);
    NS_LOG_DEBUG("Chi square result is " << result);
    if (result < maxStatistic) {
      break;
    }
  }

  NS_TEST_ASSERT_MSG_LT(result, maxStatistic,
                        "Chi-squared statistic out of range");

  double min = 0.0;
  double max = 10.0;
  double value;

  Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable>();

  x->SetAttribute("Min", DoubleValue(min));
  x->SetAttribute("Max", DoubleValue(max));

  for (uint32_t i = 0; i < N_MEASUREMENTS; ++i) {
    value = x->GetValue();
    NS_TEST_ASSERT_MSG_EQ((value >= min), true, "Value less than minimum.");
    NS_TEST_ASSERT_MSG_LT(value, max,
                          "Value greater than or equal to maximum.");
  }

  static const uint32_t UNIFORM_INTEGER_MIN{0};
  static const uint32_t UNIFORM_INTEGER_MAX{4294967295U};
  uint32_t intValue;
  intValue = x->GetInteger(UNIFORM_INTEGER_MIN, UNIFORM_INTEGER_MIN);
  NS_TEST_ASSERT_MSG_EQ(intValue, UNIFORM_INTEGER_MIN,
                        "Uniform RV GetInteger boundary testing");
  intValue = x->GetInteger(UNIFORM_INTEGER_MAX, UNIFORM_INTEGER_MAX);
  NS_TEST_ASSERT_MSG_EQ(intValue, UNIFORM_INTEGER_MAX,
                        "Uniform RV GetInteger boundary testing");
  intValue = 0;
  for (int i = 0; i < 20; i++) {
    intValue += x->GetInteger(UNIFORM_INTEGER_MIN, UNIFORM_INTEGER_MIN + 1);
  }
  NS_TEST_ASSERT_MSG_GT(intValue, 0, "Uniform RV GetInteger boundary testing");
  NS_TEST_ASSERT_MSG_LT(intValue, 20, "Uniform RV GetInteger boundary testing");
  uint32_t count = 0;
  for (int i = 0; i < 20; i++) {
    intValue = x->GetInteger(UNIFORM_INTEGER_MAX - 1, UNIFORM_INTEGER_MAX);
    if (intValue == UNIFORM_INTEGER_MAX) {
      count++;
    }
  }
  NS_TEST_ASSERT_MSG_GT(count, 0, "Uniform RV GetInteger boundary testing");
  NS_TEST_ASSERT_MSG_LT(count, 20, "Uniform RV GetInteger boundary testing");
  intValue = x->GetInteger(UNIFORM_INTEGER_MIN, UNIFORM_INTEGER_MAX);
  uint32_t intValue2 = x->GetInteger(UNIFORM_INTEGER_MIN, UNIFORM_INTEGER_MAX);
  NS_TEST_ASSERT_MSG_GT(intValue + intValue2, 0,
                        "Uniform RV GetInteger boundary testing");
}

class UniformAntitheticTestCase : public TestCaseBase {
public:
  UniformAntitheticTestCase();

  double ChiSquaredTest(Ptr<RandomVariableStream> rng) const override;

private:
  void DoRun() override;
};

UniformAntitheticTestCase::UniformAntitheticTestCase()
    : TestCaseBase("Antithetic Uniform Random Variable Stream Generator") {}

double
UniformAntitheticTestCase::ChiSquaredTest(Ptr<RandomVariableStream> rng) const {
  gsl_histogram *h = gsl_histogram_alloc(N_BINS);

  gsl_histogram_set_ranges_uniform(h, 0., 1.);

  std::vector<double> expected(N_BINS,
                               ((double)N_MEASUREMENTS / (double)N_BINS));

  double chiSquared = ChiSquared(h, expected, rng);
  gsl_histogram_free(h);
  return chiSquared;
}

void UniformAntitheticTestCase::DoRun() {
  NS_LOG_FUNCTION(this);
  SetTestSuiteSeed();

  auto generator = RngGenerator<UniformRandomVariable>(true);
  double sum = ChiSquaredsAverage(&generator, N_RUNS);
  double maxStatistic = gsl_cdf_chisq_Qinv(0.05, N_BINS);
  NS_TEST_ASSERT_MSG_LT(sum, maxStatistic,
                        "Chi-squared statistic out of range");

  double min = 0.0;
  double max = 10.0;
  double value;

  Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable>();

  x->SetAttribute("Antithetic", BooleanValue(true));

  x->SetAttribute("Min", DoubleValue(min));
  x->SetAttribute("Max", DoubleValue(max));

  for (uint32_t i = 0; i < N_MEASUREMENTS; ++i) {
    value = x->GetValue();
    NS_TEST_ASSERT_MSG_EQ((value >= min), true, "Value less than minimum.");
    NS_TEST_ASSERT_MSG_LT(value, max,
                          "Value greater than or equal to maximum.");
  }
}

class ConstantTestCase : public TestCaseBase {
public:
  ConstantTestCase();

private:
  void DoRun() override;

  static constexpr double TOLERANCE{1e-8};
};

ConstantTestCase::ConstantTestCase()
    : TestCaseBase("Constant Random Variable Stream Generator") {}

void ConstantTestCase::DoRun() {
  NS_LOG_FUNCTION(this);
  SetTestSuiteSeed();

  Ptr<ConstantRandomVariable> c = CreateObject<ConstantRandomVariable>();

  double constant;

  constant = 10.0;
  c->SetAttribute("Constant", DoubleValue(constant));
  NS_TEST_ASSERT_MSG_EQ_TOL(c->GetValue(), constant, TOLERANCE,
                            "Constant value changed");
  c->SetAttribute("Constant", DoubleValue(20.0));
  NS_TEST_ASSERT_MSG_NE(c->GetValue(), constant, "Constant value not changed");

  constant = c->GetValue();
  for (uint32_t i = 0; i < N_MEASUREMENTS; ++i) {
    NS_TEST_ASSERT_MSG_EQ_TOL(c->GetValue(), constant, TOLERANCE,
                              "Constant value changed in loop");
  }
}

class SequentialTestCase : public TestCaseBase {
public:
  SequentialTestCase();

private:
  void DoRun() override;

  static constexpr double TOLERANCE{1e-8};
};

SequentialTestCase::SequentialTestCase()
    : TestCaseBase("Sequential Random Variable Stream Generator") {}

void SequentialTestCase::DoRun() {
  NS_LOG_FUNCTION(this);
  SetTestSuiteSeed();

  Ptr<SequentialRandomVariable> s = CreateObject<SequentialRandomVariable>();

  s->SetAttribute("Min", DoubleValue(4));
  s->SetAttribute("Max", DoubleValue(11));
  s->SetAttribute("Increment",
                  StringValue("ns3::UniformRandomVariable[Min=3.0|Max=3.0]"));
  s->SetAttribute("Consecutive", IntegerValue(2));

  double value;

  value = s->GetValue();
  NS_TEST_ASSERT_MSG_EQ_TOL(value, 4, TOLERANCE, "Sequence value 1 wrong.");
  value = s->GetValue();
  NS_TEST_ASSERT_MSG_EQ_TOL(value, 4, TOLERANCE, "Sequence value 2 wrong.");
  value = s->GetValue();
  NS_TEST_ASSERT_MSG_EQ_TOL(value, 7, TOLERANCE, "Sequence value 3 wrong.");
  value = s->GetValue();
  NS_TEST_ASSERT_MSG_EQ_TOL(value, 7, TOLERANCE, "Sequence value 4 wrong.");
  value = s->GetValue();
  NS_TEST_ASSERT_MSG_EQ_TOL(value, 10, TOLERANCE, "Sequence value 5 wrong.");
  value = s->GetValue();
  NS_TEST_ASSERT_MSG_EQ_TOL(value, 10, TOLERANCE, "Sequence value 6 wrong.");
}

class NormalTestCase : public TestCaseBase {
public:
  NormalTestCase();

  double ChiSquaredTest(Ptr<RandomVariableStream> rng) const override;

private:
  void DoRun() override;

  static constexpr double TOLERANCE{5};
};

NormalTestCase::NormalTestCase()
    : TestCaseBase("Normal Random Variable Stream Generator") {}

double NormalTestCase::ChiSquaredTest(Ptr<RandomVariableStream> rng) const {
  gsl_histogram *h = gsl_histogram_alloc(N_BINS);
  auto range = UniformHistogramBins(h, -4., 4.);

  std::vector<double> expected(N_BINS);

  double sigma = 1.;

  for (std::size_t i = 0; i < N_BINS; ++i) {
    expected[i] = gsl_cdf_gaussian_P(range[i + 1], sigma) -
                  gsl_cdf_gaussian_P(range[i], sigma);
    expected[i] *= N_MEASUREMENTS;
  }

  double chiSquared = ChiSquared(h, expected, rng);
  gsl_histogram_free(h);
  return chiSquared;
}

void NormalTestCase::DoRun() {
  NS_LOG_FUNCTION(this);
  SetTestSuiteSeed();

  auto generator = RngGenerator<NormalRandomVariable>();
  auto rng = generator.Create();

  double sum = ChiSquaredsAverage(&generator, N_RUNS);
  double maxStatistic = gsl_cdf_chisq_Qinv(0.05, N_BINS);
  NS_TEST_ASSERT_MSG_LT(sum, maxStatistic,
                        "Chi-squared statistic out of range");

  double mean = 5.0;
  double variance = 2.0;

  Ptr<NormalRandomVariable> x = CreateObject<NormalRandomVariable>();
  x->SetAttribute("Mean", DoubleValue(mean));
  x->SetAttribute("Variance", DoubleValue(variance));

  double valueMean = Average(x);

  double expectedMean = mean;
  double expectedRms = mean / std::sqrt(variance * N_MEASUREMENTS);

  NS_TEST_ASSERT_MSG_EQ_TOL(valueMean, expectedMean, expectedRms * TOLERANCE,
                            "Wrong mean value.");
}

class NormalAntitheticTestCase : public TestCaseBase {
public:
  NormalAntitheticTestCase();

  double ChiSquaredTest(Ptr<RandomVariableStream> rng) const override;

private:
  void DoRun() override;

  static constexpr double TOLERANCE{5};
};

NormalAntitheticTestCase::NormalAntitheticTestCase()
    : TestCaseBase("Antithetic Normal Random Variable Stream Generator") {}

double
NormalAntitheticTestCase::ChiSquaredTest(Ptr<RandomVariableStream> rng) const {
  gsl_histogram *h = gsl_histogram_alloc(N_BINS);
  auto range = UniformHistogramBins(h, -4, 4);

  std::vector<double> expected(N_BINS);

  double sigma = 1.;

  for (std::size_t i = 0; i < N_BINS; ++i) {
    expected[i] = gsl_cdf_gaussian_P(range[i + 1], sigma) -
                  gsl_cdf_gaussian_P(range[i], sigma);
    expected[i] *= N_MEASUREMENTS;
  }

  double chiSquared = ChiSquared(h, expected, rng);

  gsl_histogram_free(h);
  return chiSquared;
}

void NormalAntitheticTestCase::DoRun() {
  NS_LOG_FUNCTION(this);
  SetTestSuiteSeed();

  auto generator = RngGenerator<NormalRandomVariable>(true);
  double sum = ChiSquaredsAverage(&generator, N_RUNS);
  double maxStatistic = gsl_cdf_chisq_Qinv(0.05, N_BINS);
  NS_TEST_ASSERT_MSG_LT(sum, maxStatistic,
                        "Chi-squared statistic out of range");

  double mean = 5.0;
  double variance = 2.0;

  Ptr<NormalRandomVariable> x = CreateObject<NormalRandomVariable>();
  x->SetAttribute("Mean", DoubleValue(mean));
  x->SetAttribute("Variance", DoubleValue(variance));

  x->SetAttribute("Antithetic", BooleanValue(true));

  double valueMean = Average(x);

  double expectedMean = mean;
  double expectedRms = mean / std::sqrt(variance * N_MEASUREMENTS);

  NS_TEST_ASSERT_MSG_EQ_TOL(valueMean, expectedMean, expectedRms * TOLERANCE,
                            "Wrong mean value.");
}

class ExponentialTestCase : public TestCaseBase {
public:
  ExponentialTestCase();

  double ChiSquaredTest(Ptr<RandomVariableStream> rng) const override;

private:
  void DoRun() override;

  static constexpr double TOLERANCE{5};
};

ExponentialTestCase::ExponentialTestCase()
    : TestCaseBase("Exponential Random Variable Stream Generator") {}

double
ExponentialTestCase::ChiSquaredTest(Ptr<RandomVariableStream> rng) const {
  gsl_histogram *h = gsl_histogram_alloc(N_BINS);
  auto range = UniformHistogramBins(h, 0, 10, false);

  std::vector<double> expected(N_BINS);

  double mu = 1.;

  for (std::size_t i = 0; i < N_BINS; ++i) {
    expected[i] = gsl_cdf_exponential_P(range[i + 1], mu) -
                  gsl_cdf_exponential_P(range[i], mu);
    expected[i] *= N_MEASUREMENTS;
  }

  double chiSquared = ChiSquared(h, expected, rng);

  gsl_histogram_free(h);
  return chiSquared;
}

void ExponentialTestCase::DoRun() {
  NS_LOG_FUNCTION(this);
  SetTestSuiteSeed();

  auto generator = RngGenerator<ExponentialRandomVariable>();
  double sum = ChiSquaredsAverage(&generator, N_RUNS);
  double maxStatistic = gsl_cdf_chisq_Qinv(0.05, N_BINS);
  NS_TEST_ASSERT_MSG_LT(sum, maxStatistic,
                        "Chi-squared statistic out of range");

  double mean = 3.14;
  double bound = 0.0;

  Ptr<ExponentialRandomVariable> x = CreateObject<ExponentialRandomVariable>();
  x->SetAttribute("Mean", DoubleValue(mean));
  x->SetAttribute("Bound", DoubleValue(bound));

  double valueMean = Average(x);
  double expectedMean = mean;
  double expectedRms = std::sqrt(mean / N_MEASUREMENTS);

  NS_TEST_ASSERT_MSG_EQ_TOL(valueMean, expectedMean, expectedRms * TOLERANCE,
                            "Wrong mean value.");
}

class ExponentialAntitheticTestCase : public TestCaseBase {
public:
  ExponentialAntitheticTestCase();

  double ChiSquaredTest(Ptr<RandomVariableStream> rng) const override;

private:
  void DoRun() override;

  static constexpr double TOLERANCE{5};
};

ExponentialAntitheticTestCase::ExponentialAntitheticTestCase()
    : TestCaseBase("Antithetic Exponential Random Variable Stream Generator") {}

double ExponentialAntitheticTestCase::ChiSquaredTest(
    Ptr<RandomVariableStream> rng) const {
  gsl_histogram *h = gsl_histogram_alloc(N_BINS);
  auto range = UniformHistogramBins(h, 0, 10, false);

  std::vector<double> expected(N_BINS);

  double mu = 1.;

  for (std::size_t i = 0; i < N_BINS; ++i) {
    expected[i] = gsl_cdf_exponential_P(range[i + 1], mu) -
                  gsl_cdf_exponential_P(range[i], mu);
    expected[i] *= N_MEASUREMENTS;
  }

  double chiSquared = ChiSquared(h, expected, rng);

  gsl_histogram_free(h);
  return chiSquared;
}

void ExponentialAntitheticTestCase::DoRun() {
  NS_LOG_FUNCTION(this);
  SetTestSuiteSeed();

  auto generator = RngGenerator<ExponentialRandomVariable>(true);
  double sum = ChiSquaredsAverage(&generator, N_RUNS);
  double maxStatistic = gsl_cdf_chisq_Qinv(0.05, N_BINS);
  NS_TEST_ASSERT_MSG_LT(sum, maxStatistic,
                        "Chi-squared statistic out of range");

  double mean = 3.14;
  double bound = 0.0;

  Ptr<ExponentialRandomVariable> x = CreateObject<ExponentialRandomVariable>();
  x->SetAttribute("Mean", DoubleValue(mean));
  x->SetAttribute("Bound", DoubleValue(bound));

  x->SetAttribute("Antithetic", BooleanValue(true));

  double valueMean = Average(x);
  double expectedMean = mean;
  double expectedRms = std::sqrt(mean / N_MEASUREMENTS);

  NS_TEST_ASSERT_MSG_EQ_TOL(valueMean, expectedMean, expectedRms * TOLERANCE,
                            "Wrong mean value.");
}

class ParetoTestCase : public TestCaseBase {
public:
  ParetoTestCase();

  double ChiSquaredTest(Ptr<RandomVariableStream> rng) const override;

private:
  void DoRun() override;

  static constexpr double TOLERANCE{1e-2};
};

ParetoTestCase::ParetoTestCase()
    : TestCaseBase("Pareto Random Variable Stream Generator") {}

double ParetoTestCase::ChiSquaredTest(Ptr<RandomVariableStream> rng) const {
  gsl_histogram *h = gsl_histogram_alloc(N_BINS);
  auto range = UniformHistogramBins(h, 1, 10, false);

  std::vector<double> expected(N_BINS);

  double shape = 2.0;
  double scale = 1.0;

  for (std::size_t i = 0; i < N_BINS; ++i) {
    expected[i] = gsl_cdf_pareto_P(range[i + 1], shape, scale) -
                  gsl_cdf_pareto_P(range[i], shape, scale);
    expected[i] *= N_MEASUREMENTS;
  }

  double chiSquared = ChiSquared(h, expected, rng);

  gsl_histogram_free(h);
  return chiSquared;
}

void ParetoTestCase::DoRun() {
  NS_LOG_FUNCTION(this);
  SetTestSuiteSeed();

  auto generator = RngGenerator<ParetoRandomVariable>();
  double sum = ChiSquaredsAverage(&generator, N_RUNS);
  double maxStatistic = gsl_cdf_chisq_Qinv(0.05, N_BINS);
  NS_TEST_ASSERT_MSG_LT(sum, maxStatistic,
                        "Chi-squared statistic out of range");

  double shape = 2.0;
  double scale = 1.0;

  Ptr<ParetoRandomVariable> x = CreateObject<ParetoRandomVariable>();
  x->SetAttribute("Shape", DoubleValue(shape));
  x->SetAttribute("Scale", DoubleValue(scale));

  double valueMean = Average(x);

  double expectedMean = (shape * scale) / (shape - 1.0);

  NS_TEST_ASSERT_MSG_EQ_TOL(valueMean, expectedMean, expectedMean * TOLERANCE,
                            "Wrong mean value.");
}

class ParetoAntitheticTestCase : public TestCaseBase {
public:
  ParetoAntitheticTestCase();

  double ChiSquaredTest(Ptr<RandomVariableStream> rng) const override;

private:
  void DoRun() override;

  static constexpr double TOLERANCE{1e-2};
};

ParetoAntitheticTestCase::ParetoAntitheticTestCase()
    : TestCaseBase("Antithetic Pareto Random Variable Stream Generator") {}

double
ParetoAntitheticTestCase::ChiSquaredTest(Ptr<RandomVariableStream> rng) const {
  gsl_histogram *h = gsl_histogram_alloc(N_BINS);
  auto range = UniformHistogramBins(h, 1, 10, false);

  std::vector<double> expected(N_BINS);

  double shape = 2.0;
  double scale = 1.0;

  for (std::size_t i = 0; i < N_BINS; ++i) {
    expected[i] = gsl_cdf_pareto_P(range[i + 1], shape, scale) -
                  gsl_cdf_pareto_P(range[i], shape, scale);
    expected[i] *= N_MEASUREMENTS;
  }

  double chiSquared = ChiSquared(h, expected, rng);

  gsl_histogram_free(h);
  return chiSquared;
}

void ParetoAntitheticTestCase::DoRun() {
  NS_LOG_FUNCTION(this);
  SetTestSuiteSeed();

  auto generator = RngGenerator<ParetoRandomVariable>(true);
  double sum = ChiSquaredsAverage(&generator, N_RUNS);
  double maxStatistic = gsl_cdf_chisq_Qinv(0.05, N_BINS);
  NS_TEST_ASSERT_MSG_LT(sum, maxStatistic,
                        "Chi-squared statistic out of range");

  double shape = 2.0;
  double scale = 1.0;

  Ptr<ParetoRandomVariable> x = CreateObject<ParetoRandomVariable>();
  x->SetAttribute("Shape", DoubleValue(shape));
  x->SetAttribute("Scale", DoubleValue(scale));

  x->SetAttribute("Antithetic", BooleanValue(true));

  double valueMean = Average(x);

  double expectedMean = (shape * scale) / (shape - 1.0);

  NS_TEST_ASSERT_MSG_EQ_TOL(valueMean, expectedMean, expectedMean * TOLERANCE,
                            "Wrong mean value.");
}

class WeibullTestCase : public TestCaseBase {
public:
  WeibullTestCase();

  double ChiSquaredTest(Ptr<RandomVariableStream> rng) const override;

private:
  void DoRun() override;

  static constexpr double TOLERANCE{1e-2};
};

WeibullTestCase::WeibullTestCase()
    : TestCaseBase("Weibull Random Variable Stream Generator") {}

double WeibullTestCase::ChiSquaredTest(Ptr<RandomVariableStream> rng) const {
  gsl_histogram *h = gsl_histogram_alloc(N_BINS);
  auto range = UniformHistogramBins(h, 1, 10, false);

  std::vector<double> expected(N_BINS);

  double a = 1.0;
  double b = 1.0;

  for (std::size_t i = 0; i < N_BINS; ++i) {
    expected[i] = gsl_cdf_weibull_P(range[i + 1], a, b) -
                  gsl_cdf_weibull_P(range[i], a, b);
    expected[i] *= N_MEASUREMENTS;
    NS_LOG_INFO("weibull: " << expected[i]);
  }

  double chiSquared = ChiSquared(h, expected, rng);

  gsl_histogram_free(h);
  return chiSquared;
}

void WeibullTestCase::DoRun() {
  NS_LOG_FUNCTION(this);
  SetTestSuiteSeed();

  auto generator = RngGenerator<WeibullRandomVariable>();
  double sum = ChiSquaredsAverage(&generator, N_RUNS);
  double maxStatistic = gsl_cdf_chisq_Qinv(0.05, N_BINS);
  NS_TEST_ASSERT_MSG_LT(sum, maxStatistic,
                        "Chi-squared statistic out of range");

  double scale = 5.0;
  double shape = 1.0;

  Ptr<WeibullRandomVariable> x = CreateObject<WeibullRandomVariable>();
  x->SetAttribute("Scale", DoubleValue(scale));
  x->SetAttribute("Shape", DoubleValue(shape));

  double valueMean = Average(x);

  double expectedMean = scale;

  NS_TEST_ASSERT_MSG_EQ_TOL(valueMean, expectedMean, expectedMean * TOLERANCE,
                            "Wrong mean value.");
}

class WeibullAntitheticTestCase : public TestCaseBase {
public:
  WeibullAntitheticTestCase();

  double ChiSquaredTest(Ptr<RandomVariableStream> rng) const override;

private:
  void DoRun() override;

  static constexpr double TOLERANCE{1e-2};
};

WeibullAntitheticTestCase::WeibullAntitheticTestCase()
    : TestCaseBase("Antithetic Weibull Random Variable Stream Generator") {}

double
WeibullAntitheticTestCase::ChiSquaredTest(Ptr<RandomVariableStream> rng) const {
  gsl_histogram *h = gsl_histogram_alloc(N_BINS);
  auto range = UniformHistogramBins(h, 1, 10, false);

  std::vector<double> expected(N_BINS);

  double a = 1.0;
  double b = 1.0;

  for (std::size_t i = 0; i < N_BINS; ++i) {
    expected[i] = gsl_cdf_weibull_P(range[i + 1], a, b) -
                  gsl_cdf_weibull_P(range[i], a, b);
    expected[i] *= N_MEASUREMENTS;
  }

  double chiSquared = ChiSquared(h, expected, rng);

  gsl_histogram_free(h);
  return chiSquared;
}

void WeibullAntitheticTestCase::DoRun() {
  NS_LOG_FUNCTION(this);
  SetTestSuiteSeed();

  auto generator = RngGenerator<WeibullRandomVariable>(true);
  double sum = ChiSquaredsAverage(&generator, N_RUNS);
  double maxStatistic = gsl_cdf_chisq_Qinv(0.05, N_BINS);
  NS_TEST_ASSERT_MSG_LT(sum, maxStatistic,
                        "Chi-squared statistic out of range");

  double scale = 5.0;
  double shape = 1.0;

  Ptr<WeibullRandomVariable> x = CreateObject<WeibullRandomVariable>();
  x->SetAttribute("Scale", DoubleValue(scale));
  x->SetAttribute("Shape", DoubleValue(shape));

  x->SetAttribute("Antithetic", BooleanValue(true));

  double valueMean = Average(x);

  double expectedMean = scale;

  NS_TEST_ASSERT_MSG_EQ_TOL(valueMean, expectedMean, expectedMean * TOLERANCE,
                            "Wrong mean value.");
}

class LogNormalTestCase : public TestCaseBase {
public:
  LogNormalTestCase();

  double ChiSquaredTest(Ptr<RandomVariableStream> rng) const override;

private:
  void DoRun() override;

  static constexpr double TOLERANCE{3e-2};
};

LogNormalTestCase::LogNormalTestCase()
    : TestCaseBase("Log-Normal Random Variable Stream Generator") {}

double LogNormalTestCase::ChiSquaredTest(Ptr<RandomVariableStream> rng) const {
  gsl_histogram *h = gsl_histogram_alloc(N_BINS);
  auto range = UniformHistogramBins(h, 0, 10, false);

  std::vector<double> expected(N_BINS);

  double mu = 0.0;
  double sigma = 1.0;

  for (std::size_t i = 0; i < N_BINS; ++i) {
    expected[i] = gsl_cdf_lognormal_P(range[i + 1], mu, sigma) -
                  gsl_cdf_lognormal_P(range[i], mu, sigma);
    expected[i] *= N_MEASUREMENTS;
  }

  double chiSquared = ChiSquared(h, expected, rng);

  gsl_histogram_free(h);
  return chiSquared;
}

void LogNormalTestCase::DoRun() {
  NS_LOG_FUNCTION(this);
  SetTestSuiteSeed();

  auto generator = RngGenerator<LogNormalRandomVariable>();
  double sum = ChiSquaredsAverage(&generator, N_RUNS);
  double maxStatistic = gsl_cdf_chisq_Qinv(0.05, N_BINS);

  NS_TEST_ASSERT_MSG_LT(sum, maxStatistic,
                        "Chi-squared statistic out of range");

  double mu = 5.0;
  double sigma = 2.0;

  Ptr<LogNormalRandomVariable> x = CreateObject<LogNormalRandomVariable>();
  x->SetAttribute("Mu", DoubleValue(mu));
  x->SetAttribute("Sigma", DoubleValue(sigma));

  double valueMean = Average(x);

  double expectedMean = std::exp(mu + sigma * sigma / 2.0);

  NS_TEST_ASSERT_MSG_EQ_TOL(valueMean, expectedMean, expectedMean * TOLERANCE,
                            "Wrong mean value.");
}

class LogNormalAntitheticTestCase : public TestCaseBase {
public:
  LogNormalAntitheticTestCase();

  double ChiSquaredTest(Ptr<RandomVariableStream> rng) const override;

private:
  void DoRun() override;

  static constexpr double TOLERANCE{3e-2};
};

LogNormalAntitheticTestCase::LogNormalAntitheticTestCase()
    : TestCaseBase("Antithetic Log-Normal Random Variable Stream Generator") {}

double LogNormalAntitheticTestCase::ChiSquaredTest(
    Ptr<RandomVariableStream> rng) const {
  gsl_histogram *h = gsl_histogram_alloc(N_BINS);
  auto range = UniformHistogramBins(h, 0, 10, false);

  std::vector<double> expected(N_BINS);

  double mu = 0.0;
  double sigma = 1.0;

  for (std::size_t i = 0; i < N_BINS; ++i) {
    expected[i] = gsl_cdf_lognormal_P(range[i + 1], mu, sigma) -
                  gsl_cdf_lognormal_P(range[i], mu, sigma);
    expected[i] *= N_MEASUREMENTS;
  }

  double chiSquared = ChiSquared(h, expected, rng);

  gsl_histogram_free(h);
  return chiSquared;
}

void LogNormalAntitheticTestCase::DoRun() {
  NS_LOG_FUNCTION(this);
  SetTestSuiteSeed();

  auto generator = RngGenerator<LogNormalRandomVariable>(true);
  double sum = ChiSquaredsAverage(&generator, N_RUNS);
  double maxStatistic = gsl_cdf_chisq_Qinv(0.05, N_BINS);
  NS_TEST_ASSERT_MSG_LT(sum, maxStatistic,
                        "Chi-squared statistic out of range");

  double mu = 5.0;
  double sigma = 2.0;

  Ptr<LogNormalRandomVariable> x = CreateObject<LogNormalRandomVariable>();
  x->SetAttribute("Mu", DoubleValue(mu));
  x->SetAttribute("Sigma", DoubleValue(sigma));

  x->SetAttribute("Antithetic", BooleanValue(true));

  double valueMean = Average(x);

  double expectedMean = std::exp(mu + sigma * sigma / 2.0);

  NS_TEST_ASSERT_MSG_EQ_TOL(valueMean, expectedMean, expectedMean * TOLERANCE,
                            "Wrong mean value.");
}

class GammaTestCase : public TestCaseBase {
public:
  GammaTestCase();

  double ChiSquaredTest(Ptr<RandomVariableStream> rng) const override;

private:
  void DoRun() override;

  static constexpr double TOLERANCE{1e-2};
};

GammaTestCase::GammaTestCase()
    : TestCaseBase("Gamma Random Variable Stream Generator") {}

double GammaTestCase::ChiSquaredTest(Ptr<RandomVariableStream> rng) const {
  gsl_histogram *h = gsl_histogram_alloc(N_BINS);
  auto range = UniformHistogramBins(h, 0, 10, false);

  std::vector<double> expected(N_BINS);

  double alpha = 1.0;
  double beta = 1.0;

  for (std::size_t i = 0; i < N_BINS; ++i) {
    expected[i] = gsl_cdf_gamma_P(range[i + 1], alpha, beta) -
                  gsl_cdf_gamma_P(range[i], alpha, beta);
    expected[i] *= N_MEASUREMENTS;
  }

  double chiSquared = ChiSquared(h, expected, rng);

  gsl_histogram_free(h);
  return chiSquared;
}

void GammaTestCase::DoRun() {
  NS_LOG_FUNCTION(this);
  SetTestSuiteSeed();

  auto generator = RngGenerator<GammaRandomVariable>();
  double sum = ChiSquaredsAverage(&generator, N_RUNS);
  double maxStatistic = gsl_cdf_chisq_Qinv(0.05, N_BINS);
  NS_TEST_ASSERT_MSG_LT(sum, maxStatistic,
                        "Chi-squared statistic out of range");

  double alpha = 5.0;
  double beta = 2.0;

  Ptr<GammaRandomVariable> x = CreateObject<GammaRandomVariable>();
  x->SetAttribute("Alpha", DoubleValue(alpha));
  x->SetAttribute("Beta", DoubleValue(beta));

  double valueMean = Average(x);

  double expectedMean = alpha * beta;

  NS_TEST_ASSERT_MSG_EQ_TOL(valueMean, expectedMean, expectedMean * TOLERANCE,
                            "Wrong mean value.");
}

class GammaAntitheticTestCase : public TestCaseBase {
public:
  GammaAntitheticTestCase();

  double ChiSquaredTest(Ptr<RandomVariableStream> rng) const override;

private:
  void DoRun() override;

  static constexpr double TOLERANCE{1e-2};
};

GammaAntitheticTestCase::GammaAntitheticTestCase()
    : TestCaseBase("Antithetic Gamma Random Variable Stream Generator") {}

double
GammaAntitheticTestCase::ChiSquaredTest(Ptr<RandomVariableStream> rng) const {
  gsl_histogram *h = gsl_histogram_alloc(N_BINS);
  auto range = UniformHistogramBins(h, 0, 10, false);

  std::vector<double> expected(N_BINS);

  double alpha = 1.0;
  double beta = 1.0;

  for (std::size_t i = 0; i < N_BINS; ++i) {
    expected[i] = gsl_cdf_gamma_P(range[i + 1], alpha, beta) -
                  gsl_cdf_gamma_P(range[i], alpha, beta);
    expected[i] *= N_MEASUREMENTS;
  }

  double chiSquared = ChiSquared(h, expected, rng);

  gsl_histogram_free(h);
  return chiSquared;
}

void GammaAntitheticTestCase::DoRun() {
  NS_LOG_FUNCTION(this);
  SetTestSuiteSeed();

  auto generator = RngGenerator<GammaRandomVariable>(true);
  double sum = ChiSquaredsAverage(&generator, N_RUNS);
  double maxStatistic = gsl_cdf_chisq_Qinv(0.05, N_BINS);
  NS_TEST_ASSERT_MSG_LT(sum, maxStatistic,
                        "Chi-squared statistic out of range");

  double alpha = 5.0;
  double beta = 2.0;

  Ptr<GammaRandomVariable> x = CreateObject<GammaRandomVariable>();

  x->SetAttribute("Antithetic", BooleanValue(true));

  x->SetAttribute("Alpha", DoubleValue(alpha));
  x->SetAttribute("Beta", DoubleValue(beta));

  double valueMean = Average(x);

  double expectedMean = alpha * beta;

  NS_TEST_ASSERT_MSG_EQ_TOL(valueMean, expectedMean, expectedMean * TOLERANCE,
                            "Wrong mean value.");
}

class ErlangTestCase : public TestCaseBase {
public:
  ErlangTestCase();

  double ChiSquaredTest(Ptr<RandomVariableStream> rng) const override;

private:
  void DoRun() override;

  static constexpr double TOLERANCE{1e-2};
};

ErlangTestCase::ErlangTestCase()
    : TestCaseBase("Erlang Random Variable Stream Generator") {}

double ErlangTestCase::ChiSquaredTest(Ptr<RandomVariableStream> rng) const {
  gsl_histogram *h = gsl_histogram_alloc(N_BINS);
  auto range = UniformHistogramBins(h, 0, 10, false);

  std::vector<double> expected(N_BINS);

  uint32_t k = 1;
  double lambda = 1.0;

  for (std::size_t i = 0; i < N_BINS; ++i) {
    expected[i] = gsl_cdf_gamma_P(range[i + 1], k, lambda) -
                  gsl_cdf_gamma_P(range[i], k, lambda);
    expected[i] *= N_MEASUREMENTS;
  }

  double chiSquared = ChiSquared(h, expected, rng);

  gsl_histogram_free(h);
  return chiSquared;
}

void ErlangTestCase::DoRun() {
  NS_LOG_FUNCTION(this);
  SetTestSuiteSeed();

  auto generator = RngGenerator<ErlangRandomVariable>();
  double sum = ChiSquaredsAverage(&generator, N_RUNS);
  double maxStatistic = gsl_cdf_chisq_Qinv(0.05, N_BINS);
  NS_TEST_ASSERT_MSG_LT(sum, maxStatistic,
                        "Chi-squared statistic out of range");

  uint32_t k = 5;
  double lambda = 2.0;

  Ptr<ErlangRandomVariable> x = CreateObject<ErlangRandomVariable>();
  x->SetAttribute("K", IntegerValue(k));
  x->SetAttribute("Lambda", DoubleValue(lambda));

  double valueMean = Average(x);

  double expectedMean = k * lambda;

  NS_TEST_ASSERT_MSG_EQ_TOL(valueMean, expectedMean, expectedMean * TOLERANCE,
                            "Wrong mean value.");
}

class ErlangAntitheticTestCase : public TestCaseBase {
public:
  ErlangAntitheticTestCase();

  double ChiSquaredTest(Ptr<RandomVariableStream> rng) const override;

private:
  void DoRun() override;

  static constexpr double TOLERANCE{1e-2};
};

ErlangAntitheticTestCase::ErlangAntitheticTestCase()
    : TestCaseBase("Antithetic Erlang Random Variable Stream Generator") {}

double
ErlangAntitheticTestCase::ChiSquaredTest(Ptr<RandomVariableStream> rng) const {
  gsl_histogram *h = gsl_histogram_alloc(N_BINS);
  auto range = UniformHistogramBins(h, 0, 10, false);

  std::vector<double> expected(N_BINS);

  uint32_t k = 1;
  double lambda = 1.0;

  for (std::size_t i = 0; i < N_BINS; ++i) {
    expected[i] = gsl_cdf_gamma_P(range[i + 1], k, lambda) -
                  gsl_cdf_gamma_P(range[i], k, lambda);
    expected[i] *= N_MEASUREMENTS;
  }

  double chiSquared = ChiSquared(h, expected, rng);

  gsl_histogram_free(h);
  return chiSquared;
}

void ErlangAntitheticTestCase::DoRun() {
  NS_LOG_FUNCTION(this);
  SetTestSuiteSeed();

  auto generator = RngGenerator<ErlangRandomVariable>(true);
  double sum = ChiSquaredsAverage(&generator, N_RUNS);
  double maxStatistic = gsl_cdf_chisq_Qinv(0.05, N_BINS);
  NS_TEST_ASSERT_MSG_LT(sum, maxStatistic,
                        "Chi-squared statistic out of range");

  uint32_t k = 5;
  double lambda = 2.0;

  Ptr<ErlangRandomVariable> x = CreateObject<ErlangRandomVariable>();

  x->SetAttribute("Antithetic", BooleanValue(true));

  x->SetAttribute("K", IntegerValue(k));
  x->SetAttribute("Lambda", DoubleValue(lambda));

  double valueMean = Average(x);

  double expectedMean = k * lambda;

  NS_TEST_ASSERT_MSG_EQ_TOL(valueMean, expectedMean, expectedMean * TOLERANCE,
                            "Wrong mean value.");
}

class ZipfTestCase : public TestCaseBase {
public:
  ZipfTestCase();

private:
  void DoRun() override;

  static constexpr double TOLERANCE{1e-2};
};

ZipfTestCase::ZipfTestCase()
    : TestCaseBase("Zipf Random Variable Stream Generator") {}

void ZipfTestCase::DoRun() {
  NS_LOG_FUNCTION(this);
  SetTestSuiteSeed();

  uint32_t n = 1;
  double alpha = 2.0;

  Ptr<ZipfRandomVariable> x = CreateObject<ZipfRandomVariable>();
  x->SetAttribute("N", IntegerValue(n));
  x->SetAttribute("Alpha", DoubleValue(alpha));

  double valueMean = Average(x);

  double expectedMean = 1.0;

  NS_TEST_ASSERT_MSG_EQ_TOL(valueMean, expectedMean, expectedMean * TOLERANCE,
                            "Wrong mean value.");
}

class ZipfAntitheticTestCase : public TestCaseBase {
public:
  ZipfAntitheticTestCase();

private:
  void DoRun() override;

  static constexpr double TOLERANCE{1e-2};
};

ZipfAntitheticTestCase::ZipfAntitheticTestCase()
    : TestCaseBase("Antithetic Zipf Random Variable Stream Generator") {}

void ZipfAntitheticTestCase::DoRun() {
  NS_LOG_FUNCTION(this);
  SetTestSuiteSeed();

  uint32_t n = 1;
  double alpha = 2.0;

  Ptr<ZipfRandomVariable> x = CreateObject<ZipfRandomVariable>();
  x->SetAttribute("N", IntegerValue(n));
  x->SetAttribute("Alpha", DoubleValue(alpha));

  x->SetAttribute("Antithetic", BooleanValue(true));

  double valueMean = Average(x);

  double expectedMean = 1.0;

  NS_TEST_ASSERT_MSG_EQ_TOL(valueMean, expectedMean, expectedMean * TOLERANCE,
                            "Wrong mean value.");
}

class ZetaTestCase : public TestCaseBase {
public:
  ZetaTestCase();

private:
  void DoRun() override;

  static constexpr double TOLERANCE{1e-2};
};

ZetaTestCase::ZetaTestCase()
    : TestCaseBase("Zeta Random Variable Stream Generator") {}

void ZetaTestCase::DoRun() {
  NS_LOG_FUNCTION(this);
  SetTestSuiteSeed();

  double alpha = 5.0;

  Ptr<ZetaRandomVariable> x = CreateObject<ZetaRandomVariable>();
  x->SetAttribute("Alpha", DoubleValue(alpha));

  double valueMean = Average(x);

  double expectedMean = gsl_sf_zeta_int(static_cast<int>(alpha - 1)) /
                        gsl_sf_zeta_int(static_cast<int>(alpha));

  NS_TEST_ASSERT_MSG_EQ_TOL(valueMean, expectedMean, expectedMean * TOLERANCE,
                            "Wrong mean value.");
}

class ZetaAntitheticTestCase : public TestCaseBase {
public:
  ZetaAntitheticTestCase();

private:
  void DoRun() override;

  static constexpr double TOLERANCE{1e-2};
};

ZetaAntitheticTestCase::ZetaAntitheticTestCase()
    : TestCaseBase("Antithetic Zeta Random Variable Stream Generator") {}

void ZetaAntitheticTestCase::DoRun() {
  NS_LOG_FUNCTION(this);
  SetTestSuiteSeed();

  double alpha = 5.0;

  Ptr<ZetaRandomVariable> x = CreateObject<ZetaRandomVariable>();
  x->SetAttribute("Alpha", DoubleValue(alpha));

  x->SetAttribute("Antithetic", BooleanValue(true));

  double valueMean = Average(x);

  double expectedMean = gsl_sf_zeta_int(static_cast<int>(alpha) - 1) /
                        gsl_sf_zeta_int(static_cast<int>(alpha));

  NS_TEST_ASSERT_MSG_EQ_TOL(valueMean, expectedMean, expectedMean * TOLERANCE,
                            "Wrong mean value.");
}

class DeterministicTestCase : public TestCaseBase {
public:
  DeterministicTestCase();

private:
  void DoRun() override;

  static constexpr double TOLERANCE{1e-8};
};

DeterministicTestCase::DeterministicTestCase()
    : TestCaseBase("Deterministic Random Variable Stream Generator") {}

void DeterministicTestCase::DoRun() {
  NS_LOG_FUNCTION(this);
  SetTestSuiteSeed();

  Ptr<DeterministicRandomVariable> s =
      CreateObject<DeterministicRandomVariable>();

  double array1[] = {4, 4, 7, 7, 10, 10};
  std::size_t count1 = 6;
  s->SetValueArray(array1, count1);

  double value;

  value = s->GetValue();
  NS_TEST_ASSERT_MSG_EQ_TOL(value, 4, TOLERANCE, "Sequence 1 value 1 wrong.");
  value = s->GetValue();
  NS_TEST_ASSERT_MSG_EQ_TOL(value, 4, TOLERANCE, "Sequence 1 value 2 wrong.");
  value = s->GetValue();
  NS_TEST_ASSERT_MSG_EQ_TOL(value, 7, TOLERANCE, "Sequence 1 value 3 wrong.");
  value = s->GetValue();
  NS_TEST_ASSERT_MSG_EQ_TOL(value, 7, TOLERANCE, "Sequence 1 value 4 wrong.");
  value = s->GetValue();
  NS_TEST_ASSERT_MSG_EQ_TOL(value, 10, TOLERANCE, "Sequence 1 value 5 wrong.");
  value = s->GetValue();
  NS_TEST_ASSERT_MSG_EQ_TOL(value, 10, TOLERANCE, "Sequence 1 value 6 wrong.");

  double array2[] = {1000, 2000, 3000, 4000};
  std::size_t count2 = 4;
  s->SetValueArray(array2, count2);

  value = s->GetValue();
  NS_TEST_ASSERT_MSG_EQ_TOL(value, 1000, TOLERANCE,
                            "Sequence 2 value 1 wrong.");
  value = s->GetValue();
  NS_TEST_ASSERT_MSG_EQ_TOL(value, 2000, TOLERANCE,
                            "Sequence 2 value 2 wrong.");
  value = s->GetValue();
  NS_TEST_ASSERT_MSG_EQ_TOL(value, 3000, TOLERANCE,
                            "Sequence 2 value 3 wrong.");
  value = s->GetValue();
  NS_TEST_ASSERT_MSG_EQ_TOL(value, 4000, TOLERANCE,
                            "Sequence 2 value 4 wrong.");
  value = s->GetValue();
}

class EmpiricalTestCase : public TestCaseBase {
public:
  EmpiricalTestCase();

private:
  void DoRun() override;

  static constexpr double TOLERANCE{1e-2};
};

EmpiricalTestCase::EmpiricalTestCase()
    : TestCaseBase("Empirical Random Variable Stream Generator") {}

void EmpiricalTestCase::DoRun() {
  NS_LOG_FUNCTION(this);
  SetTestSuiteSeed();

  Ptr<EmpiricalRandomVariable> x = CreateObject<EmpiricalRandomVariable>();
  x->SetInterpolate(false);
  x->CDF(0.0, 0.0);
  x->CDF(5.0, 0.25);
  x->CDF(10.0, 1.0);

  for (uint32_t i = 0; i < N_MEASUREMENTS; ++i) {
    double value = x->GetValue();
    NS_TEST_EXPECT_MSG_EQ((value == 5) || (value == 10), true,
                          "Incorrect value returned, expected only 5 or 10.");
  }

  double valueMean = Average(x);

  double expectedMean = 8.75;
  NS_TEST_ASSERT_MSG_EQ_TOL(valueMean, expectedMean, expectedMean * TOLERANCE,
                            "Wrong mean value.");

  x->SetInterpolate(true);
  valueMean = Average(x);

  expectedMean = 6.25;

  NS_TEST_ASSERT_MSG_EQ_TOL(valueMean, expectedMean, expectedMean * TOLERANCE,
                            "Wrong mean value.");

  Ptr<EmpiricalRandomVariable> y = CreateObject<EmpiricalRandomVariable>();
  y->SetInterpolate(false);
  y->CDF(-1.0, 0.0);
  y->CDF(0.0, 0.5);
  y->CDF(1.0, 1.0);
  NS_TEST_ASSERT_MSG_LT(y->GetValue(), 2,
                        "Empirical variable with negative domain");
}

class EmpiricalAntitheticTestCase : public TestCaseBase {
public:
  EmpiricalAntitheticTestCase();

private:
  void DoRun() override;

  static constexpr double TOLERANCE{1e-2};
};

EmpiricalAntitheticTestCase::EmpiricalAntitheticTestCase()
    : TestCaseBase("EmpiricalAntithetic Random Variable Stream Generator") {}

void EmpiricalAntitheticTestCase::DoRun() {
  NS_LOG_FUNCTION(this);
  SetTestSuiteSeed();

  Ptr<EmpiricalRandomVariable> x = CreateObject<EmpiricalRandomVariable>();
  x->SetInterpolate(false);
  x->CDF(0.0, 0.0);
  x->CDF(5.0, 0.25);
  x->CDF(10.0, 1.0);

  x->SetAttribute("Antithetic", BooleanValue(true));

  for (uint32_t i = 0; i < N_MEASUREMENTS; ++i) {
    double value = x->GetValue();
    NS_TEST_EXPECT_MSG_EQ((value == 5) || (value == 10), true,
                          "Incorrect value returned, expected only 5 or 10.");
  }

  double valueMean = Average(x);
  double expectedMean = 8.75;
  NS_TEST_ASSERT_MSG_EQ_TOL(valueMean, expectedMean, expectedMean * TOLERANCE,
                            "Wrong mean value.");

  x->SetInterpolate(true);
  valueMean = Average(x);

  expectedMean = 6.25;

  NS_TEST_ASSERT_MSG_EQ_TOL(valueMean, expectedMean, expectedMean * TOLERANCE,
                            "Wrong mean value.");
}

class NormalCachingTestCase : public TestCaseBase {
public:
  NormalCachingTestCase();

private:
  void DoRun() override;
};

NormalCachingTestCase::NormalCachingTestCase()
    : TestCaseBase("NormalRandomVariable caching of parameters") {}

void NormalCachingTestCase::DoRun() {
  NS_LOG_FUNCTION(this);
  SetTestSuiteSeed();

  Ptr<NormalRandomVariable> n = CreateObject<NormalRandomVariable>();
  double v1 = n->GetValue(-10, 1, 10);
  double v2 = n->GetValue(10, 1, 10);

  NS_TEST_ASSERT_MSG_LT(v1, 0, "Incorrect value returned, expected < 0");
  NS_TEST_ASSERT_MSG_GT(v2, 0, "Incorrect value returned, expected > 0");
}

class RandomVariableSuite : public TestSuite {
public:
  RandomVariableSuite();
};

RandomVariableSuite::RandomVariableSuite()
    : TestSuite("random-variable-stream-generators", UNIT) {
  AddTestCase(new UniformTestCase);
  AddTestCase(new UniformAntitheticTestCase);
  AddTestCase(new ConstantTestCase);
  AddTestCase(new SequentialTestCase);
  AddTestCase(new NormalTestCase);
  AddTestCase(new NormalAntitheticTestCase);
  AddTestCase(new ExponentialTestCase);
  AddTestCase(new ExponentialAntitheticTestCase);
  AddTestCase(new ParetoTestCase);
  AddTestCase(new ParetoAntitheticTestCase);
  AddTestCase(new WeibullTestCase);
  AddTestCase(new WeibullAntitheticTestCase);
  AddTestCase(new LogNormalTestCase);
  AddTestCase(new GammaTestCase);
  AddTestCase(new ErlangTestCase);
  AddTestCase(new ErlangAntitheticTestCase);
  AddTestCase(new ZipfTestCase);
  AddTestCase(new ZipfAntitheticTestCase);
  AddTestCase(new ZetaTestCase);
  AddTestCase(new ZetaAntitheticTestCase);
  AddTestCase(new DeterministicTestCase);
  AddTestCase(new EmpiricalTestCase);
  AddTestCase(new EmpiricalAntitheticTestCase);
  AddTestCase(new NormalCachingTestCase);
}

static RandomVariableSuite randomVariableSuite;

} // namespace RandomVariable

} // namespace test

} // namespace ns3
