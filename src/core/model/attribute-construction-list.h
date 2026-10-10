#ifndef ATTRIBUTE_CONSTRUCTION_LIST_H
#define ATTRIBUTE_CONSTRUCTION_LIST_H

#include "attribute.h"

#include <list>

namespace ns3 {

class AttributeConstructionList {
public:
  struct Item {
    Ptr<const AttributeChecker> checker;
    Ptr<AttributeValue> value;
    std::string name;
  };

  typedef std::list<Item>::const_iterator CIterator;

  AttributeConstructionList();

  void Add(std::string name, Ptr<const AttributeChecker> checker,
           Ptr<AttributeValue> value);

  Ptr<AttributeValue> Find(Ptr<const AttributeChecker> checker) const;

  CIterator Begin() const;
  CIterator End() const;

private:
  std::list<Item> m_list;
};

} // namespace ns3

#endif
