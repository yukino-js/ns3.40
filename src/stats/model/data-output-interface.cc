
#include "data-output-interface.h"

#include "ns3/log.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("DataOutputInterface");

DataOutputInterface::DataOutputInterface() { NS_LOG_FUNCTION(this); }

DataOutputInterface::~DataOutputInterface() { NS_LOG_FUNCTION(this); }

TypeId DataOutputInterface::GetTypeId() {
  static TypeId tid = TypeId("ns3::DataOutputInterface")
                          .SetParent<Object>()
                          .SetGroupName("Stats");
  return tid;
}

void DataOutputInterface::DoDispose() {
  NS_LOG_FUNCTION(this);

  Object::DoDispose();
}

void DataOutputInterface::SetFilePrefix(const std::string prefix) {
  NS_LOG_FUNCTION(this << prefix);

  m_filePrefix = prefix;
}

std::string DataOutputInterface::GetFilePrefix() const {
  NS_LOG_FUNCTION(this);

  return m_filePrefix;
}
