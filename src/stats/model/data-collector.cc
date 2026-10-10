
#include "data-collector.h"

#include "data-calculator.h"

#include "ns3/log.h"
#include "ns3/object.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("DataCollector");

DataCollector::DataCollector() { NS_LOG_FUNCTION(this); }

DataCollector::~DataCollector() { NS_LOG_FUNCTION(this); }

TypeId DataCollector::GetTypeId() {
  static TypeId tid = TypeId("ns3::DataCollector")
                          .SetParent<Object>()
                          .SetGroupName("Stats")
                          .AddConstructor<DataCollector>();
  return tid;
}

void DataCollector::DoDispose() {
  NS_LOG_FUNCTION(this);

  m_calcList.clear();
  m_metadata.clear();

  Object::DoDispose();
}

void DataCollector::DescribeRun(std::string experiment, std::string strategy,
                                std::string input, std::string runID,
                                std::string description) {
  NS_LOG_FUNCTION(this << experiment << strategy << input << runID
                       << description);

  m_experimentLabel = experiment;
  m_strategyLabel = strategy;
  m_inputLabel = input;
  m_runLabel = runID;
  m_description = description;
}

void DataCollector::AddDataCalculator(Ptr<DataCalculator> datac) {
  NS_LOG_FUNCTION(this << datac);

  m_calcList.push_back(datac);
}

DataCalculatorList::iterator DataCollector::DataCalculatorBegin() {
  return m_calcList.begin();
}

DataCalculatorList::iterator DataCollector::DataCalculatorEnd() {
  return m_calcList.end();
}

void DataCollector::AddMetadata(std::string key, std::string value) {
  NS_LOG_FUNCTION(this << key << value);

  std::pair<std::string, std::string> blob(key, value);
  m_metadata.push_back(blob);
}

void DataCollector::AddMetadata(std::string key, uint32_t value) {
  NS_LOG_FUNCTION(this << key << value);

  std::stringstream st;
  st << value;

  std::pair<std::string, std::string> blob(key, st.str());
  m_metadata.push_back(blob);
}

void DataCollector::AddMetadata(std::string key, double value) {
  NS_LOG_FUNCTION(this << key << value);

  std::stringstream st;
  st << value;

  std::pair<std::string, std::string> blob(key, st.str());
  m_metadata.push_back(blob);
}

MetadataList::iterator DataCollector::MetadataBegin() {
  return m_metadata.begin();
}

MetadataList::iterator DataCollector::MetadataEnd() { return m_metadata.end(); }
