#include "random-variable-stream.h"

#include "assert.h"
#include "boolean.h"
#include "double.h"
#include "integer.h"
#include "log.h"
#include "pointer.h"
#include "rng-seed-manager.h"
#include "rng-stream.h"
#include "string.h"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("RandomVariableStream");

NS_OBJECT_ENSURE_REGISTERED(RandomVariableStream);

TypeId RandomVariableStream::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::RandomVariableStream")
          .SetParent<Object>()
          .SetGroupName("Core")
          .AddAttribute("Stream",
                        "The stream number for this RNG stream. -1 means "
                        "\"allocate a stream automatically\". "
                        "Note that if -1 is set, Get will return -1 so that it "
                        "is not possible to know which "
                        "value was automatically allocated.",
                        IntegerValue(-1),
                        MakeIntegerAccessor(&RandomVariableStream::SetStream,
                                            &RandomVariableStream::GetStream),
                        MakeIntegerChecker<int64_t>())
          .AddAttribute(
              "Antithetic", "Set this RNG stream to generate antithetic values",
              BooleanValue(false),
              MakeBooleanAccessor(&RandomVariableStream::SetAntithetic,
                                  &RandomVariableStream::IsAntithetic),
              MakeBooleanChecker());
  return tid;
}

RandomVariableStream::RandomVariableStream() : m_rng(nullptr) {
  NS_LOG_FUNCTION(this);
}

RandomVariableStream::~RandomVariableStream() {
  NS_LOG_FUNCTION(this);
  delete m_rng;
}

void RandomVariableStream::SetAntithetic(bool isAntithetic) {
  NS_LOG_FUNCTION(this << isAntithetic);
  m_isAntithetic = isAntithetic;
}

bool RandomVariableStream::IsAntithetic() const {
  NS_LOG_FUNCTION(this);
  return m_isAntithetic;
}

uint32_t RandomVariableStream::GetInteger() {
  NS_LOG_FUNCTION(this);
  return static_cast<uint32_t>(GetValue());
}

void RandomVariableStream::SetStream(int64_t stream) {
  NS_LOG_FUNCTION(this << stream);
  NS_ASSERT(stream >= -1);
  delete m_rng;
  if (stream == -1) {
    uint64_t nextStream = RngSeedManager::GetNextStreamIndex();
    NS_ASSERT(nextStream <= ((1ULL) << 63));
    m_rng = new RngStream(RngSeedManager::GetSeed(), nextStream,
                          RngSeedManager::GetRun());
  } else {
    uint64_t base = ((1ULL) << 63);
    uint64_t target = base + stream;
    m_rng = new RngStream(RngSeedManager::GetSeed(), target,
                          RngSeedManager::GetRun());
  }
  m_stream = stream;
}

int64_t RandomVariableStream::GetStream() const {
  NS_LOG_FUNCTION(this);
  return m_stream;
}

RngStream *RandomVariableStream::Peek() const {
  NS_LOG_FUNCTION(this);
  return m_rng;
}

NS_OBJECT_ENSURE_REGISTERED(UniformRandomVariable);

TypeId UniformRandomVariable::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::UniformRandomVariable")
          .SetParent<RandomVariableStream>()
          .SetGroupName("Core")
          .AddConstructor<UniformRandomVariable>()
          .AddAttribute(
              "Min",
              "The lower bound on the values returned by this RNG stream.",
              DoubleValue(0), MakeDoubleAccessor(&UniformRandomVariable::m_min),
              MakeDoubleChecker<double>())
          .AddAttribute(
              "Max",
              "The upper bound on the values returned by this RNG stream.",
              DoubleValue(1.0),
              MakeDoubleAccessor(&UniformRandomVariable::m_max),
              MakeDoubleChecker<double>());
  return tid;
}

UniformRandomVariable::UniformRandomVariable() { NS_LOG_FUNCTION(this); }

double UniformRandomVariable::GetMin() const {
  NS_LOG_FUNCTION(this);
  return m_min;
}

double UniformRandomVariable::GetMax() const {
  NS_LOG_FUNCTION(this);
  return m_max;
}

double UniformRandomVariable::GetValue(double min, double max) {
  NS_LOG_FUNCTION(this << min << max);
  double v = min + Peek()->RandU01() * (max - min);
  if (IsAntithetic()) {
    v = min + (max - v);
  }
  return v;
}

uint32_t UniformRandomVariable::GetInteger(uint32_t min, uint32_t max) {
  NS_LOG_FUNCTION(this << min << max);
  NS_ASSERT(min <= max);
  return static_cast<uint32_t>(GetValue((double)(min), (double)(max) + 1.0));
}

double UniformRandomVariable::GetValue() {
  NS_LOG_FUNCTION(this);
  return GetValue(m_min, m_max);
}

uint32_t UniformRandomVariable::GetInteger() {
  NS_LOG_FUNCTION(this);
  return static_cast<uint32_t>(GetValue(m_min, m_max + 1));
}

NS_OBJECT_ENSURE_REGISTERED(ConstantRandomVariable);

