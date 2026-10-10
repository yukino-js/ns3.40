#include "abort.h"
#include "log.h"
#include "nstime.h"

#include <cmath>
#include <iomanip>
#include <mutex>
#include <sstream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE_MASK("Time", ns3::LOG_PREFIX_TIME);

namespace {

const int8_t UNIT_POWER[Time::LAST] = {17, 17, 17, 16, 15, 12, 9, 6, 3, 0};
const int32_t UNIT_COEFF[Time::LAST] = {315360, 864, 36, 6, 1, 1, 1, 1, 1, 1};

long double Scale(Time::Unit u) {
  return UNIT_COEFF[u] * std::pow(10L, UNIT_POWER[u]);
}

long double *InitUnitValue() {
  static long double values[Time::LAST];
  for (auto u = static_cast<int>(Time::Y); u != static_cast<int>(Time::LAST);
       ++u) {
    values[u] = Scale(static_cast<Time::Unit>(u));
  }
  return values;
}

const long double *UNIT_VALUE = InitUnitValue();

} // namespace

Time::MarkedTimes *Time::g_markingTimes = nullptr;

static std::mutex g_markingMutex;

bool Time::StaticInit() {
  static bool firstTime = true;

  std::unique_lock lock{g_markingMutex};

  if (firstTime) {
    if (!g_markingTimes) {
      static MarkedTimes markingTimes;
      g_markingTimes = &markingTimes;
    } else {
      NS_LOG_ERROR("firstTime but g_markingTimes != 0");
    }

    firstTime = false;
  }

  return firstTime;
}

Time::Time(const std::string &s) {
  NS_LOG_FUNCTION(this << &s);
  std::string::size_type n = s.find_first_not_of("+-0123456789.eE");
  if (n != std::string::npos) {
    std::istringstream iss;
    iss.str(s.substr(0, n));
    double r;
    iss >> r;
    std::string trailer = s.substr(n, std::string::npos);
    if (trailer == "s") {
      *this = Time::FromDouble(r, Time::S);
    } else if (trailer == "ms") {
      *this = Time::FromDouble(r, Time::MS);
    } else if (trailer == "us") {
      *this = Time::FromDouble(r, Time::US);
    } else if (trailer == "ns") {
      *this = Time::FromDouble(r, Time::NS);
    } else if (trailer == "ps") {
      *this = Time::FromDouble(r, Time::PS);
    } else if (trailer == "fs") {
      *this = Time::FromDouble(r, Time::FS);
    } else if (trailer == "min") {
      *this = Time::FromDouble(r, Time::MIN);
    } else if (trailer == "h") {
      *this = Time::FromDouble(r, Time::H);
    } else if (trailer == "d") {
      *this = Time::FromDouble(r, Time::D);
    } else if (trailer == "y") {
      *this = Time::FromDouble(r, Time::Y);
    } else {
      NS_ABORT_MSG("Can't Parse Time " << s);
    }
  } else {
    std::istringstream iss;
    iss.str(s);
    double v;
    iss >> v;
    *this = Time::FromDouble(v, Time::S);
  }

  if (g_markingTimes) {
    Mark(this);
  }
}

Time::Resolution &Time::SetDefaultNsResolution() {
  NS_LOG_FUNCTION_NOARGS();
  static Resolution resolution;
  SetResolution(Time::NS, &resolution, false);
  return resolution;
}

void Time::SetResolution(Unit resolution) {
  NS_LOG_FUNCTION(resolution);
  SetResolution(resolution, PeekResolution());
}

