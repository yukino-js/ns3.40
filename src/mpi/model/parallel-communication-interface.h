

#ifndef NS3_PARALLEL_COMMUNICATION_INTERFACE_H
#define NS3_PARALLEL_COMMUNICATION_INTERFACE_H

#include <ns3/buffer.h>
#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/packet.h>

#include <list>
#include <mpi.h>
#include <stdint.h>

namespace ns3 {

class ParallelCommunicationInterface {
public:
  virtual ~ParallelCommunicationInterface() {}

  virtual void Destroy() = 0;
  virtual uint32_t GetSystemId() = 0;
  virtual uint32_t GetSize() = 0;
  virtual bool IsEnabled() = 0;
  virtual void Enable(int *pargc, char ***pargv) = 0;
  virtual void Enable(MPI_Comm communicator) = 0;
  virtual void Disable() = 0;
  virtual void SendPacket(Ptr<Packet> p, const Time &rxTime, uint32_t node,
                          uint32_t dev) = 0;
  virtual MPI_Comm GetCommunicator() = 0;

private:
};

} // namespace ns3

#endif
