

#include "attribute-default-iterator.h"

#include "ns3/attribute.h"
#include "ns3/callback.h"
#include "ns3/global-value.h"
#include "ns3/object-ptr-container.h"
#include "ns3/pointer.h"
#include "ns3/string.h"

namespace ns3 {

AttributeDefaultIterator::~AttributeDefaultIterator() {}

void AttributeDefaultIterator::Iterate() {
  for (uint32_t i = 0; i < TypeId::GetRegisteredN(); i++) {
    TypeId tid = TypeId::GetRegistered(i);
    if (tid.MustHideFromDocumentation()) {
      continue;
    }
    bool calledStart = false;
    for (uint32_t j = 0; j < tid.GetAttributeN(); j++) {
      TypeId::AttributeInformation info = tid.GetAttribute(j);
      if (!(info.flags & TypeId::ATTR_CONSTRUCT)) {
        continue;
      }
      if (!info.accessor) {
        continue;
      }
      if (!info.accessor->HasSetter()) {
        continue;
      }
      if (!info.checker) {
        continue;
      }
      if (!info.initialValue) {
        continue;
      }
      Ptr<const ObjectPtrContainerValue> vector =
          DynamicCast<const ObjectPtrContainerValue>(info.initialValue);
      if (vector) {
        continue;
      }
      Ptr<const PointerValue> pointer =
          DynamicCast<const PointerValue>(info.initialValue);
      if (pointer) {
        continue;
      }
      Ptr<const CallbackValue> callback =
          DynamicCast<const CallbackValue>(info.initialValue);
      if (callback) {
        continue;
      }
      if (!calledStart) {
        StartVisitTypeId(tid.GetName());
      }
      VisitAttribute(tid, info.name,
                     info.initialValue->SerializeToString(info.checker), j);
      calledStart = true;
    }
    if (calledStart) {
      EndVisitTypeId();
    }
  }
}

void AttributeDefaultIterator::StartVisitTypeId(std::string name) {}

void AttributeDefaultIterator::EndVisitTypeId() {}

void AttributeDefaultIterator::DoVisitAttribute(std::string name,
                                                std::string defaultValue) {}

void AttributeDefaultIterator::VisitAttribute(TypeId tid, std::string name,
                                              std::string defaultValue,
                                              uint32_t index) {
  DoVisitAttribute(name, defaultValue);
}

} // namespace ns3
