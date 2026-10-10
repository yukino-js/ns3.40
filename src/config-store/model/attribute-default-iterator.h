

#ifndef ATTRIBUTE_DEFAULT_ITERATOR_H
#define ATTRIBUTE_DEFAULT_ITERATOR_H

#include "ns3/type-id.h"

#include <string>

namespace ns3 {

class AttributeDefaultIterator {
public:
  virtual ~AttributeDefaultIterator() = 0;
  void Iterate();

private:
  virtual void StartVisitTypeId(std::string name);
  virtual void EndVisitTypeId();
  virtual void VisitAttribute(TypeId tid, std::string name,
                              std::string defaultValue, uint32_t index);
  virtual void DoVisitAttribute(std::string name, std::string defaultValue);
};

} // namespace ns3

#endif
