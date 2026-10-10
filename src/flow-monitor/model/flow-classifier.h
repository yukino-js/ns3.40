
#ifndef FLOW_CLASSIFIER_H
#define FLOW_CLASSIFIER_H

#include "ns3/simple-ref-count.h"

#include <ostream>

namespace ns3 {

typedef uint32_t FlowId;

typedef uint32_t FlowPacketId;

class FlowClassifier : public SimpleRefCount<FlowClassifier> {
private:
  FlowId m_lastNewFlowId;

public:
  FlowClassifier();
  virtual ~FlowClassifier();

  FlowClassifier(const FlowClassifier &) = delete;
  FlowClassifier &operator=(const FlowClassifier &) = delete;

  virtual void SerializeToXmlStream(std::ostream &os,
                                    uint16_t indent) const = 0;

protected:
  FlowId GetNewFlowId();

  void Indent(std::ostream &os, uint16_t level) const;
};

inline void FlowClassifier::Indent(std::ostream &os, uint16_t level) const {
  for (uint16_t __xpto = 0; __xpto < level; __xpto++) {
    os << ' ';
  }
}

} // namespace ns3

#endif
