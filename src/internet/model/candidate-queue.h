
#ifndef CANDIDATE_QUEUE_H
#define CANDIDATE_QUEUE_H

#include "ns3/ipv4-address.h"

#include <list>
#include <stdint.h>

namespace ns3 {

class SPFVertex;

class CandidateQueue {
public:
  CandidateQueue();

  virtual ~CandidateQueue();

  CandidateQueue(const CandidateQueue &) = delete;
  CandidateQueue &operator=(const CandidateQueue &) = delete;

  void Clear();

  void Push(SPFVertex *vNew);

  SPFVertex *Pop();

  SPFVertex *Top() const;

  bool Empty() const;

  uint32_t Size() const;

  SPFVertex *Find(const Ipv4Address addr) const;

  void Reorder();

private:
  static bool CompareSPFVertex(const SPFVertex *v1, const SPFVertex *v2);

  typedef std::list<SPFVertex *> CandidateList_t;
  CandidateList_t m_candidates;

  friend std::ostream &operator<<(std::ostream &os, const CandidateQueue &q);
};

} // namespace ns3

#endif
