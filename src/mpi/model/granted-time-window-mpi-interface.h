

#ifndef NS3_GRANTED_TIME_WINDOW_MPI_INTERFACE_H
#define NS3_GRANTED_TIME_WINDOW_MPI_INTERFACE_H

#include "parallel-communication-interface.h"

#include "ns3/buffer.h"
#include "ns3/nstime.h"

#include <atomic>
#include <list>
#include <mpi.h>
#include <stdint.h>

namespace ns3 {

const uint32_t MAX_MPI_MSG_SIZE = 2000;

class SentBuffer {
public:
  SentBuffer();
  ~SentBuffer();

  uint8_t *GetBuffer();
  void SetBuffer(uint8_t *buffer);
  MPI_Request *GetRequest();

private:
  uint8_t *m_buffer;
  MPI_Request m_request;
};

class Packet;
class DistributedSimulatorImpl;
class HybridSimulatorImpl;

class GrantedTimeWindowMpiInterface : public ParallelCommunicationInterface,
                                      Object {
public:
  static TypeId GetTypeId();

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
  friend ns3::DistributedSimulatorImpl;
  friend ns3::HybridSimulatorImpl;

  static void ReceiveMessages();
  static void TestSendComplete();
  static uint32_t GetRxCount();
  static uint32_t GetTxCount();

  static uint32_t g_sid;
  static uint32_t g_size;

  static uint32_t g_rxCount;

  static uint32_t g_txCount;

  static bool g_enabled;

  static bool g_mpiInitCalled;

  static MPI_Request *g_requests;

  static char **g_pRxBuffers;

  static std::list<SentBuffer> g_pendingTx;

  static MPI_Comm g_communicator;

  static bool g_freeCommunicator;

#ifdef NS3_MTP
  static std::atomic<bool> g_sending;
#endif
};

} // namespace ns3

#endif
