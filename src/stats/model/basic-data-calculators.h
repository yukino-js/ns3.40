
#ifndef BASIC_DATA_CALCULATORS_H
#define BASIC_DATA_CALCULATORS_H

#include "data-calculator.h"
#include "data-output-interface.h"

#include "ns3/type-name.h"

namespace ns3 {

template <typename T = uint32_t>
class MinMaxAvgTotalCalculator : public DataCalculator,
                                 public StatisticalSummary {
public:
  MinMaxAvgTotalCalculator();
  ~MinMaxAvgTotalCalculator() override;

  static TypeId GetTypeId();

  void Update(const T i);
  void Reset();

  void Output(DataOutputCallback &callback) const override;

  long getCount() const override { return m_count; }

  double getSum() const override { return m_total; }

  double getMin() const override { return m_min; }

  double getMax() const override { return m_max; }

  double getMean() const override { return m_meanCurr; }

  double getStddev() const override { return std::sqrt(m_varianceCurr); }

  double getVariance() const override { return m_varianceCurr; }

  double getSqrSum() const override { return m_squareTotal; }

protected:
  void DoDispose() override;

  uint32_t m_count;

  T m_total;
  T m_squareTotal;
  T m_min;
  T m_max;

  double m_meanCurr;
  double m_sCurr;
  double m_varianceCurr;

  double m_meanPrev;
  double m_sPrev;
};

template <typename T> MinMaxAvgTotalCalculator<T>::MinMaxAvgTotalCalculator() {
  m_count = 0;

  m_total = 0;
  m_squareTotal = 0;

  m_meanCurr = NaN;
  m_sCurr = NaN;
  m_varianceCurr = NaN;

  m_meanPrev = NaN;
  m_sPrev = NaN;
}

template <typename T>
MinMaxAvgTotalCalculator<T>::~MinMaxAvgTotalCalculator() {}

template <typename T> void MinMaxAvgTotalCalculator<T>::DoDispose() {
  DataCalculator::DoDispose();
}

template <typename T> TypeId MinMaxAvgTotalCalculator<T>::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::MinMaxAvgTotalCalculator<" + TypeNameGet<T>() + ">")
          .SetParent<Object>()
          .SetGroupName("Stats")
          .AddConstructor<MinMaxAvgTotalCalculator<T>>();
  return tid;
}

template <typename T> void MinMaxAvgTotalCalculator<T>::Update(const T i) {
  if (m_enabled) {
    m_count++;

    m_total += i;
    m_squareTotal += i * i;

    if (m_count == 1) {
      m_min = i;
      m_max = i;
    } else {
      m_min = (i < m_min) ? i : m_min;
      m_max = (i > m_max) ? i : m_max;
    }

    if (m_count == 1) {
      m_meanCurr = i;
      m_sCurr = 0;
      m_varianceCurr = m_sCurr;
    } else {
      m_meanPrev = m_meanCurr;
      m_sPrev = m_sCurr;

      m_meanCurr = m_meanPrev + (i - m_meanPrev) / m_count;
      m_sCurr = m_sPrev + (i - m_meanPrev) * (i - m_meanCurr);
      m_varianceCurr = m_sCurr / (m_count - 1);
    }
  }
}

template <typename T> void MinMaxAvgTotalCalculator<T>::Reset() {
  m_count = 0;

  m_total = 0;
  m_squareTotal = 0;

  m_meanCurr = NaN;
  m_sCurr = NaN;
  m_varianceCurr = NaN;

  m_meanPrev = NaN;
  m_sPrev = NaN;
}

template <typename T>
void MinMaxAvgTotalCalculator<T>::Output(DataOutputCallback &callback) const {
  callback.OutputStatistic(m_context, m_key, this);
}

template <typename T = uint32_t>
class CounterCalculator : public DataCalculator {
public:
  CounterCalculator();
  ~CounterCalculator() override;

  static TypeId GetTypeId();

  void Update();
  void Update(const T i);

  T GetCount() const;

  void Output(DataOutputCallback &callback) const override;

protected:
  void DoDispose() override;

  T m_count;
};

template <typename T> CounterCalculator<T>::CounterCalculator() : m_count(0) {}

template <typename T> CounterCalculator<T>::~CounterCalculator() {}

template <typename T> TypeId CounterCalculator<T>::GetTypeId() {
  static TypeId tid = TypeId("ns3::CounterCalculator<" + TypeNameGet<T>() + ">")
                          .SetParent<Object>()
                          .SetGroupName("Stats")
                          .AddConstructor<CounterCalculator<T>>();
  return tid;
}

template <typename T> void CounterCalculator<T>::DoDispose() {
  DataCalculator::DoDispose();
}

template <typename T> void CounterCalculator<T>::Update() {
  if (m_enabled) {
    m_count++;
  }
}

template <typename T> void CounterCalculator<T>::Update(const T i) {
  if (m_enabled) {
    m_count += i;
  }
}

template <typename T> T CounterCalculator<T>::GetCount() const {
  return m_count;
}

template <typename T>
void CounterCalculator<T>::Output(DataOutputCallback &callback) const {
  callback.OutputSingleton(m_context, m_key, m_count);
}

extern template class CounterCalculator<uint32_t>;

}; // namespace ns3

#endif
