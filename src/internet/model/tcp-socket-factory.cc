#include "tcp-socket-factory.h"

#include "ns3/double.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(TcpSocketFactory);

TypeId TcpSocketFactory::GetTypeId() {
  static TypeId tid = TypeId("ns3::TcpSocketFactory")
                          .SetParent<SocketFactory>()
                          .SetGroupName("Internet");
  return tid;
}

} // namespace ns3
