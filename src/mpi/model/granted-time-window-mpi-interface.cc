

#include "granted-time-window-mpi-interface.h"

#include "mpi-interface.h"
#include "mpi-receiver.h"

#include "ns3/log.h"
#include "ns3/net-device.h"
#include "ns3/node-list.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/simulator-impl.h"
#include "ns3/simulator.h"
#ifdef NS3_MTP
#include "ns3/mtp-interface.h"
#endif

#include <iomanip>
#include <iostream>
#include <list>
#include <mpi.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("GrantedTimeWindowMpiInterface");

NS_OBJECT_ENSURE_REGISTERED(GrantedTimeWindowMpiInterface);

SentBuffer::SentBuffer() {
  m_buffer = nullptr;
  m_request = nullptr;
}

SentBuffer::~SentBuffer() { delete[] m_buffer; }

uint8_t *SentBuffer::GetBuffer() { return m_buffer; }

void SentBuffer::SetBuffer(uint8_t *buffer) { m_buffer = buffer; }

MPI_Request *SentBuffer::GetRequest() { return &m_request; }

uint32_t GrantedTimeWindowMpiInterface::g_sid = 0;
uint32_t GrantedTimeWindowMpiInterface::g_size = 1;
bool GrantedTimeWindowMpiInterface::g_enabled = false;
bool GrantedTimeWindowMpiInterface::g_mpiInitCalled = false;
uint32_t GrantedTimeWindowMpiInterface::g_rxCount = 0;
uint32_t GrantedTimeWindowMpiInterface::g_txCount = 0;
std::list<SentBuffer> GrantedTimeWindowMpiInterface::g_pendingTx;

MPI_Request *GrantedTimeWindowMpiInterface::g_requests;
char **GrantedTimeWindowMpiInterface::g_pRxBuffers;
MPI_Comm GrantedTimeWindowMpiInterface::g_communicator = MPI_COMM_WORLD;
bool GrantedTimeWindowMpiInterface::g_freeCommunicator = false;

#ifdef NS3_MTP
std::atomic<bool> GrantedTimeWindowMpiInterface::g_sending(false);
#endif

TypeId GrantedTimeWindowMpiInterface::GetTypeId() {
  static TypeId tid = TypeId("ns3::GrantedTimeWindowMpiInterface")
                          .SetParent<Object>()
                          .SetGroupName("Mpi");
  return tid;
}

void GrantedTimeWindowMpiInterface::Destroy() {
  NS_LOG_FUNCTION(this);

  for (uint32_t i = 0; i < GetSize(); ++i) {
    delete[] g_pRxBuffers[i];
  }
  delete[] g_pRxBuffers;
  delete[] g_requests;

  g_pendingTx.clear();
}

uint32_t GrantedTimeWindowMpiInterface::GetRxCount() {
  NS_ASSERT(g_enabled);
  return g_rxCount;
}

uint32_t GrantedTimeWindowMpiInterface::GetTxCount() {
  NS_ASSERT(g_enabled);
  return g_txCount;
}

uint32_t GrantedTimeWindowMpiInterface::GetSystemId() {
  NS_ASSERT(g_enabled);
  return g_sid;
}

uint32_t GrantedTimeWindowMpiInterface::GetSize() {
  NS_ASSERT(g_enabled);
  return g_size;
}

bool GrantedTimeWindowMpiInterface::IsEnabled() { return g_enabled; }

MPI_Comm GrantedTimeWindowMpiInterface::GetCommunicator() {
  NS_ASSERT(g_enabled);
  return g_communicator;
}

void GrantedTimeWindowMpiInterface::Enable(int *pargc, char ***pargv) {
  NS_LOG_FUNCTION(this << pargc << pargv);

  NS_ASSERT(g_enabled == false);

  MPI_Init(pargc, pargv);
  Enable(MPI_COMM_WORLD);
  g_mpiInitCalled = true;
  g_enabled = true;
}

void GrantedTimeWindowMpiInterface::Enable(MPI_Comm communicator) {
  NS_LOG_FUNCTION(this);

  NS_ASSERT(g_enabled == false);

  MPI_Comm_dup(communicator, &g_communicator);
  g_freeCommunicator = true;

  MPI_Barrier(g_communicator);

  int mpiSystemId;
  int mpiSize;
  MPI_Comm_rank(g_communicator, &mpiSystemId);
  MPI_Comm_size(g_communicator, &mpiSize);
  g_sid = mpiSystemId;
  g_size = mpiSize;

  g_enabled = true;
  g_pRxBuffers = new char *[g_size];
  g_requests = new MPI_Request[g_size];
  for (uint32_t i = 0; i < GetSize(); ++i) {
    g_pRxBuffers[i] = new char[MAX_MPI_MSG_SIZE];
    MPI_Irecv(g_pRxBuffers[i], MAX_MPI_MSG_SIZE, MPI_CHAR, MPI_ANY_SOURCE, 0,
              g_communicator, &g_requests[i]);
  }
}

