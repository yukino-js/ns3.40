
#ifndef DATA_OUTPUT_INTERFACE_H
#define DATA_OUTPUT_INTERFACE_H

#include "data-calculator.h"

#include "ns3/nstime.h"
#include "ns3/object.h"

namespace ns3 {

class DataCollector;

class DataOutputInterface : public Object {
public:
  DataOutputInterface();
  ~DataOutputInterface() override;

  static TypeId GetTypeId();

  virtual void Output(DataCollector &dc) = 0;

  void SetFilePrefix(const std::string prefix);
  std::string GetFilePrefix() const;

protected:
  void DoDispose() override;

  std::string m_filePrefix;
};

class DataOutputCallback {
public:
  virtual ~DataOutputCallback() {}

  virtual void OutputStatistic(std::string key, std::string variable,
                               const StatisticalSummary *statSum) = 0;

  virtual void OutputSingleton(std::string key, std::string variable,
                               int val) = 0;

  virtual void OutputSingleton(std::string key, std::string variable,
                               uint32_t val) = 0;

  virtual void OutputSingleton(std::string key, std::string variable,
                               double val) = 0;

  virtual void OutputSingleton(std::string key, std::string variable,
                               std::string val) = 0;

  virtual void OutputSingleton(std::string key, std::string variable,
                               Time val) = 0;
};

}; // namespace ns3

#endif
