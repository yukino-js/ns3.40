#ifndef TCPERRORCHANNEL_H
#define TCPERRORCHANNEL_H

#include "ns3/error-model.h"
#include "ns3/ipv4-header.h"
#include "ns3/tcp-header.h"

namespace ns3 {

class TcpGeneralErrorModel : public ErrorModel {
public:
  static TypeId GetTypeId();
  TcpGeneralErrorModel();

  void SetDropCallback(
      Callback<void, const Ipv4Header &, const TcpHeader &, Ptr<const Packet>>
          cb) {
    m_dropCallback = cb;
  }

protected:
  virtual bool ShouldDrop(const Ipv4Header &ipHeader,
                          const TcpHeader &tcpHeader, uint32_t packetSize) = 0;

private:
  bool DoCorrupt(Ptr<Packet> p) override;
  Callback<void, const Ipv4Header &, const TcpHeader &, Ptr<const Packet>>
      m_dropCallback;
};

class TcpSeqErrorModel : public TcpGeneralErrorModel {
public:
  static TypeId GetTypeId();

  TcpSeqErrorModel() : TcpGeneralErrorModel() {}

  void AddSeqToKill(const SequenceNumber32 &seq) {
    m_seqToKill.insert(m_seqToKill.end(), seq);
  }

protected:
  bool ShouldDrop(const Ipv4Header &ipHeader, const TcpHeader &tcpHeader,
                  uint32_t packetSize) override;

protected:
  std::list<SequenceNumber32> m_seqToKill;

private:
  void DoReset() override;
};

class TcpFlagErrorModel : public TcpGeneralErrorModel {
public:
  static TypeId GetTypeId();
  TcpFlagErrorModel();

  void SetFlagToKill(TcpHeader::Flags_t flags) { m_flagsToKill = flags; }

  void SetKillRepeat(int16_t killNumber) { m_killNumber = killNumber; }

protected:
  bool ShouldDrop(const Ipv4Header &ipHeader, const TcpHeader &tcpHeader,
                  uint32_t packetSize) override;

protected:
  TcpHeader::Flags_t m_flagsToKill;
  int16_t m_killNumber;

private:
  void DoReset() override;
};

} // namespace ns3

#endif
