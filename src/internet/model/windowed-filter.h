

#ifndef WINDOWED_FILTER_H_
#define WINDOWED_FILTER_H_

namespace ns3 {
template <class T> struct MinFilter {
public:
  bool operator()(const T &lhs, const T &rhs) const {
    if (rhs == 0 || lhs == 0) {
      return false;
    }
    return lhs <= rhs;
  }
};

template <class T> struct MaxFilter {
public:
  bool operator()(const T &lhs, const T &rhs) const {
    if (rhs == 0 || lhs == 0) {
      return false;
    }
    return lhs >= rhs;
  }
};

template <class T, class Compare, typename TimeT, typename TimeDeltaT>
class WindowedFilter {
public:
  WindowedFilter() {}

  WindowedFilter(TimeDeltaT windowLength, T zeroValue, TimeT zeroTime)
      : m_windowLength(windowLength), m_zeroValue(zeroValue),
        m_samples{Sample(m_zeroValue, zeroTime), Sample(m_zeroValue, zeroTime),
                  Sample(m_zeroValue, zeroTime)} {}

  void SetWindowLength(TimeDeltaT windowLength) {
    m_windowLength = windowLength;
  }

  void Update(T new_sample, TimeT new_time) {
    if (m_samples[0].sample == m_zeroValue ||
        Compare()(new_sample, m_samples[0].sample) ||
        new_time - m_samples[2].time > m_windowLength) {
      Reset(new_sample, new_time);
      return;
    }
    if (Compare()(new_sample, m_samples[1].sample)) {
      m_samples[1] = Sample(new_sample, new_time);
      m_samples[2] = m_samples[1];
    } else if (Compare()(new_sample, m_samples[2].sample)) {
      m_samples[2] = Sample(new_sample, new_time);
    }
    if (new_time - m_samples[0].time > m_windowLength) {
      m_samples[0] = m_samples[1];
      m_samples[1] = m_samples[2];
      m_samples[2] = Sample(new_sample, new_time);
      if (new_time - m_samples[0].time > m_windowLength) {
        m_samples[0] = m_samples[1];
        m_samples[1] = m_samples[2];
      }
      return;
    }
    if (m_samples[1].sample == m_samples[0].sample &&
        new_time - m_samples[1].time > m_windowLength >> 2) {
      m_samples[2] = m_samples[1] = Sample(new_sample, new_time);
      return;
    }
    if (m_samples[2].sample == m_samples[1].sample &&
        new_time - m_samples[2].time > m_windowLength >> 1) {
      m_samples[2] = Sample(new_sample, new_time);
    }
  }

  void Reset(T new_sample, TimeT new_time) {
    m_samples[0] = m_samples[1] = m_samples[2] = Sample(new_sample, new_time);
  }

  T GetBest() const { return m_samples[0].sample; }

  T GetSecondBest() const { return m_samples[1].sample; }

  T GetThirdBest() const { return m_samples[2].sample; }

  struct Sample {
    T sample;
    TimeT time;

    Sample() {}

    Sample(T init_sample, TimeT init_time)
        : sample(init_sample), time(init_time) {}
  };

  TimeDeltaT m_windowLength;
  T m_zeroValue;
  Sample m_samples[3];
};

} // namespace ns3
#endif