TypeId ConstantRandomVariable::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::ConstantRandomVariable")
          .SetParent<RandomVariableStream>()
          .SetGroupName("Core")
          .AddConstructor<ConstantRandomVariable>()
          .AddAttribute("Constant",
                        "The constant value returned by this RNG stream.",
                        DoubleValue(0),
                        MakeDoubleAccessor(&ConstantRandomVariable::m_constant),
                        MakeDoubleChecker<double>());
  return tid;
}

ConstantRandomVariable::ConstantRandomVariable() { NS_LOG_FUNCTION(this); }

double ConstantRandomVariable::GetConstant() const {
  NS_LOG_FUNCTION(this);
  return m_constant;
}

double ConstantRandomVariable::GetValue(double constant) {
  NS_LOG_FUNCTION(this << constant);
  return constant;
}

uint32_t ConstantRandomVariable::GetInteger(uint32_t constant) {
  NS_LOG_FUNCTION(this << constant);
  return constant;
}

double ConstantRandomVariable::GetValue() {
  NS_LOG_FUNCTION(this);
  return GetValue(m_constant);
}

NS_OBJECT_ENSURE_REGISTERED(SequentialRandomVariable);

TypeId SequentialRandomVariable::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::SequentialRandomVariable")
          .SetParent<RandomVariableStream>()
          .SetGroupName("Core")
          .AddConstructor<SequentialRandomVariable>()
          .AddAttribute("Min", "The first value of the sequence.",
                        DoubleValue(0),
                        MakeDoubleAccessor(&SequentialRandomVariable::m_min),
                        MakeDoubleChecker<double>())
          .AddAttribute("Max", "One more than the last value of the sequence.",
                        DoubleValue(0),
                        MakeDoubleAccessor(&SequentialRandomVariable::m_max),
                        MakeDoubleChecker<double>())
          .AddAttribute(
              "Increment", "The sequence random variable increment.",
              StringValue("ns3::ConstantRandomVariable[Constant=1]"),
              MakePointerAccessor(&SequentialRandomVariable::m_increment),
              MakePointerChecker<RandomVariableStream>())
          .AddAttribute(
              "Consecutive",
              "The number of times each member of the sequence is repeated.",
              IntegerValue(1),
              MakeIntegerAccessor(&SequentialRandomVariable::m_consecutive),
              MakeIntegerChecker<uint32_t>());
  return tid;
}

SequentialRandomVariable::SequentialRandomVariable()
    : m_current(0), m_currentConsecutive(0), m_isCurrentSet(false) {
  NS_LOG_FUNCTION(this);
}

double SequentialRandomVariable::GetMin() const {
  NS_LOG_FUNCTION(this);
  return m_min;
}

double SequentialRandomVariable::GetMax() const {
  NS_LOG_FUNCTION(this);
  return m_max;
}

Ptr<RandomVariableStream> SequentialRandomVariable::GetIncrement() const {
  NS_LOG_FUNCTION(this);
  return m_increment;
}

uint32_t SequentialRandomVariable::GetConsecutive() const {
  NS_LOG_FUNCTION(this);
  return m_consecutive;
}

double SequentialRandomVariable::GetValue() {
  NS_LOG_FUNCTION(this);
  if (!m_isCurrentSet) {
    m_current = m_min;
    m_isCurrentSet = true;
  }

  double r = m_current;
  if (++m_currentConsecutive == m_consecutive) {
    m_currentConsecutive = 0;
    m_current += m_increment->GetValue();
    if (m_current >= m_max) {
      m_current = m_min + (m_current - m_max);
    }
  }
  return r;
}

NS_OBJECT_ENSURE_REGISTERED(ExponentialRandomVariable);

TypeId ExponentialRandomVariable::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::ExponentialRandomVariable")
          .SetParent<RandomVariableStream>()
          .SetGroupName("Core")
          .AddConstructor<ExponentialRandomVariable>()
          .AddAttribute("Mean",
                        "The mean of the values returned by this RNG stream.",
                        DoubleValue(1.0),
                        MakeDoubleAccessor(&ExponentialRandomVariable::m_mean),
                        MakeDoubleChecker<double>())
          .AddAttribute(
              "Bound",
              "The upper bound on the values returned by this RNG stream.",
              DoubleValue(0.0),
              MakeDoubleAccessor(&ExponentialRandomVariable::m_bound),
              MakeDoubleChecker<double>());
  return tid;
}

ExponentialRandomVariable::ExponentialRandomVariable() {
  NS_LOG_FUNCTION(this);
}

double ExponentialRandomVariable::GetMean() const {
  NS_LOG_FUNCTION(this);
  return m_mean;
}

double ExponentialRandomVariable::GetBound() const {
  NS_LOG_FUNCTION(this);
  return m_bound;
}

double ExponentialRandomVariable::GetValue(double mean, double bound) {
  NS_LOG_FUNCTION(this << mean << bound);
  while (true) {
    double v = Peek()->RandU01();
    if (IsAntithetic()) {
      v = (1 - v);
    }

    double r = -mean * std::log(v);

    if (bound == 0 || r <= bound) {
      return r;
    }
  }
}

