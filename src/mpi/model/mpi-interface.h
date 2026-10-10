

#ifndef NS3_MPI_INTERFACE_H
#define NS3_MPI_INTERFACE_H

#include <ns3/nstime.h>
#include <ns3/packet.h>

#include <mpi.h>

namespace ns3 {

class ParallelCommunicationInterface;

class MpiInterface {
public:
  static void Destroy();
  static uint32_t GetSystemId();
  static uint32_t GetSize();
  static bool IsEnabled();
  static void Enable(int *pargc, char ***pargv);
  static void Enable(MPI_Comm communicator);
  static void Disable();
  static void SendPacket(Ptr<Packet> p, const Time &rxTime, uint32_t node,
                         uint32_t dev);

  static MPI_Comm GetCommunicator();

private:
  static void SetParallelSimulatorImpl();

  static ParallelCommunicationInterface *g_parallelCommunicationInterface;
};

} // namespace ns3

#endif
