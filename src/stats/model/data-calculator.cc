
#include "data-calculator.h"

#include "ns3/log.h"
#include "ns3/simulator.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("DataCalculator");

static double zero = 0;
const double ns3::NaN = zero / zero;

DataCalculator::DataCalculator() : m_enabled(true) { NS_LOG_FUNCTION(this); }

DataCalculator::~DataCalculator() { NS_LOG_FUNCTION(this); }

TypeId DataCalculator::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::DataCalculator").SetParent<Object>().SetGroupName("Stats");
  return tid;
}

void DataCalculator::DoDispose() {
  NS_LOG_FUNCTION(this);

  Simulator::Cancel(m_startEvent);
  Simulator::Cancel(m_stopEvent);

  Object::DoDispose();
}

void DataCalculator::SetKey(const std::string key) {
  NS_LOG_FUNCTION(this << key);

  m_key = key;
}

std::string DataCalculator::GetKey() const {
  NS_LOG_FUNCTION(this);

  return m_key;
}

void DataCalculator::SetContext(const std::string context) {
  NS_LOG_FUNCTION(this << context);

  m_context = context;
}

std::string DataCalculator::GetContext() const {
  NS_LOG_FUNCTION(this);

  return m_context;
}

void DataCalculator::Enable() {
  NS_LOG_FUNCTION(this);

  m_enabled = true;
}

void DataCalculator::Disable() {
  NS_LOG_FUNCTION(this);

  m_enabled = false;
}

bool DataCalculator::GetEnabled() const {
  NS_LOG_FUNCTION(this);

  return m_enabled;
}

void DataCalculator::Start(const Time &startTime) {
  NS_LOG_FUNCTION(this << startTime);

  m_startEvent = Simulator::Schedule(startTime, &DataCalculator::Enable, this);
}

void DataCalculator::Stop(const Time &stopTime) {
  NS_LOG_FUNCTION(this << stopTime);

  m_stopEvent = Simulator::Schedule(stopTime, &DataCalculator::Disable, this);
}
