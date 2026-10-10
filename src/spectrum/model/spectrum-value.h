
#ifndef SPECTRUM_VALUE_H
#define SPECTRUM_VALUE_H

#include "spectrum-model.h"

#include <ns3/ptr.h>
#include <ns3/simple-ref-count.h>

#include <ostream>
#include <vector>

namespace ns3 {

typedef std::vector<double> Values;

class SpectrumValue : public SimpleRefCount<SpectrumValue> {
public:
  SpectrumValue(Ptr<const SpectrumModel> sm);

  SpectrumValue();

  double &operator[](size_t index);

  const double &operator[](size_t index) const;

  SpectrumModelUid_t GetSpectrumModelUid() const;

  Ptr<const SpectrumModel> GetSpectrumModel() const;

  Bands::const_iterator ConstBandsBegin() const;

  Bands::const_iterator ConstBandsEnd() const;

  Values::const_iterator ConstValuesBegin() const;

  Values::const_iterator ConstValuesEnd() const;

  Values::iterator ValuesBegin();

  Values::iterator ValuesEnd();

  uint32_t GetValuesN() const;

  const double &ValuesAt(uint32_t pos) const;

  friend SpectrumValue operator+(const SpectrumValue &lhs,
                                 const SpectrumValue &rhs);

  friend SpectrumValue operator+(const SpectrumValue &lhs, double rhs);

  friend SpectrumValue operator+(double lhs, const SpectrumValue &rhs);

  friend SpectrumValue operator-(const SpectrumValue &lhs,
                                 const SpectrumValue &rhs);

  friend SpectrumValue operator-(const SpectrumValue &lhs, double rhs);

  friend SpectrumValue operator-(double lhs, const SpectrumValue &rhs);

  friend SpectrumValue operator*(const SpectrumValue &lhs,
                                 const SpectrumValue &rhs);

  friend SpectrumValue operator*(const SpectrumValue &lhs, double rhs);

  friend SpectrumValue operator*(double lhs, const SpectrumValue &rhs);

  friend SpectrumValue operator/(const SpectrumValue &lhs,
                                 const SpectrumValue &rhs);

  friend SpectrumValue operator/(const SpectrumValue &lhs, double rhs);

  friend SpectrumValue operator/(double lhs, const SpectrumValue &rhs);

  friend SpectrumValue operator+(const SpectrumValue &rhs);

  friend SpectrumValue operator-(const SpectrumValue &rhs);

  SpectrumValue operator<<(int n) const;

  SpectrumValue operator>>(int n) const;

  SpectrumValue &operator+=(const SpectrumValue &rhs);

  SpectrumValue &operator-=(const SpectrumValue &rhs);

  SpectrumValue &operator*=(const SpectrumValue &rhs);

  SpectrumValue &operator/=(const SpectrumValue &rhs);

  SpectrumValue &operator+=(double rhs);

  SpectrumValue &operator-=(double rhs);

  SpectrumValue &operator*=(double rhs);

  SpectrumValue &operator/=(double rhs);

  SpectrumValue &operator=(double rhs);

  friend double Norm(const SpectrumValue &x);

  friend double Sum(const SpectrumValue &x);

  friend double Prod(const SpectrumValue &x);

  friend SpectrumValue Pow(const SpectrumValue &lhs, double rhs);

  friend SpectrumValue Pow(double lhs, const SpectrumValue &rhs);

  friend SpectrumValue Log10(const SpectrumValue &arg);

  friend SpectrumValue Log2(const SpectrumValue &arg);

  friend SpectrumValue Log(const SpectrumValue &arg);

  friend double Integral(const SpectrumValue &arg);

  Ptr<SpectrumValue> Copy() const;

  typedef void (*TracedCallback)(Ptr<SpectrumValue> value);

private:
  void Add(const SpectrumValue &x);
  void Add(double s);
  void Subtract(const SpectrumValue &x);
  void Subtract(double s);
  void Multiply(const SpectrumValue &x);
  void Multiply(double s);
  void Divide(const SpectrumValue &x);
  void Divide(double s);
  void ChangeSign();
  void ShiftLeft(int n);
  void ShiftRight(int n);
  void Pow(double exp);
  void Exp(double base);
  void Log10();
  void Log2();
  void Log();

  Ptr<const SpectrumModel> m_spectrumModel;

  Values m_values;
};

std::ostream &operator<<(std::ostream &os, const SpectrumValue &pvf);

double Norm(const SpectrumValue &x);
double Sum(const SpectrumValue &x);
double Prod(const SpectrumValue &x);
SpectrumValue Pow(const SpectrumValue &lhs, double rhs);
SpectrumValue Pow(double lhs, const SpectrumValue &rhs);
SpectrumValue Log10(const SpectrumValue &arg);
SpectrumValue Log2(const SpectrumValue &arg);
SpectrumValue Log(const SpectrumValue &arg);
double Integral(const SpectrumValue &arg);

} // namespace ns3

#endif