void Time::SetResolution(Unit unit, Resolution *resolution,
                         const bool convert) {
  NS_LOG_FUNCTION(resolution);
  if (convert) {
    ConvertTimes(unit);
  }

  for (int i = 0; i < Time::LAST; i++) {
    int shift = UNIT_POWER[i] - UNIT_POWER[(int)unit];
    int quotient = 1;
    if (UNIT_COEFF[i] > UNIT_COEFF[(int)unit]) {
      quotient = UNIT_COEFF[i] / UNIT_COEFF[(int)unit];
      NS_ASSERT(quotient * UNIT_COEFF[(int)unit] == UNIT_COEFF[i]);
    } else if (UNIT_COEFF[i] < UNIT_COEFF[(int)unit]) {
      quotient = UNIT_COEFF[(int)unit] / UNIT_COEFF[i];
      NS_ASSERT(quotient * UNIT_COEFF[i] == UNIT_COEFF[(int)unit]);
    }
    NS_LOG_DEBUG("SetResolution for unit " << (int)unit << " loop iteration "
                                           << i << " has shift " << shift
                                           << " has quotient " << quotient);

    Information *info = &resolution->info[i];
    if ((std::pow(10, std::fabs(shift)) * quotient) >
        static_cast<double>(std::numeric_limits<int64_t>::max())) {
      NS_LOG_DEBUG("SetResolution for unit " << (int)unit << " loop iteration "
                                             << i << " marked as INVALID");
      info->isValid = false;
      continue;
    }
    auto factor =
        static_cast<int64_t>(std::pow(10, std::fabs(shift)) * quotient);
    double realFactor = std::pow(10, (double)shift) *
                        static_cast<double>(UNIT_COEFF[i]) /
                        UNIT_COEFF[(int)unit];
    NS_LOG_DEBUG("SetResolution factor " << factor << " real factor "
                                         << realFactor);
    info->factor = factor;
    if (shift == 0 && quotient == 1) {
      info->timeFrom = int64x64_t(1);
      info->timeTo = int64x64_t(1);
      info->toMul = true;
      info->fromMul = true;
      info->isValid = true;
    } else if (realFactor > 1) {
      info->timeFrom = int64x64_t(factor);
      info->timeTo = int64x64_t::Invert(factor);
      info->toMul = false;
      info->fromMul = true;
      info->isValid = true;
    } else {
      NS_ASSERT(realFactor < 1);
      info->timeFrom = int64x64_t::Invert(factor);
      info->timeTo = int64x64_t(factor);
      info->toMul = true;
      info->fromMul = false;
      info->isValid = true;
    }
  }
  resolution->unit = unit;
}

void Time::ClearMarkedTimes() {

  std::unique_lock lock{g_markingMutex};

  NS_LOG_FUNCTION_NOARGS();
  if (g_markingTimes) {
    NS_LOG_LOGIC("clearing MarkedTimes");
    g_markingTimes->erase(g_markingTimes->begin(), g_markingTimes->end());
    g_markingTimes = nullptr;
  }
}

void Time::Mark(Time *const time) {
  std::unique_lock lock{g_markingMutex};

  NS_LOG_FUNCTION(time);
  NS_ASSERT(time != nullptr);

  if (g_markingTimes) {
    auto ret = g_markingTimes->insert(time);
    NS_LOG_LOGIC("\t[" << g_markingTimes->size() << "] recording " << time);

    if (!ret.second) {
      NS_LOG_WARN("already recorded " << time << "!");
    }
  }
}

void Time::Clear(Time *const time) {
  std::unique_lock lock{g_markingMutex};

  NS_LOG_FUNCTION(time);
  NS_ASSERT(time != nullptr);

  if (g_markingTimes) {
    NS_ASSERT_MSG(g_markingTimes->count(time) == 1,
                  "Time object " << time << " registered "
                                 << g_markingTimes->count(time)
                                 << " times (should be 1).");

    MarkedTimes::size_type num = g_markingTimes->erase(time);
    if (num != 1) {
      NS_LOG_WARN("unexpected result erasing " << time << "!");
      NS_LOG_WARN("got " << num << ", expected 1");
    } else {
      NS_LOG_LOGIC("\t[" << g_markingTimes->size() << "] removing  " << time);
    }
  }
}

void Time::ConvertTimes(const Unit unit) {
  std::unique_lock lock{g_markingMutex};

  NS_LOG_FUNCTION_NOARGS();

  NS_ASSERT_MSG(g_markingTimes != nullptr,
                "No MarkedTimes registry. "
                "Time::SetResolution () called more than once?");

  for (auto it = g_markingTimes->begin(); it != g_markingTimes->end(); it++) {
    Time *const tp = *it;
    if (!(tp->m_data == std::numeric_limits<int64_t>::min() ||
          tp->m_data == std::numeric_limits<int64_t>::max())) {
      tp->m_data = tp->ToInteger(unit);
    }
  }

  NS_LOG_LOGIC("logged " << g_markingTimes->size() << " Time objects.");

  NS_LOG_LOGIC("clearing MarkedTimes");
  g_markingTimes->erase(g_markingTimes->begin(), g_markingTimes->end());
  g_markingTimes = nullptr;
}

