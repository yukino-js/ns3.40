
#ifndef DATA_COLLECTOR_H
#define DATA_COLLECTOR_H

#include "ns3/object.h"

#include <list>
#include <string>

namespace ns3 {

class DataCalculator;

typedef std::list<Ptr<DataCalculator>> DataCalculatorList;
typedef std::list<std::pair<std::string, std::string>> MetadataList;

class DataCollector : public Object {
public:
  DataCollector();
  ~DataCollector() override;

  static TypeId GetTypeId();

  void DescribeRun(std::string experiment, std::string strategy,
                   std::string input, std::string runID,
                   std::string description = "");

  std::string GetExperimentLabel() const { return m_experimentLabel; }

  std::string GetStrategyLabel() const { return m_strategyLabel; }

  std::string GetInputLabel() const { return m_inputLabel; }

  std::string GetRunLabel() const { return m_runLabel; }

  std::string GetDescription() const { return m_description; }

  void AddMetadata(std::string key, std::string value);
  void AddMetadata(std::string key, double value);
  void AddMetadata(std::string key, uint32_t value);
  MetadataList::iterator MetadataBegin();
  MetadataList::iterator MetadataEnd();

  void AddDataCalculator(Ptr<DataCalculator> datac);
  DataCalculatorList::iterator DataCalculatorBegin();
  DataCalculatorList::iterator DataCalculatorEnd();

protected:
  void DoDispose() override;

private:
  std::string m_experimentLabel;
  std::string m_strategyLabel;
  std::string m_inputLabel;
  std::string m_runLabel;
  std::string m_description;

  MetadataList m_metadata;
  DataCalculatorList m_calcList;
};

}; // namespace ns3

#endif
