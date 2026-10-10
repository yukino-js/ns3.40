#ifndef RANDOM_VARIABLE_STREAM_H
#define RANDOM_VARIABLE_STREAM_H

#include "attribute-helper.h"
#include "object.h"
#include "type-id.h"

#include <map>
#include <stdint.h>

namespace ns3 {

class RngStream;

class RandomVariableStream : public Object {
public:
  static TypeId GetTypeId();
  RandomVariableStream();
  ~RandomVariableStream() override;

  RandomVariableStream(const RandomVariableStream &) = delete;
  RandomVariableStream &operator=(const RandomVariableStream &) = delete;

  void SetStream(int64_t stream);

  int64_t GetStream() const;

  void SetAntithetic(bool isAntithetic);

  bool IsAntithetic() const;

  virtual double GetValue() = 0;

  virtual uint32_t GetInteger();

protected:
  RngStream *Peek() const;

private:
  RngStream *m_rng;

  bool m_isAntithetic;

  int64_t m_stream;
};

class UniformRandomVariable : public RandomVariableStream {
public:
  static TypeId GetTypeId();

  UniformRandomVariable();

  double GetMin() const;

  double GetMax() const;

  double GetValue(double min, double max);

  uint32_t GetInteger(uint32_t min, uint32_t max);

  double GetValue() override;

  uint32_t GetInteger() override;

private:
  double m_min;

  double m_max;
};

class ConstantRandomVariable : public RandomVariableStream {
public:
  static TypeId GetTypeId();

  ConstantRandomVariable();

  double GetConstant() const;

  double GetValue(double constant);
  uint32_t GetInteger(uint32_t constant);

  double GetValue() override;
  using RandomVariableStream::GetInteger;

private:
  double m_constant;
};

class SequentialRandomVariable : public RandomVariableStream {
public:
  static TypeId GetTypeId();

  SequentialRandomVariable();

  double GetMin() const;

  double GetMax() const;

  Ptr<RandomVariableStream> GetIncrement() const;

  uint32_t GetConsecutive() const;

  double GetValue() override;
  using RandomVariableStream::GetInteger;

private:
  double m_min;

  double m_max;

  Ptr<RandomVariableStream> m_increment;

  uint32_t m_consecutive;

  double m_current;

  uint32_t m_currentConsecutive;

  bool m_isCurrentSet;
};

class ExponentialRandomVariable : public RandomVariableStream {
public:
  static TypeId GetTypeId();

  ExponentialRandomVariable();

  double GetMean() const;

  double GetBound() const;

  double GetValue(double mean, double bound);

  uint32_t GetInteger(uint32_t mean, uint32_t bound);

  double GetValue() override;
  using RandomVariableStream::GetInteger;

private:
  double m_mean;

  double m_bound;
};

class ParetoRandomVariable : public RandomVariableStream {
public:
  static TypeId GetTypeId();

  ParetoRandomVariable();

  double GetScale() const;

  double GetShape() const;

  double GetBound() const;

  double GetValue(double scale, double shape, double bound);

  uint32_t GetInteger(uint32_t scale, uint32_t shape, uint32_t bound);

  double GetValue() override;
  using RandomVariableStream::GetInteger;

private:
  double m_scale;

  double m_shape;

  double m_bound;
};

class WeibullRandomVariable : public RandomVariableStream {
public:
  static TypeId GetTypeId();

  WeibullRandomVariable();

  double GetScale() const;

  double GetShape() const;

  double GetBound() const;

  double GetValue(double scale, double shape, double bound);

  uint32_t GetInteger(uint32_t scale, uint32_t shape, uint32_t bound);

  double GetValue() override;
  using RandomVariableStream::GetInteger;

private:
  double m_scale;

  double m_shape;

  double m_bound;
};

class NormalRandomVariable : public RandomVariableStream {
public:
  static const double INFINITE_VALUE;

  static TypeId GetTypeId();

  NormalRandomVariable();

  double GetMean() const;

  double GetVariance() const;

  double GetBound() const;

  double GetValue(double mean, double variance,
                  double bound = NormalRandomVariable::INFINITE_VALUE);

  uint32_t GetInteger(uint32_t mean, uint32_t variance, uint32_t bound);

