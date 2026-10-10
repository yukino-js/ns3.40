
#ifndef DSR_MAIN_HELPER_H
#define DSR_MAIN_HELPER_H

#include "dsr-helper.h"

#include "ns3/dsr-routing.h"
#include "ns3/node-container.h"
#include "ns3/node.h"
#include "ns3/object-factory.h"

namespace ns3 {

class DsrMainHelper {
public:
  DsrMainHelper();
  ~DsrMainHelper();
  DsrMainHelper(const DsrMainHelper &o);
  void Install(DsrHelper &dsrHelper, NodeContainer nodes);
  void SetDsrHelper(DsrHelper &dsrHelper);

private:
  void Install(Ptr<Node> node);
  DsrMainHelper &operator=(const DsrMainHelper &o);
  const DsrHelper *m_dsrHelper;
};

} // namespace ns3

#endif
