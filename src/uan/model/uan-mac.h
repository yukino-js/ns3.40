
#ifndef UAN_MAC_H
#define UAN_MAC_H

#include "ns3/address.h"
#include "ns3/mac8-address.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"

namespace ns3 {

class UanPhy;
class UanChannel;
class UanNetDevice;
class UanTransducer;
class UanTxMode;
class Mac8Address;

class UanMac : public Object {
public:
  UanMac();
  static TypeId GetTypeId();

  virtual Address GetAddress();

  virtual void SetAddress(Mac8Address addr);

  virtual bool Enqueue(Ptr<Packet> pkt, uint16_t protocolNumber,
                       const Address &dest) = 0;
  virtual void SetForwardUpCb(
      Callback<void, Ptr<Packet>, uint16_t, const Mac8Address &> cb) = 0;

  virtual void AttachPhy(Ptr<UanPhy> phy) = 0;

  virtual Address GetBroadcast() const;

  virtual void Clear() = 0;

  virtual int64_t AssignStreams(int64_t stream) = 0;

  typedef void (*PacketModeTracedCallback)(Ptr<const Packet> packet,
                                           UanTxMode mode);

  uint32_t GetTxModeIndex() const;

  void SetTxModeIndex(uint32_t txModeIndex);

private:
  uint32_t m_txModeIndex;
  Mac8Address m_address;
};

} // namespace ns3

#endif
