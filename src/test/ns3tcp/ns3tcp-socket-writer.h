
#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/node.h"
#include "ns3/ptr.h"
#include "ns3/socket.h"

namespace ns3 {

class SocketWriter : public Application {
public:
  SocketWriter();
  ~SocketWriter() override;
  static TypeId GetTypeId();

  void Setup(Ptr<Node> node, Address peer);
  void Connect();
  void Write(uint32_t numBytes);
  void Close();

private:
  void StartApplication() override;
  void StopApplication() override;
  Address m_peer;
  Ptr<Node> m_node;
  Ptr<Socket> m_socket;
  bool m_isSetup;
  bool m_isConnected;
};
} // namespace ns3