void GrantedTimeWindowMpiInterface::SendPacket(Ptr<Packet> p,
                                               const Time &rxTime,
                                               uint32_t node, uint32_t dev) {
  NS_LOG_FUNCTION(this << p << rxTime.GetTimeStep() << node << dev);

#ifdef NS3_MTP
  while (g_sending.exchange(true, std::memory_order_acquire))
    ;
#endif

  SentBuffer sendBuf;
  g_pendingTx.push_back(sendBuf);
  auto i = g_pendingTx.rbegin();

  uint32_t serializedSize = p->GetSerializedSize();
  auto buffer = new uint8_t[serializedSize + 16];
  i->SetBuffer(buffer);
  uint64_t t = rxTime.GetInteger();
  auto pTime = reinterpret_cast<uint64_t *>(buffer);
  *pTime++ = t;
  auto pData = reinterpret_cast<uint32_t *>(pTime);
  *pData++ = node;
  *pData++ = dev;
  p->Serialize(reinterpret_cast<uint8_t *>(pData), serializedSize);

  Ptr<Node> destNode = NodeList::GetNode(node);
#ifdef NS3_MTP
  uint32_t nodeSysId = destNode->GetSystemId() & 0xFFFF;
#else
  uint32_t nodeSysId = destNode->GetSystemId();
#endif

  MPI_Isend(reinterpret_cast<void *>(i->GetBuffer()), serializedSize + 16,
            MPI_CHAR, nodeSysId, 0, g_communicator, (i->GetRequest()));
  g_txCount++;

#ifdef NS3_MTP
  g_sending.store(false, std::memory_order_release);
#endif
}

void GrantedTimeWindowMpiInterface::ReceiveMessages() {
  NS_LOG_FUNCTION_NOARGS();

  while (true) {
    int flag = 0;
    int index = 0;
    MPI_Status status;

    MPI_Testany(MpiInterface::GetSize(), g_requests, &index, &flag, &status);
    if (!flag) {
      break;
    }
    int count;
    MPI_Get_count(&status, MPI_CHAR, &count);
    g_rxCount++;

    auto pTime = reinterpret_cast<uint64_t *>(g_pRxBuffers[index]);
    uint64_t time = *pTime++;
    auto pData = reinterpret_cast<uint32_t *>(pTime);
    uint32_t node = *pData++;
    uint32_t dev = *pData++;

    Time rxTime(time);

    count -= sizeof(time) + sizeof(node) + sizeof(dev);

    Ptr<Packet> p =
        Create<Packet>(reinterpret_cast<uint8_t *>(pData), count, true);

    Ptr<Node> pNode = NodeList::GetNode(node);
    Ptr<MpiReceiver> pMpiRec = nullptr;
    uint32_t nDevices = pNode->GetNDevices();
    for (uint32_t i = 0; i < nDevices; ++i) {
      Ptr<NetDevice> pThisDev = pNode->GetDevice(i);
      if (pThisDev->GetIfIndex() == dev) {
        pMpiRec = pThisDev->GetObject<MpiReceiver>();
        break;
      }
    }

    NS_ASSERT(pNode && pMpiRec);

#ifdef NS3_MTP
    MtpInterface::GetSystem(pNode->GetSystemId() >> 16)
        ->ScheduleAt(pNode->GetId(), rxTime,
                     MakeEvent(&MpiReceiver::Receive, pMpiRec, p));
#else
    Simulator::ScheduleWithContext(pNode->GetId(), rxTime - Simulator::Now(),
                                   &MpiReceiver::Receive, pMpiRec, p);
#endif

    MPI_Irecv(g_pRxBuffers[index], MAX_MPI_MSG_SIZE, MPI_CHAR, MPI_ANY_SOURCE,
              0, g_communicator, &g_requests[index]);
  }
}

void GrantedTimeWindowMpiInterface::TestSendComplete() {
  NS_LOG_FUNCTION_NOARGS();

  auto i = g_pendingTx.begin();
  while (i != g_pendingTx.end()) {
    MPI_Status status;
    int flag = 0;
    MPI_Test(i->GetRequest(), &flag, &status);
    auto current = i;
    i++;
    if (flag) {
      g_pendingTx.erase(current);
    }
  }
}

void GrantedTimeWindowMpiInterface::Disable() {
  NS_LOG_FUNCTION_NOARGS();

  if (g_freeCommunicator) {
    MPI_Comm_free(&g_communicator);
    g_freeCommunicator = false;
  }

  if (g_mpiInitCalled) {
    int flag = 0;
    MPI_Initialized(&flag);
    if (flag) {
      MPI_Finalize();
    } else {
      NS_FATAL_ERROR(
          "Cannot disable MPI environment without Initializing it first");
    }
    g_mpiInitCalled = false;
  }

  g_enabled = false;
}

} // namespace ns3
