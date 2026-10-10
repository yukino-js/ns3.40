
#ifndef WIMAX_CONNECTION_H
#define WIMAX_CONNECTION_H

#include "cid.h"
#include "service-flow.h"
#include "wimax-mac-header.h"
#include "wimax-mac-queue.h"

#include "ns3/object.h"

#include <ostream>
#include <stdint.h>

namespace ns3 {

class ServiceFlow;
class Cid;

class WimaxConnection : public Object {
public:
  static TypeId GetTypeId();

  WimaxConnection(Cid cid, Cid::Type type);
  ~WimaxConnection() override;

  Cid GetCid() const;

  Cid::Type GetType() const;
  Ptr<WimaxMacQueue> GetQueue() const;
  void SetServiceFlow(ServiceFlow *serviceFlow);
  ServiceFlow *GetServiceFlow() const;

  uint8_t GetSchedulingType() const;
  bool Enqueue(Ptr<Packet> packet, const MacHeaderType &hdrType,
               const GenericMacHeader &hdr);
  Ptr<Packet> Dequeue(MacHeaderType::HeaderType packetType =
                          MacHeaderType::HEADER_TYPE_GENERIC);
  Ptr<Packet> Dequeue(MacHeaderType::HeaderType packetType,
                      uint32_t availableByte);
  bool HasPackets() const;
  bool HasPackets(MacHeaderType::HeaderType packetType) const;

  std::string GetTypeStr() const;

  typedef std::list<Ptr<const Packet>> FragmentsQueue;
  const FragmentsQueue GetFragmentsQueue() const;
  void FragmentEnqueue(Ptr<const Packet> fragment);
  void ClearFragmentsQueue();

private:
  void DoDispose() override;

  Cid m_cid;
  Cid::Type m_cidType;
  Ptr<WimaxMacQueue> m_queue;
  ServiceFlow *m_serviceFlow;

  FragmentsQueue m_fragmentsQueue;
};

} // namespace ns3

#endif
