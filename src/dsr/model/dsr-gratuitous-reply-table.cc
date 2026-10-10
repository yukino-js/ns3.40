
#include "dsr-gratuitous-reply-table.h"

#include "ns3/log.h"

#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("DsrGraReplyTable");

namespace dsr {

NS_OBJECT_ENSURE_REGISTERED(DsrGraReply);

TypeId DsrGraReply::GetTypeId() {
  static TypeId tid = TypeId("ns3::dsr::DsrGraReply")
                          .SetParent<Object>()
                          .SetGroupName("Dsr")
                          .AddConstructor<DsrGraReply>();
  return tid;
}

DsrGraReply::DsrGraReply() {}

DsrGraReply::~DsrGraReply() { NS_LOG_FUNCTION_NOARGS(); }

bool DsrGraReply::FindAndUpdate(Ipv4Address replyTo, Ipv4Address replyFrom,
                                Time gratReplyHoldoff) {
  Purge();
  for (auto i = m_graReply.begin(); i != m_graReply.end(); ++i) {
    if ((i->m_replyTo == replyTo) && (i->m_hearFrom == replyFrom)) {
      NS_LOG_DEBUG(
          "Update the reply to ip address if found the gratuitous reply entry");
      i->m_gratReplyHoldoff =
          std::max(gratReplyHoldoff + Simulator::Now(), i->m_gratReplyHoldoff);
      return true;
    }
  }
  return false;
}

bool DsrGraReply::AddEntry(GraReplyEntry &graTableEntry) {
  m_graReply.push_back(graTableEntry);
  return true;
}

void DsrGraReply::Purge() {
  m_graReply.erase(remove_if(m_graReply.begin(), m_graReply.end(), IsExpired()),
                   m_graReply.end());
}

} // namespace dsr
} // namespace ns3
