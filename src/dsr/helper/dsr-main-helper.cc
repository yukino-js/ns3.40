
#include "dsr-main-helper.h"

#include "dsr-helper.h"

#include "ns3/dsr-rcache.h"
#include "ns3/dsr-routing.h"
#include "ns3/dsr-rreq-table.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/node-list.h"
#include "ns3/node.h"
#include "ns3/ptr.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("DsrMainHelper");

DsrMainHelper::DsrMainHelper() : m_dsrHelper(nullptr) { NS_LOG_FUNCTION(this); }

DsrMainHelper::DsrMainHelper(const DsrMainHelper &o) {
  NS_LOG_FUNCTION(this);
  m_dsrHelper = o.m_dsrHelper->Copy();
}

DsrMainHelper::~DsrMainHelper() {
  NS_LOG_FUNCTION(this);
  delete m_dsrHelper;
}

DsrMainHelper &DsrMainHelper::operator=(const DsrMainHelper &o) {
  if (this == &o) {
    return *this;
  }
  m_dsrHelper = o.m_dsrHelper->Copy();
  return *this;
}

void DsrMainHelper::Install(DsrHelper &dsrHelper, NodeContainer nodes) {
  NS_LOG_DEBUG("Passed node container");
  delete m_dsrHelper;
  m_dsrHelper = dsrHelper.Copy();
  for (auto i = nodes.Begin(); i != nodes.End(); ++i) {
    Install(*i);
  }
}

void DsrMainHelper::Install(Ptr<Node> node) {
  NS_LOG_FUNCTION(node);
  Ptr<ns3::dsr::DsrRouting> dsr = m_dsrHelper->Create(node);
  dsr->SetNode(node);
}

void DsrMainHelper::SetDsrHelper(DsrHelper &dsrHelper) {
  NS_LOG_FUNCTION(this);
  delete m_dsrHelper;
  m_dsrHelper = dsrHelper.Copy();
}

} // namespace ns3
