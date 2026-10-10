
#ifndef SIMPLE_OFDM_WIMAX_PHY_H
#define SIMPLE_OFDM_WIMAX_PHY_H

#include "bvec.h"
#include "snr-to-block-error-rate-manager.h"
#include "wimax-connection.h"
#include "wimax-phy.h"

#include "ns3/callback.h"
#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"

#include <list>
#include <stdint.h>

namespace ns3 {

class WimaxChannel;
class WimaxNetDevice;
class NetDevice;
class Packet;
class SimpleOfdmWimaxChannel;

class SimpleOfdmWimaxPhy : public WimaxPhy {
public:
  enum FrameDurationCode {
    FRAME_DURATION_2_POINT_5_MS,
    FRAME_DURATION_4_MS,
    FRAME_DURATION_5_MS,
    FRAME_DURATION_8_MS,
    FRAME_DURATION_10_MS,
    FRAME_DURATION_12_POINT_5_MS,
    FRAME_DURATION_20_MS
  };

  static TypeId GetTypeId();
  SimpleOfdmWimaxPhy();
  SimpleOfdmWimaxPhy(char *tracesPath);
  ~SimpleOfdmWimaxPhy() override;
  void ActivateLoss(bool loss);
  void SetSNRToBlockErrorRateTracesPath(char *tracesPath);
  void DoAttach(Ptr<WimaxChannel> channel) override;
  void SetReceiveCallback(
      Callback<void, Ptr<PacketBurst>, Ptr<WimaxConnection>> callback);
  void Send(Ptr<PacketBurst> burst, WimaxPhy::ModulationType modulationType,
            uint8_t direction);
  void Send(SendParams *params) override;
  WimaxPhy::PhyType GetPhyType() const override;

  void StartReceive(uint32_t burstSize, bool isFirstBlock, uint64_t frequency,
                    WimaxPhy::ModulationType modulationType, uint8_t direction,
                    double rxPower, Ptr<PacketBurst> burst);

  uint32_t GetBandwidth() const;
  void SetBandwidth(uint32_t BW);
  double GetTxPower() const;
  void SetTxPower(double txPower);
  double GetNoiseFigure() const;
  void SetNoiseFigure(double nf);

  void NotifyTxBegin(Ptr<PacketBurst> burst);

  void NotifyTxEnd(Ptr<PacketBurst> burst);

  void NotifyTxDrop(Ptr<PacketBurst> burst);

  void NotifyRxBegin(Ptr<PacketBurst> burst);

  void NotifyRxEnd(Ptr<PacketBurst> burst);

  void NotifyRxDrop(Ptr<PacketBurst> burst);

  int64_t AssignStreams(int64_t stream) override;

private:
  Time
  DoGetTransmissionTime(uint32_t size,
                        WimaxPhy::ModulationType modulationType) const override;
  uint64_t
  DoGetNrSymbols(uint32_t size,
                 WimaxPhy::ModulationType modulationType) const override;
  uint64_t DoGetNrBytes(uint32_t symbols,
                        WimaxPhy::ModulationType modulationType) const override;
  Bvec ConvertBurstToBits(Ptr<const PacketBurst> burst);
  Ptr<PacketBurst> ConvertBitsToBurst(Bvec buffer);
  void CreateFecBlocks(const Bvec &buffer,
                       WimaxPhy::ModulationType modulationType);
  Bvec RecreateBuffer();
  uint32_t GetFecBlockSize(WimaxPhy::ModulationType type) const;
  uint32_t GetCodedFecBlockSize(WimaxPhy::ModulationType modulationType) const;
  void SetBlockParameters(uint32_t burstSize,
                          WimaxPhy::ModulationType modulationType);
  uint16_t GetNrBlocks(uint32_t burstSize,
                       WimaxPhy::ModulationType modulationType) const;
  void DoDispose() override;
  void EndSend();
  void EndSendFecBlock(WimaxPhy::ModulationType modulationType,
                       uint8_t direction);
  void EndReceive(Ptr<const PacketBurst> burst);
  void EndReceiveFecBlock(uint32_t burstSize,
                          WimaxPhy::ModulationType modulationType,
                          uint8_t direction, bool drop, Ptr<PacketBurst> burst);
  void StartSendDummyFecBlock(bool isFirstBlock,
                              WimaxPhy::ModulationType modulationType,
                              uint8_t direction);
  Time GetBlockTransmissionTime(WimaxPhy::ModulationType modulationType) const;
  void DoSetDataRates() override;
  void InitSimpleOfdmWimaxPhy();

  void GetModulationFecParams(WimaxPhy::ModulationType modulationType,
                              uint8_t &bitsPerSymbol, double &fecCode) const;
  uint32_t CalculateDataRate(WimaxPhy::ModulationType modulationType) const;
  uint32_t
  DoGetDataRate(WimaxPhy::ModulationType modulationType) const override;
  uint16_t DoGetTtg() const override;
  uint16_t DoGetRtg() const override;
  uint8_t DoGetFrameDurationCode() const override;
  Time DoGetFrameDuration(uint8_t frameDurationCode) const override;
  void DoSetPhyParameters() override;
  uint16_t DoGetNfft() const override;
  void DoSetNfft(uint16_t nfft);
  double DoGetSamplingFactor() const override;
  double DoGetSamplingFrequency() const override;
  double DoGetGValue() const override;
  void DoSetGValue(double g);

  double GetRxGain() const;
  void SetRxGain(double rxgain);

  double GetTxGain() const;
  void SetTxGain(double txgain);

  std::string GetTraceFilePath() const;
  void SetTraceFilePath(std::string path);

  uint16_t m_fecBlockSize;
  uint32_t m_currentBurstSize;

  std::list<Bvec> *m_receivedFecBlocks;
  uint32_t m_nrFecBlocksSent;
  std::list<Bvec> *m_fecBlocks;
  Time m_blockTime;

  TracedCallback<Ptr<const PacketBurst>> m_traceRx;
  TracedCallback<Ptr<const PacketBurst>> m_traceTx;

  uint32_t m_dataRateBpsk12;
  uint32_t m_dataRateQpsk12;
  uint32_t m_dataRateQpsk34;
  uint32_t m_dataRateQam16_12;
  uint32_t m_dataRateQam16_34;
  uint32_t m_dataRateQam64_23;
  uint32_t m_dataRateQam64_34;

  uint16_t m_nrBlocks;
  uint16_t m_nrRemainingBlocksToSend;
  Ptr<PacketBurst> m_currentBurst;
  uint16_t m_blockSize;
  uint32_t m_paddingBits;
  uint16_t m_nbErroneousBlock;
  uint16_t m_nrReceivedFecBlocks;
  uint16_t m_nfft;
  double m_g;
  double m_bandWidth;
  double m_txPower;
  double m_noiseFigure;
  double m_txGain;
  double m_rxGain;
  TracedCallback<Ptr<PacketBurst>> m_phyTxBeginTrace;

  TracedCallback<Ptr<PacketBurst>> m_phyTxEndTrace;

  TracedCallback<Ptr<PacketBurst>> m_phyTxDropTrace;

  TracedCallback<Ptr<PacketBurst>> m_phyRxBeginTrace;

  TracedCallback<Ptr<PacketBurst>> m_phyRxEndTrace;

  TracedCallback<Ptr<PacketBurst>> m_phyRxDropTrace;

  SNRToBlockErrorRateManager *m_snrToBlockErrorRateManager;

  Ptr<UniformRandomVariable> m_URNG;
};

} // namespace ns3

#endif