uint32_t ExponentialRandomVariable::GetInteger(uint32_t mean, uint32_t bound) {
  NS_LOG_FUNCTION(this << mean << bound);
  return static_cast<uint32_t>(GetValue(mean, bound));
}

double ExponentialRandomVariable::GetValue() {
  NS_LOG_FUNCTION(this);
  return GetValue(m_mean, m_bound);
}

NS_OBJECT_ENSURE_REGISTERED(ParetoRandomVariable);

TypeId ParetoRandomVariable::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::ParetoRandomVariable")
          .SetParent<RandomVariableStream>()
          .SetGroupName("Core")
          .AddConstructor<ParetoRandomVariable>()
          .AddAttribute("Scale",
                        "The scale parameter for the Pareto distribution "
                        "returned by this RNG stream.",
                        DoubleValue(1.0),
                        MakeDoubleAccessor(&ParetoRandomVariable::m_scale),
                        MakeDoubleChecker<double>())
          .AddAttribute("Shape",
                        "The shape parameter for the Pareto distribution "
                        "returned by this RNG stream.",
                        DoubleValue(2.0),
                        MakeDoubleAccessor(&ParetoRandomVariable::m_shape),
                        MakeDoubleChecker<double>())
          .AddAttribute("Bound",
                        "The upper bound on the values returned by this RNG "
                        "stream (if non-zero).",
                        DoubleValue(0.0),
                        MakeDoubleAccessor(&ParetoRandomVariable::m_bound),
                        MakeDoubleChecker<double>());
  return tid;
}

ParetoRandomVariable::ParetoRandomVariable() { NS_LOG_FUNCTION(this); }

double ParetoRandomVariable::GetScale() const {
  NS_LOG_FUNCTION(this);
  return m_scale;
}

double ParetoRandomVariable::GetShape() const {
  NS_LOG_FUNCTION(this);
  return m_shape;
}

double ParetoRandomVariable::GetBound() const {
  NS_LOG_FUNCTION(this);
  return m_bound;
}

double ParetoRandomVariable::GetValue(double scale, double shape,
                                      double bound) {
  NS_LOG_FUNCTION(this << scale << shape << bound);

  while (true) {
    double v = Peek()->RandU01();
    if (IsAntithetic()) {
      v = (1 - v);
    }

    double r = (scale * (1.0 / std::pow(v, 1.0 / shape)));

    if (bound == 0 || r <= bound) {
      return r;
    }
  }
}

uint32_t ParetoRandomVariable::GetInteger(uint32_t scale, uint32_t shape,
                                          uint32_t bound) {
  NS_LOG_FUNCTION(this << scale << shape << bound);
  return static_cast<uint32_t>(GetValue(scale, shape, bound));
}

double ParetoRandomVariable::GetValue() {
  NS_LOG_FUNCTION(this);
  return GetValue(m_scale, m_shape, m_bound);
}

NS_OBJECT_ENSURE_REGISTERED(WeibullRandomVariable);

TypeId WeibullRandomVariable::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::WeibullRandomVariable")
          .SetParent<RandomVariableStream>()
          .SetGroupName("Core")
          .AddConstructor<WeibullRandomVariable>()
          .AddAttribute("Scale",
                        "The scale parameter for the Weibull distribution "
                        "returned by this RNG stream.",
                        DoubleValue(1.0),
                        MakeDoubleAccessor(&WeibullRandomVariable::m_scale),
                        MakeDoubleChecker<double>())
          .AddAttribute("Shape",
                        "The shape parameter for the Weibull distribution "
                        "returned by this RNG stream.",
                        DoubleValue(1),
                        MakeDoubleAccessor(&WeibullRandomVariable::m_shape),
                        MakeDoubleChecker<double>())
          .AddAttribute(
              "Bound",
              "The upper bound on the values returned by this RNG stream.",
              DoubleValue(0.0),
              MakeDoubleAccessor(&WeibullRandomVariable::m_bound),
              MakeDoubleChecker<double>());
  return tid;
}

WeibullRandomVariable::WeibullRandomVariable() { NS_LOG_FUNCTION(this); }

double WeibullRandomVariable::GetScale() const {
  NS_LOG_FUNCTION(this);
  return m_scale;
}

double WeibullRandomVariable::GetShape() const {
  NS_LOG_FUNCTION(this);
  return m_shape;
}

double WeibullRandomVariable::GetBound() const {
  NS_LOG_FUNCTION(this);
  return m_bound;
}

double WeibullRandomVariable::GetValue(double scale, double shape,
                                       double bound) {
  NS_LOG_FUNCTION(this << scale << shape << bound);
  double exponent = 1.0 / shape;
  while (true) {
    double v = Peek()->RandU01();
    if (IsAntithetic()) {
      v = (1 - v);
    }

    double r = scale * std::pow(-std::log(v), exponent);

    if (bound == 0 || r <= bound) {
      return r;
    }
  }
}

