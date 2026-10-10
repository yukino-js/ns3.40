
#ifndef DSR_HELPER_H
#define DSR_HELPER_H

#include "ns3/dsr-routing.h"
#include "ns3/icmpv4-l4-protocol.h"
#include "ns3/node-container.h"
#include "ns3/node.h"
#include "ns3/object-factory.h"
#include "ns3/tcp-l4-protocol.h"
#include "ns3/udp-l4-protocol.h"

namespace ns3 {

class DsrHelper {
public:
  DsrHelper();
  ~DsrHelper();

  DsrHelper &operator=(const DsrHelper &) = delete;

  DsrHelper(const DsrHelper &o);
  DsrHelper *Copy() const;
  Ptr<ns3::dsr::DsrRouting> Create(Ptr<Node> node) const;
  void Set(std::string name, const AttributeValue &value);

private:
  ObjectFactory m_agentFactory;
};

} // namespace ns3

#endif
