#ifndef TIME_H
#define TIME_H

#include "assert.h"
#include "attribute-helper.h"
#include "attribute.h"
#include "event-id.h"
#include "int64x64.h"
#include "type-name.h"

#include <cmath>
#include <limits>
#include <ostream>
#include <set>
#include <stdint.h>

namespace ns3 {

class TimeWithUnit;

class Time {
public:
  enum Unit {
    Y = 0,
    D = 1,
    H = 2,
    MIN = 3,
    S = 4,
    MS = 5,
    US = 6,
    NS = 7,
    PS = 8,
    FS = 9,
    LAST = 10,
    AUTO = 11
  };

  inline Time &operator=(const Time &o) {
    m_data = o.m_data;
    return *this;
  }

  inline Time() : m_data() {
    if (g_markingTimes) {
      Mark(this);
    }
  }

  inline Time(const Time &o) : m_data(o.m_data) {
    if (g_markingTimes) {
      Mark(this);
    }
  }

  Time(Time &&o) : m_data(o.m_data) {
    if (g_markingTimes) {
      Mark(this);
    }
  }

  explicit inline Time(double v) : m_data(llround(v)) {
    if (g_markingTimes) {
      Mark(this);
    }
  }

  explicit inline Time(int v) : m_data(v) {
    if (g_markingTimes) {
      Mark(this);
    }
  }

  explicit inline Time(long int v) : m_data(v) {
    if (g_markingTimes) {
      Mark(this);
    }
  }

  explicit inline Time(long long int v) : m_data(v) {
    if (g_markingTimes) {
      Mark(this);
    }
  }

  explicit inline Time(unsigned int v) : m_data(v) {
    if (g_markingTimes) {
      Mark(this);
    }
  }

  explicit inline Time(unsigned long int v) : m_data(v) {
    if (g_markingTimes) {
      Mark(this);
    }
  }

  explicit inline Time(unsigned long long int v) : m_data(v) {
    if (g_markingTimes) {
      Mark(this);
    }
  }

  explicit inline Time(const int64x64_t &v) : m_data(v.Round()) {
    if (g_markingTimes) {
      Mark(this);
    }
  }

  explicit Time(const std::string &s);

  static Time Min() { return Time(std::numeric_limits<int64_t>::min()); }

  static Time Max() { return Time(std::numeric_limits<int64_t>::max()); }

  ~Time() {
    if (g_markingTimes) {
      Clear(this);
    }
  }

  inline bool IsZero() const { return m_data == 0; }

  inline bool IsNegative() const { return m_data <= 0; }

  inline bool IsPositive() const { return m_data >= 0; }

  inline bool IsStrictlyNegative() const { return m_data < 0; }

  inline bool IsStrictlyPositive() const { return m_data > 0; }

  inline int Compare(const Time &o) const {
    return (m_data < o.m_data) ? -1 : (m_data == o.m_data) ? 0 : 1;
  }

  inline double GetYears() const { return ToDouble(Time::Y); }

  inline double GetDays() const { return ToDouble(Time::D); }

  inline double GetHours() const { return ToDouble(Time::H); }

  inline double GetMinutes() const { return ToDouble(Time::MIN); }

  inline double GetSeconds() const { return ToDouble(Time::S); }

  inline int64_t GetMilliSeconds() const { return ToInteger(Time::MS); }

  inline int64_t GetMicroSeconds() const { return ToInteger(Time::US); }

  inline int64_t GetNanoSeconds() const { return ToInteger(Time::NS); }

  inline int64_t GetPicoSeconds() const { return ToInteger(Time::PS); }

  inline int64_t GetFemtoSeconds() const { return ToInteger(Time::FS); }

  inline int64_t GetTimeStep() const { return m_data; }

  inline double GetDouble() const { return static_cast<double>(m_data); }

  inline int64_t GetInteger() const { return GetTimeStep(); }

  static void SetResolution(Unit resolution);
  static Unit GetResolution();

  inline static Time From(const int64x64_t &value) { return Time(value); }

