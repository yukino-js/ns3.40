
#ifndef DATA_CALCULATOR_H
#define DATA_CALCULATOR_H

#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/simulator.h"

namespace ns3 {
extern const double NaN;

inline bool isNaN(double x) { return x != x; }

class DataOutputCallback;

class StatisticalSummary {
public:
  virtual ~StatisticalSummary() {}

  virtual long getCount() const = 0;

  virtual double getSum() const = 0;

  virtual double getSqrSum() const = 0;

  virtual double getMin() const = 0;

  virtual double getMax() const = 0;

  virtual double getMean() const = 0;

  virtual double getStddev() const = 0;

  virtual double getVariance() const = 0;
};

class DataCalculator : public Object {
public:
  DataCalculator();
  ~DataCalculator() override;

  static TypeId GetTypeId();

  bool GetEnabled() const;
  void Enable();
  void Disable();
  void SetKey(const std::string key);
  std::string GetKey() const;

  void SetContext(const std::string context);
  std::string GetContext() const;

  virtual void Start(const Time &startTime);
  virtual void Stop(const Time &stopTime);

  virtual void Output(DataOutputCallback &callback) const = 0;

protected:
  bool m_enabled;

  std::string m_key;
  std::string m_context;

  void DoDispose() override;

private:
  EventId m_startEvent;
  EventId m_stopEvent;
};

}; // namespace ns3

#endif