Time::Unit Time::GetResolution() { return PeekResolution()->unit; }

TimeWithUnit Time::As(const Unit unit) const {
  return TimeWithUnit(*this, unit);
}

std::ostream &operator<<(std::ostream &os, const Time &time) {
  os << time.As(Time::GetResolution());
  return os;
}

std::ostream &operator<<(std::ostream &os, const TimeWithUnit &timeU) {
  std::string label;
  Time::Unit unit = timeU.m_unit;

  if (unit == Time::AUTO) {
    auto value = static_cast<long double>(timeU.m_time.GetTimeStep());
    value *= Scale(Time::GetResolution());
    int u = Time::Y;
    while (u != Time::LAST && UNIT_VALUE[u] > value) {
      ++u;
    }
    if (u == Time::LAST) {
      --u;
    }
    unit = static_cast<Time::Unit>(u);
  }

  switch (unit) {
  case Time::Y:
    label = "y";
    break;
  case Time::D:
    label = "d";
    break;
  case Time::H:
    label = "h";
    break;
  case Time::MIN:
    label = "min";
    break;
  case Time::S:
    label = "s";
    break;
  case Time::MS:
    label = "ms";
    break;
  case Time::US:
    label = "us";
    break;
  case Time::NS:
    label = "ns";
    break;
  case Time::PS:
    label = "ps";
    break;
  case Time::FS:
    label = "fs";
    break;

  case Time::LAST:
  case Time::AUTO:
  default:
    NS_ABORT_MSG("can't be reached");
    label = "unreachable";
    break;
  }

  double v = timeU.m_time.ToDouble(unit);

  std::ios_base::fmtflags ff = os.flags();

  os << std::showpos << std::right << v << label;

  if (!(ff & std::ios_base::showpos)) {
    os << std::noshowpos;
  }
  if (ff & std::ios_base::left) {
    os << std::left;
  } else if (ff & std::ios_base::internal) {
    os << std::internal;
  }

  return os;
}

std::istream &operator>>(std::istream &is, Time &time) {
  std::string value;
  is >> value;
  time = Time(value);
  return is;
}

ATTRIBUTE_VALUE_IMPLEMENT(Time);

Ptr<const AttributeChecker> MakeTimeChecker(const Time min, const Time max) {
  NS_LOG_FUNCTION(min << max);

  struct Checker : public AttributeChecker {
    Checker(const Time minValue, const Time maxValue)
        : m_minValue(minValue), m_maxValue(maxValue) {}

    bool Check(const AttributeValue &value) const override {
      NS_LOG_FUNCTION(&value);
      const auto v = dynamic_cast<const TimeValue *>(&value);
      if (v == nullptr) {
        return false;
      }
      return v->Get() >= m_minValue && v->Get() <= m_maxValue;
    }

    std::string GetValueTypeName() const override {
      NS_LOG_FUNCTION_NOARGS();
      return "ns3::TimeValue";
    }

    bool HasUnderlyingTypeInformation() const override {
      NS_LOG_FUNCTION_NOARGS();
      return true;
    }

    std::string GetUnderlyingTypeInformation() const override {
      NS_LOG_FUNCTION_NOARGS();
      std::ostringstream oss;
      oss << "Time"
          << " " << m_minValue << ":" << m_maxValue;
      return oss.str();
    }

    Ptr<AttributeValue> Create() const override {
      NS_LOG_FUNCTION_NOARGS();
      return ns3::Create<TimeValue>();
    }

    bool Copy(const AttributeValue &source,
              AttributeValue &destination) const override {
      NS_LOG_FUNCTION(&source << &destination);
      const auto src = dynamic_cast<const TimeValue *>(&source);
      auto dst = dynamic_cast<TimeValue *>(&destination);
      if (src == nullptr || dst == nullptr) {
        return false;
      }
      *dst = *src;
      return true;
    }

    Time m_minValue;
    Time m_maxValue;
  } *checker = new Checker(min, max);

  return Ptr<const AttributeChecker>(checker, false);
}

} // namespace ns3