  inline static Time FromInteger(uint64_t value, Unit unit) {
    Information *info = PeekInformation(unit);

    NS_ASSERT_MSG(info->isValid,
                  "Attempted a conversion from an unavailable unit.");

    if (info->fromMul) {
      value *= info->factor;
    } else {
      value /= info->factor;
    }
    return Time(value);
  }

  inline static Time FromDouble(double value, Unit unit) {
    return From(int64x64_t(value), unit);
  }

  inline static Time From(const int64x64_t &value, Unit unit) {
    Information *info = PeekInformation(unit);

    NS_ASSERT_MSG(info->isValid,
                  "Attempted a conversion from an unavailable unit.");

    int64x64_t retval = value;
    if (info->fromMul) {
      retval *= info->timeFrom;
    } else {
      retval.MulByInvert(info->timeFrom);
    }
    return Time(retval);
  }

  inline int64_t ToInteger(Unit unit) const {
    Information *info = PeekInformation(unit);

    NS_ASSERT_MSG(info->isValid,
                  "Attempted a conversion to an unavailable unit.");

    int64_t v = m_data;
    if (info->toMul) {
      v *= info->factor;
    } else {
      v /= info->factor;
    }
    return v;
  }

  inline double ToDouble(Unit unit) const { return To(unit).GetDouble(); }

  inline int64x64_t To(Unit unit) const {
    Information *info = PeekInformation(unit);

    NS_ASSERT_MSG(info->isValid,
                  "Attempted a conversion to an unavailable unit.");

    int64x64_t retval(m_data);
    if (info->toMul) {
      retval *= info->timeTo;
    } else {
      retval.MulByInvert(info->timeTo);
    }
    return retval;
  }

  Time RoundTo(Unit unit) const { return From(this->To(unit).Round(), unit); }

  TimeWithUnit As(const Unit unit = Time::AUTO) const;

  typedef void (*TracedCallback)(Time value);

private:
  struct Information {
    bool toMul;
    bool fromMul;
    int64_t factor;
    int64x64_t timeTo;
    int64x64_t timeFrom;
    bool isValid;
  };

  struct Resolution {
    Information info[LAST];
    Time::Unit unit;
  };

  static inline Resolution *PeekResolution() {
    static Time::Resolution &resolution{SetDefaultNsResolution()};
    return &resolution;
  }

  static inline Information *PeekInformation(Unit timeUnit) {
    return &(PeekResolution()->info[timeUnit]);
  }

  static Resolution &SetDefaultNsResolution();
  static void SetResolution(Unit unit, Resolution *resolution,
                            const bool convert = true);

  typedef std::set<Time *> MarkedTimes;
  static MarkedTimes *g_markingTimes;

public:
  static bool StaticInit();

private:
  friend class Simulator;

  static void ClearMarkedTimes();
  static void Mark(Time *const time);
  static void Clear(Time *const time);
  static void ConvertTimes(const Unit unit);

  friend bool operator==(const Time &lhs, const Time &rhs);
  friend bool operator!=(const Time &lhs, const Time &rhs);
  friend bool operator<=(const Time &lhs, const Time &rhs);
  friend bool operator>=(const Time &lhs, const Time &rhs);
  friend bool operator<(const Time &lhs, const Time &rhs);
  friend bool operator>(const Time &lhs, const Time &rhs);
  friend bool operator<(const Time &time, const EventId &event);

  friend Time operator+(const Time &lhs, const Time &rhs);
  friend Time operator-(const Time &lhs, const Time &rhs);
  friend Time operator*(const Time &lhs, const int64x64_t &rhs);
  friend Time operator*(const int64x64_t &lhs, const Time &rhs);
  friend int64x64_t operator/(const Time &lhs, const Time &rhs);
  friend Time operator/(const Time &lhs, const int64x64_t &rhs);
  friend Time operator%(const Time &lhs, const Time &rhs);
  friend int64_t Div(const Time &lhs, const Time &rhs);
  friend Time Rem(const Time &lhs, const Time &rhs);

  template <class T>
  friend std::enable_if_t<std::is_integral_v<T>, Time>
  operator*(const Time &lhs, T rhs);

