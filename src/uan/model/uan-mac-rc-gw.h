
#ifndef UAN_MAC_RC_GW_H
#define UAN_MAC_RC_GW_H

#include "uan-mac.h"

#include "ns3/mac8-address.h"
#include "ns3/nstime.h"
#include "ns3/traced-callback.h"

#include <map>
#include <set>

namespace ns3 {

class UanTxMode;

class UanMacRcGw : public UanMac {
public:
  UanMacRcGw();
  ~UanMacRcGw() override;

  static TypeId GetTypeId();

  bool Enqueue(Ptr<Packet> pkt, uint16_t protocolNumber,
               const Address &dest) override;
  void SetForwardUpCb(
      Callback<void, Ptr<Packet>, uint16_t, const Mac8Address &> cb) override;
  void AttachPhy(Ptr<UanPhy> phy) override;
  void Clear() override;
  int64_t AssignStreams(int64_t stream) override;

  typedef void (*CycleCallback)(Time now, Time delay, uint32_t numRts,
                                uint32_t totalBytes, double secs,
                                uint32_t ctlRate, double actualX);

private:
  enum State { IDLE, INCYCLE, CTSING };

  State m_state;

  struct Request {
    uint8_t numFrames;
    uint8_t frameNo;
    uint8_t retryNo;
    uint16_t length;
    Time rxTime;
  };

  struct AckData {
    uint8_t frameNo;
    std::set<uint8_t> rxFrames;
    uint8_t expFrames;
  };

  Callback<void, Ptr<Packet>, uint16_t, const Mac8Address &> m_forwardUpCb;

  Ptr<UanPhy> m_phy;
  Time m_maxDelta;
  Time m_sifs;
  uint32_t m_maxRes;
  uint32_t m_numRates;
  uint32_t m_rtsSize;
  uint32_t m_ctsSizeN;
  uint32_t m_ctsSizeG;
  uint32_t m_ackSize;
  uint16_t m_currentRetryRate;
  uint32_t m_currentRateNum;
  uint32_t m_numNodes;
  uint32_t m_totalRate;
  uint32_t m_rateStep;
  uint32_t m_frameSize;

  double m_minRetryRate;
  double m_retryStep;

  std::map<Mac8Address, Time> m_propDelay;

  std::map<Mac8Address, AckData> m_ackData;

  std::map<Mac8Address, Request> m_requests;
  std::set<std::pair<Time, Mac8Address>> m_sortedRes;

  bool m_cleared;

  TracedCallback<Ptr<const Packet>, UanTxMode> m_rxLogger;

  TracedCallback<Time, Time, uint32_t, uint32_t, double, uint32_t, double>
      m_cycleLogger;

  void ReceivePacket(Ptr<Packet> pkt, double sinr, UanTxMode mode);

  void StartCycle();
  void EndCycle();
  void SendPacket(Ptr<Packet> pkt, uint32_t rate);
  void CycleStarted();
  void ReceiveError(Ptr<Packet> pkt, double sinr);

  double ComputeAlpha(uint32_t totalFrames, uint32_t totalBytes, uint32_t n,
                      uint32_t a, double deltaK);
  std::vector<double> GetExpPdk();
  double ComputeExpS(uint32_t a, uint32_t ld, std::vector<double> exppdk);
  double ComputeExpS(uint32_t a, uint32_t ld);
  uint32_t CompExpMinIndex(uint32_t n, uint32_t k);
  double ComputePiK(uint32_t a, uint32_t n, uint32_t k);
  double ComputeExpBOverA(uint32_t n, uint32_t a, uint32_t ldlh,
                          std::vector<double> deltaK);
  uint64_t NchooseK(uint32_t n, uint32_t k);
  uint32_t FindOptA();

protected:
  void DoDispose() override;
};

} // namespace ns3

#endif
