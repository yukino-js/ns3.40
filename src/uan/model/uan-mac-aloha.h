
#ifndef UAN_MAC_ALOHA_H
#define UAN_MAC_ALOHA_H

#include "uan-mac.h"

#include "ns3/mac8-address.h"

namespace ns3 {

class UanPhy;
class UanTxMode;

class UanMacAloha : public UanMac {
public:
  UanMacAloha();
  ~UanMacAloha() override;
  static TypeId GetTypeId();

  bool Enqueue(Ptr<Packet> pkt, uint16_t protocolNumber,
               const Address &dest) override;
  void SetForwardUpCb(
      Callback<void, Ptr<Packet>, uint16_t, const Mac8Address &> cb) override;
  void AttachPhy(Ptr<UanPhy> phy) override;
  void Clear() override;
  int64_t AssignStreams(int64_t stream) override;

private:
  Ptr<UanPhy> m_phy;
  Callback<void, Ptr<Packet>, uint16_t, const Mac8Address &> m_forUpCb;
  bool m_cleared;

  void RxPacketGood(Ptr<Packet> pkt, double sinr, UanTxMode txMode);

  void RxPacketError(Ptr<Packet> pkt, double sinr);

protected:
  void DoDispose() override;
};

} // namespace ns3

#endif