  template <class T>
  friend std::enable_if_t<std::is_arithmetic_v<T>, Time>
  operator*(T lhs, const Time &rhs);

  template <class T>
  friend std::enable_if_t<std::is_integral_v<T>, Time>
  operator/(const Time &lhs, T rhs);

  friend Time Abs(const Time &time);
  friend Time Max(const Time &timeA, const Time &timeB);
  friend Time Min(const Time &timeA, const Time &timeB);

  template <class T>
  friend std::enable_if_t<std::is_floating_point_v<T>, Time>
  operator*(const Time &lhs, T rhs);
  template <class T>
  friend std::enable_if_t<std::is_floating_point_v<T>, Time>
  operator/(const Time &lhs, T rhs);

  friend Time &operator+=(Time &lhs, const Time &rhs);
  friend Time &operator-=(Time &lhs, const Time &rhs);

  int64_t m_data;
};

namespace TracedValueCallback {

typedef void (*Time)(Time oldValue, Time newValue);

}

static bool g_TimeStaticInit [[maybe_unused]] = Time::StaticInit();

inline bool operator==(const Time &lhs, const Time &rhs) {
  return lhs.m_data == rhs.m_data;
}

inline bool operator!=(const Time &lhs, const Time &rhs) {
  return lhs.m_data != rhs.m_data;
}

inline bool operator<=(const Time &lhs, const Time &rhs) {
  return lhs.m_data <= rhs.m_data;
}

inline bool operator>=(const Time &lhs, const Time &rhs) {
  return lhs.m_data >= rhs.m_data;
}

inline bool operator<(const Time &lhs, const Time &rhs) {
  return lhs.m_data < rhs.m_data;
}

inline bool operator>(const Time &lhs, const Time &rhs) {
  return lhs.m_data > rhs.m_data;
}

inline bool operator<(const Time &time, const EventId &event) {
  if (time.m_data < 0) {
    return true;
  }
  return static_cast<uint64_t>(time.m_data) < event.GetTs();
}

inline Time operator+(const Time &lhs, const Time &rhs) {
  return Time(lhs.m_data + rhs.m_data);
}

inline Time operator-(const Time &lhs, const Time &rhs) {
  return Time(lhs.m_data - rhs.m_data);
}

inline Time operator*(const Time &lhs, const int64x64_t &rhs) {
  int64x64_t res = lhs.m_data;
  res *= rhs;
  return Time(res);
}

inline Time operator*(const int64x64_t &lhs, const Time &rhs) {
  return rhs * lhs;
}

template <class T>
std::enable_if_t<std::is_integral_v<T>, Time> operator*(const Time &lhs,
                                                        T rhs) {
  static_assert(!std::is_same_v<T, bool>,
                "Multiplying a Time by a boolean is not supported");

  return Time(lhs.m_data * rhs);
}

template <class T>
std::enable_if_t<std::is_floating_point_v<T>, Time> operator*(const Time &lhs,
                                                              T rhs) {
  return lhs * int64x64_t(rhs);
}

template <class T>
std::enable_if_t<std::is_arithmetic_v<T>, Time> operator*(T lhs,
                                                          const Time &rhs) {
  return rhs * lhs;
}

inline int64x64_t operator/(const Time &lhs, const Time &rhs) {
  int64x64_t num = lhs.m_data;
  int64x64_t den = rhs.m_data;
  return num / den;
}

inline Time operator/(const Time &lhs, const int64x64_t &rhs) {
  int64x64_t res = lhs.m_data;
  res /= rhs;
  return Time(res);
}

template <class T>
std::enable_if_t<std::is_integral_v<T>, Time> operator/(const Time &lhs,
                                                        T rhs) {
  static_assert(!std::is_same_v<T, bool>,
                "Dividing a Time by a boolean is not supported");

  return Time(lhs.m_data / rhs);
}

template <class T>
std::enable_if_t<std::is_floating_point_v<T>, Time> operator/(const Time &lhs,
                                                              T rhs) {
  return lhs / int64x64_t(rhs);
}

inline Time operator%(const Time &lhs, const Time &rhs) {
  return Time(lhs.m_data % rhs.m_data);
}

inline Time Rem(const Time &lhs, const Time &rhs) {
  return Time(lhs.m_data % rhs.m_data);
}

inline int64_t Div(const Time &lhs, const Time &rhs) {
  return lhs.m_data / rhs.m_data;
}

inline Time &operator+=(Time &lhs, const Time &rhs) {
  lhs.m_data += rhs.m_data;
  return lhs;
}

inline Time &operator-=(Time &lhs, const Time &rhs) {
  lhs.m_data -= rhs.m_data;
  return lhs;
}

inline Time Abs(const Time &time) {
  return Time((time.m_data < 0) ? -time.m_data : time.m_data);
}

inline Time Max(const Time &timeA, const Time &timeB) {
  return Time((timeA.m_data < timeB.m_data) ? timeB : timeA);
}

inline Time Min(const Time &timeA, const Time &timeB) {
  return Time((timeA.m_data > timeB.m_data) ? timeB : timeA);
}

std::ostream &operator<<(std::ostream &os, const Time &time);
std::istream &operator>>(std::istream &is, Time &time);

inline Time Years(double value) { return Time::FromDouble(value, Time::Y); }

inline Time Years(int64x64_t value) { return Time::From(value, Time::Y); }

inline Time Days(double value) { return Time::FromDouble(value, Time::D); }

inline Time Days(int64x64_t value) { return Time::From(value, Time::D); }

inline Time Hours(double value) { return Time::FromDouble(value, Time::H); }

inline Time Hours(int64x64_t value) { return Time::From(value, Time::H); }

inline Time Minutes(double value) { return Time::FromDouble(value, Time::MIN); }

inline Time Minutes(int64x64_t value) { return Time::From(value, Time::MIN); }

inline Time Seconds(double value) { return Time::FromDouble(value, Time::S); }

inline Time Seconds(int64x64_t value) { return Time::From(value, Time::S); }

inline Time MilliSeconds(uint64_t value) {
  return Time::FromInteger(value, Time::MS);
}

inline Time MilliSeconds(int64x64_t value) {
  return Time::From(value, Time::MS);
}

inline Time MicroSeconds(uint64_t value) {
  return Time::FromInteger(value, Time::US);
}

inline Time MicroSeconds(int64x64_t value) {
  return Time::From(value, Time::US);
}

inline Time NanoSeconds(uint64_t value) {
  return Time::FromInteger(value, Time::NS);
}

inline Time NanoSeconds(int64x64_t value) {
  return Time::From(value, Time::NS);
}

inline Time PicoSeconds(uint64_t value) {
  return Time::FromInteger(value, Time::PS);
}

inline Time PicoSeconds(int64x64_t value) {
  return Time::From(value, Time::PS);
}

inline Time FemtoSeconds(uint64_t value) {
  return Time::FromInteger(value, Time::FS);
}

inline Time FemtoSeconds(int64x64_t value) {
  return Time::From(value, Time::FS);
}

inline Time TimeStep(uint64_t ts) { return Time(ts); }

ATTRIBUTE_VALUE_DEFINE(Time);
ATTRIBUTE_ACCESSOR_DEFINE(Time);

Ptr<const AttributeChecker> MakeTimeChecker(const Time min, const Time max);

inline Ptr<const AttributeChecker> MakeTimeChecker() {
  return MakeTimeChecker(Time::Min(), Time::Max());
}

inline Ptr<const AttributeChecker> MakeTimeChecker(const Time min) {
  return MakeTimeChecker(min, Time::Max());
}

class TimeWithUnit {
public:
  TimeWithUnit(const Time time, const Time::Unit unit)
      : m_time(time), m_unit(unit) {}

private:
  Time m_time;
  Time::Unit m_unit;

  friend std::ostream &operator<<(std::ostream &os, const TimeWithUnit &timeU);
};

TYPENAMEGET_DEFINE(Time);

} // namespace ns3

#endif
