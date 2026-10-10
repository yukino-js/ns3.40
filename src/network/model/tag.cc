#include "tag.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(Tag);

TypeId Tag::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::Tag").SetParent<ObjectBase>().SetGroupName("Network");
  return tid;
}

} // namespace ns3
