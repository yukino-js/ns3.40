

#ifndef ATTRIBUTE_ITERATOR_H
#define ATTRIBUTE_ITERATOR_H

#include "ns3/object-ptr-container.h"
#include "ns3/object.h"
#include "ns3/ptr.h"

#include <vector>

namespace ns3 {

class AttributeIterator {
public:
  AttributeIterator();
  virtual ~AttributeIterator();

  void Iterate();

protected:
  std::string GetCurrentPath() const;

private:
  virtual void DoVisitAttribute(Ptr<Object> object, std::string name) = 0;
  virtual void DoStartVisitObject(Ptr<Object> object);
  virtual void DoEndVisitObject();
  virtual void DoStartVisitPointerAttribute(Ptr<Object> object,
                                            std::string name,
                                            Ptr<Object> value);
  virtual void DoEndVisitPointerAttribute();
  virtual void
  DoStartVisitArrayAttribute(Ptr<Object> object, std::string name,
                             const ObjectPtrContainerValue &vector);
  virtual void DoEndVisitArrayAttribute();
  virtual void DoStartVisitArrayItem(const ObjectPtrContainerValue &vector,
                                     uint32_t index, Ptr<Object> item);
  virtual void DoEndVisitArrayItem();

  void DoIterate(Ptr<Object> object);
  bool IsExamined(Ptr<const Object> object);
  std::string GetCurrentPath(std::string attr) const;

  void VisitAttribute(Ptr<Object> object, std::string name);
  void StartVisitObject(Ptr<Object> object);
  void EndVisitObject();
  void StartVisitPointerAttribute(Ptr<Object> object, std::string name,
                                  Ptr<Object> value);
  void EndVisitPointerAttribute();
  void StartVisitArrayAttribute(Ptr<Object> object, std::string name,
                                const ObjectPtrContainerValue &vector);
  void EndVisitArrayAttribute();
  void StartVisitArrayItem(const ObjectPtrContainerValue &vector,
                           uint32_t index, Ptr<Object> item);
  void EndVisitArrayItem();

  std::vector<Ptr<Object>> m_examined;
  std::vector<std::string> m_currentPath;
};

} // namespace ns3

#endif
