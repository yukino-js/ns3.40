#include "attribute-construction-list.h"

#include "log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("AttributeConstructionList");

AttributeConstructionList::AttributeConstructionList() {
  NS_LOG_FUNCTION(this);
}

void AttributeConstructionList::Add(std::string name,
                                    Ptr<const AttributeChecker> checker,
                                    Ptr<AttributeValue> value) {
  NS_LOG_FUNCTION(this << name << checker << value);

  for (auto k = m_list.begin(); k != m_list.end(); k++) {
    if (k->checker == checker) {
      m_list.erase(k);
      break;
    }
  }
  Item attr;
  attr.checker = checker;
  attr.value = value;
  attr.name = name;
  m_list.push_back(attr);
}

Ptr<AttributeValue>
AttributeConstructionList::Find(Ptr<const AttributeChecker> checker) const {
  NS_LOG_FUNCTION(this << checker);
  for (auto k = m_list.begin(); k != m_list.end(); k++) {
    NS_LOG_DEBUG("Found " << k->name << " " << k->checker << " " << k->value);
    if (k->checker == checker) {
      return k->value;
    }
  }
  return nullptr;
}

AttributeConstructionList::CIterator AttributeConstructionList::Begin() const {
  NS_LOG_FUNCTION(this);
  return m_list.begin();
}

AttributeConstructionList::CIterator AttributeConstructionList::End() const {
  NS_LOG_FUNCTION(this);
  return m_list.end();
}

} // namespace ns3