uint32_t WeibullRandomVariable::GetInteger(uint32_t scale, uint32_t shape,
                                           uint32_t bound) {
  NS_LOG_FUNCTION(this << scale << shape << bound);
  return static_cast<uint32_t>(GetValue(scale, shape, bound));
}

double WeibullRandomVariable::GetValue() {
  NS_LOG_FUNCTION(this);
  return GetValue(m_scale, m_shape, m_bound);
}

NS_OBJECT_ENSURE_REGISTERED(NormalRandomVariable);

const double NormalRandomVariable::INFINITE_VALUE = 1e307;

TypeId NormalRandomVariable::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::NormalRandomVariable")
          .SetParent<RandomVariableStream>()
          .SetGroupName("Core")
          .AddConstructor<NormalRandomVariable>()
          .AddAttribute("Mean",
                        "The mean value for the normal distribution returned "
                        "by this RNG stream.",
                        DoubleValue(0.0),
                        MakeDoubleAccessor(&NormalRandomVariable::m_mean),
                        MakeDoubleChecker<double>())
          .AddAttribute("Variance",
                        "The variance value for the normal distribution "
                        "returned by this RNG stream.",
                        DoubleValue(1.0),
                        MakeDoubleAccessor(&NormalRandomVariable::m_variance),
                        MakeDoubleChecker<double>())
          .AddAttribute("Bound",
                        "The bound on the values returned by this RNG stream.",
                        DoubleValue(INFINITE_VALUE),
                        MakeDoubleAccessor(&NormalRandomVariable::m_bound),
                        MakeDoubleChecker<double>());
  return tid;
}

NormalRandomVariable::NormalRandomVariable() : m_nextValid(false) {
  NS_LOG_FUNCTION(this);
}

double NormalRandomVariable::GetMean() const {
  NS_LOG_FUNCTION(this);
  return m_mean;
}

double NormalRandomVariable::GetVariance() const {
  NS_LOG_FUNCTION(this);
  return m_variance;
}

double NormalRandomVariable::GetBound() const {
  NS_LOG_FUNCTION(this);
  return m_bound;
}

double NormalRandomVariable::GetValue(double mean, double variance,
                                      double bound) {
  NS_LOG_FUNCTION(this << mean << variance << bound);
  if (m_nextValid) {
    m_nextValid = false;
    double x2 = mean + m_v2 * m_y * std::sqrt(variance);
    if (std::fabs(x2 - mean) <= bound) {
      return x2;
    }
  }
  while (true) {
    double u1 = Peek()->RandU01();
    double u2 = Peek()->RandU01();
    if (IsAntithetic()) {
      u1 = (1 - u1);
      u2 = (1 - u2);
    }
    double v1 = 2 * u1 - 1;
    double v2 = 2 * u2 - 1;
    double w = v1 * v1 + v2 * v2;
    if (w <= 1.0) {
      double y = std::sqrt((-2 * std::log(w)) / w);
      double x1 = mean + v1 * y * std::sqrt(variance);
      if (std::fabs(x1 - mean) <= bound) {
        m_nextValid = true;
        m_y = y;
        m_v2 = v2;
        return x1;
      }
      double x2 = mean + v2 * y * std::sqrt(variance);
      if (std::fabs(x2 - mean) <= bound) {
        m_nextValid = false;
        return x2;
      }
    }
  }
}

uint32_t NormalRandomVariable::GetInteger(uint32_t mean, uint32_t variance,
                                          uint32_t bound) {
  NS_LOG_FUNCTION(this << mean << variance << bound);
  return static_cast<uint32_t>(GetValue(mean, variance, bound));
}

double NormalRandomVariable::GetValue() {
  NS_LOG_FUNCTION(this);
  return GetValue(m_mean, m_variance, m_bound);
}

NS_OBJECT_ENSURE_REGISTERED(LogNormalRandomVariable);

TypeId LogNormalRandomVariable::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::LogNormalRandomVariable")
          .SetParent<RandomVariableStream>()
          .SetGroupName("Core")
          .AddConstructor<LogNormalRandomVariable>()
          .AddAttribute("Mu",
                        "The mu value for the log-normal distribution returned "
                        "by this RNG stream.",
                        DoubleValue(0.0),
                        MakeDoubleAccessor(&LogNormalRandomVariable::m_mu),
                        MakeDoubleChecker<double>())
          .AddAttribute("Sigma",
                        "The sigma value for the log-normal distribution "
                        "returned by this RNG stream.",
                        DoubleValue(1.0),
                        MakeDoubleAccessor(&LogNormalRandomVariable::m_sigma),
                        MakeDoubleChecker<double>());
  return tid;
}

LogNormalRandomVariable::LogNormalRandomVariable() : m_nextValid(false) {
  NS_LOG_FUNCTION(this);
}

double LogNormalRandomVariable::GetMu() const {
  NS_LOG_FUNCTION(this);
  return m_mu;
}

double LogNormalRandomVariable::GetSigma() const {
  NS_LOG_FUNCTION(this);
  return m_sigma;
}

