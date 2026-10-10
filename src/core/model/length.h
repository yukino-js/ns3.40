
#ifndef NS3_LENGTH_H_
#define NS3_LENGTH_H_

#include "attribute-helper.h"
#include "attribute.h"

#ifdef HAVE_BOOST
#include <boost/units/quantity.hpp>
#include <boost/units/systems/si.hpp>
#endif

#include <istream>
#include <limits>
#include <optional>
#include <ostream>
#include <string>

namespace ns3 {

class Length {
public:
  enum Unit : uint16_t {
    Nanometer = 1,
    Micrometer,
    Millimeter,
    Centimeter,
    Meter,
    Kilometer,
    NauticalMile,

    Inch,
    Foot,
    Yard,
    Mile
  };

  class Quantity {
  public:
    Quantity(double value, Length::Unit unit) : m_value(value), m_unit(unit) {}

    Quantity(const Quantity &) = default;

    Quantity(Quantity &&) = default;

    ~Quantity() = default;

    Quantity &operator=(const Quantity &other) = default;

    Quantity &operator=(Quantity &&other) = default;

    double Value() const { return m_value; }

    Length::Unit Unit() const { return m_unit; }

  private:
    double m_value;
    Length::Unit m_unit;
  };

  static constexpr double DEFAULT_TOLERANCE =
      std::numeric_limits<double>::epsilon();

  static std::optional<Length> TryParse(double value, const std::string &unit);

  Length();

  Length(const std::string &text);

  Length(double value, const std::string &unit);

  Length(double value, Length::Unit unit);

  Length(Quantity quantity);

#ifdef HAVE_BOOST_UNITS
  template <class U, class T>
  explicit Length(boost::units::quantity<U, T> quantity);
#endif

  Length(const Length &other) = default;

  Length(Length &&other) = default;

  ~Length() = default;

  Length &operator=(const Length &other) = default;

  Length &operator=(Length &&other) = default;

  Length &operator=(const Length::Quantity &q);

  bool IsEqual(const Length &other, double tolerance = DEFAULT_TOLERANCE) const;

  bool IsNotEqual(const Length &other,
                  double tolerance = DEFAULT_TOLERANCE) const;

  bool IsLess(const Length &other, double tolerance = DEFAULT_TOLERANCE) const;

  bool IsLessOrEqual(const Length &other,
                     double tolerance = DEFAULT_TOLERANCE) const;

  bool IsGreater(const Length &other,
                 double tolerance = DEFAULT_TOLERANCE) const;

  bool IsGreaterOrEqual(const Length &other,
                        double tolerance = DEFAULT_TOLERANCE) const;

  void swap(Length &other);

  double GetDouble() const;

  Quantity As(Unit unit) const;

private:
  double m_value;
};

ATTRIBUTE_HELPER_HEADER(Length);

std::string ToSymbol(Length::Unit unit);

std::string ToName(Length::Unit unit, bool plural = false);

std::optional<Length::Unit> FromString(std::string unitString);

std::ostream &operator<<(std::ostream &stream, const Length &l);

std::ostream &operator<<(std::ostream &stream, const Length::Quantity &q);

std::ostream &operator<<(std::ostream &stream, Length::Unit unit);

std::istream &operator>>(std::istream &stream, Length &l);

bool operator==(const Length &left, const Length &right);

bool operator!=(const Length &left, const Length &right);

bool operator<(const Length &left, const Length &right);

bool operator<=(const Length &left, const Length &right);

bool operator>(const Length &left, const Length &right);

bool operator>=(const Length &left, const Length &right);

Length operator+(const Length &left, const Length &right);

Length operator-(const Length &left, const Length &right);

Length operator*(double scalar, const Length &l);
Length operator*(const Length &l, double scalar);

Length operator/(const Length &left, double scalar);

double operator/(const Length &numerator, const Length &denominator);

int64_t Div(const Length &numerator, const Length &denominator,
            Length *remainder = nullptr);

Length Mod(const Length &numerator, const Length &denominator);

Length NanoMeters(double value);
Length MicroMeters(double value);
Length MilliMeters(double value);
Length CentiMeters(double value);
Length Meters(double value);
Length KiloMeters(double value);
Length NauticalMiles(double value);
Length Inches(double value);
Length Feet(double value);
Length Yards(double value);
Length Miles(double value);

#ifdef HAVE_BOOST_UNITS
template <class U, class T>
Length::Length(boost::units::quantity<U, T> quantity) : m_value(0) {
  namespace bu = boost::units;
  using BoostMeters = bu::quantity<bu::si::length, double>;

  m_value = static_cast<BoostMeters>(quantity).value();
}
#endif

} // namespace ns3

#endif
