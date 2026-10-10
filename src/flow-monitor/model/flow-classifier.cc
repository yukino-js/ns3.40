
#include "flow-classifier.h"

namespace ns3 {

FlowClassifier::FlowClassifier() : m_lastNewFlowId(0) {}

FlowClassifier::~FlowClassifier() {}

FlowId FlowClassifier::GetNewFlowId() { return ++m_lastNewFlowId; }

} // namespace ns3