double LogNormalRandomVariable::GetValue(double mu, double sigma) {
  if (m_nextValid) {
    m_nextValid = false;
    double normal = m_v2 * m_normal;

    return std::exp(sigma * normal + mu);
  }

  double v1;
  double v2;
  double r2;
  double normal;
  double x;

  NS_LOG_FUNCTION(this << mu << sigma);

  do {

    double u1 = Peek()->RandU01();
    double u2 = Peek()->RandU01();
    if (IsAntithetic()) {
      u1 = (1 - u1);
      u2 = (1 - u2);
    }

    v1 = -1 + 2 * u1;
    v2 = -1 + 2 * u2;

    r2 = v1 * v1 + v2 * v2;
  } while (r2 > 1.0 || r2 == 0);

  m_normal = std::sqrt(-2.0 * std::log(r2) / r2);
  normal = v1 * m_normal;
  m_nextValid = true;
  m_v2 = v2;

  x = std::exp(sigma * normal + mu);

  return x;
}

uint32_t LogNormalRandomVariable::GetInteger(uint32_t mu, uint32_t sigma) {
  NS_LOG_FUNCTION(this << mu << sigma);
  return static_cast<uint32_t>(GetValue(mu, sigma));
}

double LogNormalRandomVariable::GetValue() {
  NS_LOG_FUNCTION(this);
  return GetValue(m_mu, m_sigma);
}

NS_OBJECT_ENSURE_REGISTERED(GammaRandomVariable);

TypeId GammaRandomVariable::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::GammaRandomVariable")
          .SetParent<RandomVariableStream>()
          .SetGroupName("Core")
          .AddConstructor<GammaRandomVariable>()
          .AddAttribute("Alpha",
                        "The alpha value for the gamma distribution returned "
                        "by this RNG stream.",
                        DoubleValue(1.0),
                        MakeDoubleAccessor(&GammaRandomVariable::m_alpha),
                        MakeDoubleChecker<double>())
          .AddAttribute("Beta",
                        "The beta value for the gamma distribution returned by "
                        "this RNG stream.",
                        DoubleValue(1.0),
                        MakeDoubleAccessor(&GammaRandomVariable::m_beta),
                        MakeDoubleChecker<double>());
  return tid;
}

GammaRandomVariable::GammaRandomVariable() : m_nextValid(false) {
  NS_LOG_FUNCTION(this);
}

double GammaRandomVariable::GetAlpha() const {
  NS_LOG_FUNCTION(this);
  return m_alpha;
}

double GammaRandomVariable::GetBeta() const {
  NS_LOG_FUNCTION(this);
  return m_beta;
}

double GammaRandomVariable::GetValue(double alpha, double beta) {
  NS_LOG_FUNCTION(this << alpha << beta);
  if (alpha < 1) {
    double u = Peek()->RandU01();
    if (IsAntithetic()) {
      u = (1 - u);
    }
    return GetValue(1.0 + alpha, beta) * std::pow(u, 1.0 / alpha);
  }

  double x;
  double v;
  double u;
  double d = alpha - 1.0 / 3.0;
  double c = (1.0 / 3.0) / std::sqrt(d);

  while (true) {
    do {
      double mean = 0.0;
      double variance = 1.0;
      double bound = NormalRandomVariable::INFINITE_VALUE;
      x = GetNormalValue(mean, variance, bound);

      v = 1.0 + c * x;
    } while (v <= 0);

    v = v * v * v;
    u = Peek()->RandU01();
    if (IsAntithetic()) {
      u = (1 - u);
    }
    if (u < 1 - 0.0331 * x * x * x * x) {
      break;
    }
    if (std::log(u) < 0.5 * x * x + d * (1 - v + std::log(v))) {
      break;
    }
  }

  return beta * d * v;
}

double GammaRandomVariable::GetValue() {
  NS_LOG_FUNCTION(this);
  return GetValue(m_alpha, m_beta);
}

double GammaRandomVariable::GetNormalValue(double mean, double variance,
                                           double bound) {
  NS_LOG_FUNCTION(this << mean << variance << bound);
  if (m_nextValid) {
    m_nextValid = false;
    double x2 = mean + m_v2 * m_y * std::sqrt(variance);
    if (std::fabs(x2 - mean) <= bound) {
      return x2;
    }
  }
  while (true) {
    double u1 = Peek()->RandU01();
    double u2 = Peek()->RandU01();
    if (IsAntithetic()) {
      u1 = (1 - u1);
      u2 = (1 - u2);
    }
    double v1 = 2 * u1 - 1;
    double v2 = 2 * u2 - 1;
    double w = v1 * v1 + v2 * v2;
    if (w <= 1.0) {
      double y = std::sqrt((-2 * std::log(w)) / w);
      double x1 = mean + v1 * y * std::sqrt(variance);
      if (std::fabs(x1 - mean) <= bound) {
        m_nextValid = true;
        m_y = y;
        m_v2 = v2;
        return x1;
      }
      double x2 = mean + v2 * y * std::sqrt(variance);
      if (std::fabs(x2 - mean) <= bound) {
        m_nextValid = false;
        return x2;
      }
    }
  }
}