  double GetValue() override;
  using RandomVariableStream::GetInteger;

private:
  double m_mean;

  double m_variance;

  double m_bound;

  bool m_nextValid;

  double m_v2;
  double m_y;
};

class LogNormalRandomVariable : public RandomVariableStream {
public:
  static TypeId GetTypeId();

  LogNormalRandomVariable();

  double GetMu() const;

  double GetSigma() const;

  double GetValue(double mu, double sigma);

  uint32_t GetInteger(uint32_t mu, uint32_t sigma);

  double GetValue() override;
  using RandomVariableStream::GetInteger;

private:
  double m_mu;

  double m_sigma;

  bool m_nextValid;

  double m_v2;

  double m_normal;
};

class GammaRandomVariable : public RandomVariableStream {
public:
  static TypeId GetTypeId();

  GammaRandomVariable();

  double GetAlpha() const;

  double GetBeta() const;

  double GetValue(double alpha, double beta);

  uint32_t GetInteger(uint32_t alpha, uint32_t beta);

  double GetValue() override;
  using RandomVariableStream::GetInteger;

private:
  double GetNormalValue(double mean, double variance, double bound);

  double m_alpha;

  double m_beta;

  bool m_nextValid;

  double m_v2;
  double m_y;
};

class ErlangRandomVariable : public RandomVariableStream {
public:
  static TypeId GetTypeId();

  ErlangRandomVariable();

  uint32_t GetK() const;

  double GetLambda() const;

  double GetValue(uint32_t k, double lambda);

  uint32_t GetInteger(uint32_t k, uint32_t lambda);

  double GetValue() override;
  using RandomVariableStream::GetInteger;

private:
  double GetExponentialValue(double mean, double bound);

  uint32_t m_k;

  double m_lambda;
};

class TriangularRandomVariable : public RandomVariableStream {
public:
  static TypeId GetTypeId();

  TriangularRandomVariable();

  double GetMean() const;

  double GetMin() const;

  double GetMax() const;

  double GetValue(double mean, double min, double max);

  uint32_t GetInteger(uint32_t mean, uint32_t min, uint32_t max);

  double GetValue() override;
  using RandomVariableStream::GetInteger;

private:
  double m_mean;

  double m_min;

  double m_max;
};

class ZipfRandomVariable : public RandomVariableStream {
public:
  static TypeId GetTypeId();

  ZipfRandomVariable();

  uint32_t GetN() const;

  double GetAlpha() const;

  double GetValue(uint32_t n, double alpha);

  uint32_t GetInteger(uint32_t n, uint32_t alpha);

  double GetValue() override;
  using RandomVariableStream::GetInteger;

private:
  uint32_t m_n;

  double m_alpha;

  double m_c;
};

class ZetaRandomVariable : public RandomVariableStream {
public:
  static TypeId GetTypeId();

  ZetaRandomVariable();

  double GetAlpha() const;

  double GetValue(double alpha);

  uint32_t GetInteger(uint32_t alpha);

  double GetValue() override;
  using RandomVariableStream::GetInteger;

private:
  double m_alpha;

  double m_b;
};

class DeterministicRandomVariable : public RandomVariableStream {
public:
  static TypeId GetTypeId();

  DeterministicRandomVariable();
  ~DeterministicRandomVariable() override;

  void SetValueArray(const std::vector<double> &values);
  void SetValueArray(const double *values, std::size_t length);

  double GetValue() override;
  using RandomVariableStream::GetInteger;

private:
  std::size_t m_count;

  std::size_t m_next;

  double *m_data;
};

class EmpiricalRandomVariable : public RandomVariableStream {
public:
  static TypeId GetTypeId();

  EmpiricalRandomVariable();

  void CDF(double v, double c);

  double GetValue() override;
  using RandomVariableStream::GetInteger;

  virtual double Interpolate();

  bool SetInterpolate(bool interpolate);

private:
  void Validate();
  bool PreSample(double &value);
  double DoSampleCDF(double r);
  double DoInterpolate(double r);

  bool m_validated;
  std::map<double, double> m_empCdf;
  bool m_interpolate;
};

} // namespace ns3

#endif
