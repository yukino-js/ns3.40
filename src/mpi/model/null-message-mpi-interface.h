

#ifndef NS3_NULLMESSAGE_MPI_INTERFACE_H
#define NS3_NULLMESSAGE_MPI_INTERFACE_H

#include "parallel-communication-interface.h"

#include <ns3/buffer.h>
#include <ns3/nstime.h>

#include <list>
#include <mpi.h>

namespace ns3 {

class NullMessageSimulatorImpl;
class NullMessageSentBuffer;
class RemoteChannelBundle;
class Packet;

class NullMessageMpiInterface : public ParallelCommunicationInterface, Object {
public:
  static TypeId GetTypeId();

  NullMessageMpiInterface();
  ~NullMessageMpiInterface() override;

  void Destroy() override;
  uint32_t GetSystemId() override;
  uint32_t GetSize() override;
  bool IsEnabled() override;
  void Enable(int *pargc, char ***pargv) override;
  void Enable(MPI_Comm communicator) override;
  void Disable() override;
  void SendPacket(Ptr<Packet> p, const Time &rxTime, uint32_t node,
                  uint32_t dev) override;
  MPI_Comm GetCommunicator() override;

private:
  friend ns3::RemoteChannelBundle;
  friend ns3::NullMessageSimulatorImpl;

  static void SendNullMessage(const Time &guaranteeUpdate,
                              Ptr<RemoteChannelBundle> bundle);
  static void ReceiveMessagesNonBlocking();
  static void ReceiveMessagesBlocking();
  static void TestSendComplete();

  static void InitializeSendReceiveBuffers();

  static void ReceiveMessages(bool blocking = false);

  static uint32_t g_sid;

  static uint32_t g_size;

  static uint32_t g_numNeighbors;

  static bool g_enabled;

  static bool g_mpiInitCalled;

  static MPI_Request *g_requests;

  static char **g_pRxBuffers;

  static std::list<NullMessageSentBuffer> g_pendingTx;

  static MPI_Comm g_communicator;

  static bool g_freeCommunicator;
};

} // namespace ns3

#endif