NS_OBJECT_ENSURE_REGISTERED(ErlangRandomVariable);

TypeId ErlangRandomVariable::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::ErlangRandomVariable")
          .SetParent<RandomVariableStream>()
          .SetGroupName("Core")
          .AddConstructor<ErlangRandomVariable>()
          .AddAttribute("K",
                        "The k value for the Erlang distribution returned by "
                        "this RNG stream.",
                        IntegerValue(1),
                        MakeIntegerAccessor(&ErlangRandomVariable::m_k),
                        MakeIntegerChecker<uint32_t>())
          .AddAttribute("Lambda",
                        "The lambda value for the Erlang distribution returned "
                        "by this RNG stream.",
                        DoubleValue(1.0),
                        MakeDoubleAccessor(&ErlangRandomVariable::m_lambda),
                        MakeDoubleChecker<double>());
  return tid;
}

ErlangRandomVariable::ErlangRandomVariable() { NS_LOG_FUNCTION(this); }

uint32_t ErlangRandomVariable::GetK() const {
  NS_LOG_FUNCTION(this);
  return m_k;
}

double ErlangRandomVariable::GetLambda() const {
  NS_LOG_FUNCTION(this);
  return m_lambda;
}

double ErlangRandomVariable::GetValue(uint32_t k, double lambda) {
  NS_LOG_FUNCTION(this << k << lambda);
  double mean = lambda;
  double bound = 0.0;

  double result = 0;
  for (unsigned int i = 0; i < k; ++i) {
    result += GetExponentialValue(mean, bound);
  }

  return result;
}

uint32_t ErlangRandomVariable::GetInteger(uint32_t k, uint32_t lambda) {
  NS_LOG_FUNCTION(this << k << lambda);
  return static_cast<uint32_t>(GetValue(k, lambda));
}

double ErlangRandomVariable::GetValue() {
  NS_LOG_FUNCTION(this);
  return GetValue(m_k, m_lambda);
}

double ErlangRandomVariable::GetExponentialValue(double mean, double bound) {
  NS_LOG_FUNCTION(this << mean << bound);
  while (true) {
    double v = Peek()->RandU01();
    if (IsAntithetic()) {
      v = (1 - v);
    }

    double r = -mean * std::log(v);

    if (bound == 0 || r <= bound) {
      return r;
    }
  }
}

NS_OBJECT_ENSURE_REGISTERED(TriangularRandomVariable);

TypeId TriangularRandomVariable::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::TriangularRandomVariable")
          .SetParent<RandomVariableStream>()
          .SetGroupName("Core")
          .AddConstructor<TriangularRandomVariable>()
          .AddAttribute("Mean",
                        "The mean value for the triangular distribution "
                        "returned by this RNG stream.",
                        DoubleValue(0.5),
                        MakeDoubleAccessor(&TriangularRandomVariable::m_mean),
                        MakeDoubleChecker<double>())
          .AddAttribute(
              "Min",
              "The lower bound on the values returned by this RNG stream.",
              DoubleValue(0.0),
              MakeDoubleAccessor(&TriangularRandomVariable::m_min),
              MakeDoubleChecker<double>())
          .AddAttribute(
              "Max",
              "The upper bound on the values returned by this RNG stream.",
              DoubleValue(1.0),
              MakeDoubleAccessor(&TriangularRandomVariable::m_max),
              MakeDoubleChecker<double>());
  return tid;
}

TriangularRandomVariable::TriangularRandomVariable() { NS_LOG_FUNCTION(this); }

double TriangularRandomVariable::GetMean() const {
  NS_LOG_FUNCTION(this);
  return m_mean;
}

double TriangularRandomVariable::GetMin() const {
  NS_LOG_FUNCTION(this);
  return m_min;
}

double TriangularRandomVariable::GetMax() const {
  NS_LOG_FUNCTION(this);
  return m_max;
}

double TriangularRandomVariable::GetValue(double mean, double min, double max) {
  NS_LOG_FUNCTION(this << mean << min << max);
  double mode = 3.0 * mean - min - max;

  double u = Peek()->RandU01();
  if (IsAntithetic()) {
    u = (1 - u);
  }

  if (u <= (mode - min) / (max - min)) {
    return min + std::sqrt(u * (max - min) * (mode - min));
  } else {
    return max - std::sqrt((1 - u) * (max - min) * (max - mode));
  }
}

uint32_t TriangularRandomVariable::GetInteger(uint32_t mean, uint32_t min,
                                              uint32_t max) {
  NS_LOG_FUNCTION(this << mean << min << max);
  return static_cast<uint32_t>(GetValue(mean, min, max));
}

double TriangularRandomVariable::GetValue() {
  NS_LOG_FUNCTION(this);
  return GetValue(m_mean, m_min, m_max);
}

NS_OBJECT_ENSURE_REGISTERED(ZipfRandomVariable);

