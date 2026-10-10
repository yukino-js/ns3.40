
#include "uan-transducer.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(UanTransducer);

TypeId UanTransducer::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::UanTransducer").SetParent<Object>().SetGroupName("Uan");
  return tid;
}

} // namespace ns3
