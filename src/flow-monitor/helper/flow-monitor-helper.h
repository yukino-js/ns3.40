#ifndef FLOW_MONITOR_HELPER_H
#define FLOW_MONITOR_HELPER_H

#include "ns3/flow-classifier.h"
#include "ns3/flow-monitor.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"

#include <string>

namespace ns3 {

class AttributeValue;
class Ipv4FlowClassifier;
class Ipv6FlowClassifier;

class FlowMonitorHelper {
public:
  FlowMonitorHelper();
  ~FlowMonitorHelper();

  FlowMonitorHelper(const FlowMonitorHelper &) = delete;
  FlowMonitorHelper &operator=(const FlowMonitorHelper &) = delete;

  void SetMonitorAttribute(std::string n1, const AttributeValue &v1);

  Ptr<FlowMonitor> Install(NodeContainer nodes);
  Ptr<FlowMonitor> Install(Ptr<Node> node);
  Ptr<FlowMonitor> InstallAll();

  Ptr<FlowMonitor> GetMonitor();

  Ptr<FlowClassifier> GetClassifier();

  Ptr<FlowClassifier> GetClassifier6();

  void SerializeToXmlStream(std::ostream &os, uint16_t indent,
                            bool enableHistograms, bool enableProbes);

  std::string SerializeToXmlString(uint16_t indent, bool enableHistograms,
                                   bool enableProbes);

  void SerializeToXmlFile(std::string fileName, bool enableHistograms,
                          bool enableProbes);

private:
  ObjectFactory m_monitorFactory;
  Ptr<FlowMonitor> m_flowMonitor;
  Ptr<FlowClassifier> m_flowClassifier4;
  Ptr<FlowClassifier> m_flowClassifier6;
};

} // namespace ns3

#endif