TypeId ZipfRandomVariable::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::ZipfRandomVariable")
          .SetParent<RandomVariableStream>()
          .SetGroupName("Core")
          .AddConstructor<ZipfRandomVariable>()
          .AddAttribute("N",
                        "The n value for the Zipf distribution returned by "
                        "this RNG stream.",
                        IntegerValue(1),
                        MakeIntegerAccessor(&ZipfRandomVariable::m_n),
                        MakeIntegerChecker<uint32_t>())
          .AddAttribute("Alpha",
                        "The alpha value for the Zipf distribution returned by "
                        "this RNG stream.",
                        DoubleValue(0.0),
                        MakeDoubleAccessor(&ZipfRandomVariable::m_alpha),
                        MakeDoubleChecker<double>());
  return tid;
}

ZipfRandomVariable::ZipfRandomVariable() { NS_LOG_FUNCTION(this); }

uint32_t ZipfRandomVariable::GetN() const {
  NS_LOG_FUNCTION(this);
  return m_n;
}

double ZipfRandomVariable::GetAlpha() const {
  NS_LOG_FUNCTION(this);
  return m_alpha;
}

double ZipfRandomVariable::GetValue(uint32_t n, double alpha) {
  NS_LOG_FUNCTION(this << n << alpha);
  m_c = 0.0;
  for (uint32_t i = 1; i <= n; i++) {
    m_c += (1.0 / std::pow((double)i, alpha));
  }
  m_c = 1.0 / m_c;

  double u = Peek()->RandU01();
  if (IsAntithetic()) {
    u = (1 - u);
  }

  double sum_prob = 0;
  double zipf_value = 0;
  for (uint32_t i = 1; i <= m_n; i++) {
    sum_prob += m_c / std::pow((double)i, m_alpha);
    if (sum_prob > u) {
      zipf_value = i;
      break;
    }
  }
  return zipf_value;
}

uint32_t ZipfRandomVariable::GetInteger(uint32_t n, uint32_t alpha) {
  NS_LOG_FUNCTION(this << n << alpha);
  return static_cast<uint32_t>(GetValue(n, alpha));
}

double ZipfRandomVariable::GetValue() {
  NS_LOG_FUNCTION(this);
  return GetValue(m_n, m_alpha);
}

NS_OBJECT_ENSURE_REGISTERED(ZetaRandomVariable);

TypeId ZetaRandomVariable::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::ZetaRandomVariable")
          .SetParent<RandomVariableStream>()
          .SetGroupName("Core")
          .AddConstructor<ZetaRandomVariable>()
          .AddAttribute("Alpha",
                        "The alpha value for the zeta distribution returned by "
                        "this RNG stream.",
                        DoubleValue(3.14),
                        MakeDoubleAccessor(&ZetaRandomVariable::m_alpha),
                        MakeDoubleChecker<double>());
  return tid;
}

ZetaRandomVariable::ZetaRandomVariable() { NS_LOG_FUNCTION(this); }

double ZetaRandomVariable::GetAlpha() const {
  NS_LOG_FUNCTION(this);
  return m_alpha;
}

double ZetaRandomVariable::GetValue(double alpha) {
  NS_LOG_FUNCTION(this << alpha);
  m_b = std::pow(2.0, alpha - 1.0);

  double u;
  double v;
  double X;
  double T;
  double test;

  do {
    u = Peek()->RandU01();
    if (IsAntithetic()) {
      u = (1 - u);
    }

    v = Peek()->RandU01();
    if (IsAntithetic()) {
      v = (1 - v);
    }

    X = std::floor(std::pow(u, -1.0 / (m_alpha - 1.0)));
    T = std::pow(1.0 + 1.0 / X, m_alpha - 1.0);
    test = v * X * (T - 1.0) / (m_b - 1.0);
  } while (test > (T / m_b));

  return X;
}

uint32_t ZetaRandomVariable::GetInteger(uint32_t alpha) {
  NS_LOG_FUNCTION(this << alpha);
  return static_cast<uint32_t>(GetValue(alpha));
}

double ZetaRandomVariable::GetValue() {
  NS_LOG_FUNCTION(this);
  return GetValue(m_alpha);
}

NS_OBJECT_ENSURE_REGISTERED(DeterministicRandomVariable);

TypeId DeterministicRandomVariable::GetTypeId() {
  static TypeId tid = TypeId("ns3::DeterministicRandomVariable")
                          .SetParent<RandomVariableStream>()
                          .SetGroupName("Core")
                          .AddConstructor<DeterministicRandomVariable>();
  return tid;
}

DeterministicRandomVariable::DeterministicRandomVariable()
    : m_count(0), m_next(0), m_data(nullptr) {
  NS_LOG_FUNCTION(this);
}

DeterministicRandomVariable::~DeterministicRandomVariable() {
  NS_LOG_FUNCTION(this);
  if (m_data != nullptr) {
    delete[] m_data;
  }
}

void DeterministicRandomVariable::SetValueArray(
    const std::vector<double> &values) {
  SetValueArray(values.data(), values.size());
}

