
#ifndef DSR_GRATUITOUS_REPLY_TABLE_H
#define DSR_GRATUITOUS_REPLY_TABLE_H

#include "ns3/callback.h"
#include "ns3/ipv4-address.h"
#include "ns3/simulator.h"
#include "ns3/timer.h"

#include <vector>

namespace ns3 {
namespace dsr {
struct GraReplyEntry {
  Ipv4Address m_replyTo;
  Ipv4Address m_hearFrom;
  Time m_gratReplyHoldoff;

  GraReplyEntry(Ipv4Address t, Ipv4Address f, Time h)
      : m_replyTo(t), m_hearFrom(f), m_gratReplyHoldoff(h) {}
};

class DsrGraReply : public Object {
public:
  static TypeId GetTypeId();

  DsrGraReply();
  ~DsrGraReply() override;

  void SetGraTableSize(uint32_t g) { GraReplyTableSize = g; }

  uint32_t GetGraTableSize() const { return GraReplyTableSize; }

  bool AddEntry(GraReplyEntry &graTableEntry);
  bool FindAndUpdate(Ipv4Address replyTo, Ipv4Address replyFrom,
                     Time gratReplyHoldoff);
  void Purge();

  void Clear() { m_graReply.clear(); }

private:
  std::vector<GraReplyEntry> m_graReply;
  uint32_t GraReplyTableSize;

  struct IsExpired {
    bool operator()(const GraReplyEntry &b) const {
      return (b.m_gratReplyHoldoff < Simulator::Now());
    }
  };
};
} // namespace dsr
} // namespace ns3

#endif