void DeterministicRandomVariable::SetValueArray(const double *values,
                                                std::size_t length) {
  NS_LOG_FUNCTION(this << values << length);
  if (m_data != nullptr) {
    delete[] m_data;
  }

  m_data = new double[length];
  m_count = length;
  m_next = length;

  for (std::size_t i = 0; i < m_count; i++) {
    m_data[i] = values[i];
  }
}

double DeterministicRandomVariable::GetValue() {
  NS_LOG_FUNCTION(this);
  NS_ASSERT(m_count > 0);

  if (m_next == m_count) {
    m_next = 0;
  }
  return m_data[m_next++];
}

NS_OBJECT_ENSURE_REGISTERED(EmpiricalRandomVariable);

TypeId EmpiricalRandomVariable::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::EmpiricalRandomVariable")
          .SetParent<RandomVariableStream>()
          .SetGroupName("Core")
          .AddConstructor<EmpiricalRandomVariable>()
          .AddAttribute(
              "Interpolate",
              "Treat the CDF as a smooth distribution and interpolate, "
              "default is to treat the CDF as a histogram and sample.",
              BooleanValue(false),
              MakeBooleanAccessor(&EmpiricalRandomVariable::m_interpolate),
              MakeBooleanChecker());
  return tid;
}

EmpiricalRandomVariable::EmpiricalRandomVariable() : m_validated(false) {
  NS_LOG_FUNCTION(this);
}

bool EmpiricalRandomVariable::SetInterpolate(bool interpolate) {
  NS_LOG_FUNCTION(this << interpolate);
  bool prev = m_interpolate;
  m_interpolate = interpolate;
  return prev;
}

bool EmpiricalRandomVariable::PreSample(double &value) {
  NS_LOG_FUNCTION(this);

  if (!m_validated) {
    Validate();
  }

  double r = Peek()->RandU01();
  if (IsAntithetic()) {
    r = (1 - r);
  }

  value = r;
  bool valid = false;
  if (r <= m_empCdf.begin()->first) {
    value = m_empCdf.begin()->second;
    valid = true;
  } else if (r >= m_empCdf.rbegin()->first) {
    value = m_empCdf.rbegin()->second;
    valid = true;
  }
  return valid;
}

double EmpiricalRandomVariable::GetValue() {
  NS_LOG_FUNCTION(this);

  double value;
  if (PreSample(value)) {
    return value;
  }

  if (m_interpolate) {
    value = DoInterpolate(value);
  } else {
    value = DoSampleCDF(value);
  }
  return value;
}

double EmpiricalRandomVariable::DoSampleCDF(double r) {
  NS_LOG_FUNCTION(this << r);

  auto bound = m_empCdf.upper_bound(r);

  return bound->second;
}

double EmpiricalRandomVariable::Interpolate() {
  NS_LOG_FUNCTION(this);

  double value;
  if (PreSample(value)) {
    return value;
  }

  value = DoInterpolate(value);
  return value;
}

double EmpiricalRandomVariable::DoInterpolate(double r) {
  NS_LOG_FUNCTION(this << r);

  auto upper = m_empCdf.upper_bound(r);
  auto lower = std::prev(upper, 1);

  if (upper == m_empCdf.begin()) {
    lower = upper;
  }

  double c1 = lower->first;
  double c2 = upper->first;
  double v1 = lower->second;
  double v2 = upper->second;

  double value = (v1 + ((v2 - v1) / (c2 - c1)) * (r - c1));
  return value;
}

void EmpiricalRandomVariable::CDF(double v, double c) {
  NS_LOG_FUNCTION(this << v << c);

  auto vPrevious = m_empCdf.find(c);

  if (vPrevious != m_empCdf.end()) {
    NS_LOG_WARN("Empirical CDF already has a value "
                << vPrevious->second << " for CDF " << c
                << ". Overwriting it with value " << v << ".");
  }

  m_empCdf[c] = v;
}

void EmpiricalRandomVariable::Validate() {
  NS_LOG_FUNCTION(this);

  if (m_empCdf.empty()) {
    NS_FATAL_ERROR("CDF is not initialized");
  }

  double vPrev = m_empCdf.begin()->second;

  for (const auto &cdfPair : m_empCdf) {
    const auto &vCurr = cdfPair.second;

    if (vCurr < vPrev) {
      NS_FATAL_ERROR(
          "Empirical distribution has decreasing CDF values. Current CDF: "
          << vCurr << ", prior CDF: " << vPrev);
    }

    vPrev = vCurr;
  }

  auto firstCdfPair = m_empCdf.begin();
  auto lastCdfPair = m_empCdf.rbegin();

  if (firstCdfPair->first < 0.0) {
    NS_FATAL_ERROR("Empirical distribution has invalid first CDF value. CDF: "
                   << firstCdfPair->first
                   << ", Value: " << firstCdfPair->second);
  }

  if (lastCdfPair->first > 1.0) {
    NS_FATAL_ERROR("Empirical distribution has invalid last CDF value. CDF: "
                   << lastCdfPair->first << ", Value: " << lastCdfPair->second);
  }

  m_validated = true;
}

} // namespace ns3
