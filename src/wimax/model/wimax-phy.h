
#ifndef WIMAX_PHY_H
#define WIMAX_PHY_H

#include "bvec.h"
#include "send-params.h"

#include "ns3/callback.h"
#include "ns3/event-id.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/traced-callback.h"

#include <list>
#include <stdint.h>

namespace ns3 {

class WimaxChannel;
class WimaxNetDevice;
class NetDevice;
class Packet;

class WimaxPhy : public Object {
public:
  enum ModulationType {
    MODULATION_TYPE_BPSK_12,
    MODULATION_TYPE_QPSK_12,
    MODULATION_TYPE_QPSK_34,
    MODULATION_TYPE_QAM16_12,
    MODULATION_TYPE_QAM16_34,
    MODULATION_TYPE_QAM64_23,
    MODULATION_TYPE_QAM64_34
  };

  enum PhyState {
    PHY_STATE_IDLE,
    PHY_STATE_SCANNING,
    PHY_STATE_TX,
    PHY_STATE_RX
  };

  enum PhyType { SimpleWimaxPhy, simpleOfdmWimaxPhy };

  static TypeId GetTypeId();
  WimaxPhy();
  ~WimaxPhy() override;
  void Attach(Ptr<WimaxChannel> channel);
  Ptr<WimaxChannel> GetChannel() const;
  void SetDevice(Ptr<WimaxNetDevice> device);
  Ptr<NetDevice> GetDevice() const;
  void SetReceiveCallback(Callback<void, Ptr<const PacketBurst>> callback);
  Callback<void, Ptr<const PacketBurst>> GetReceiveCallback() const;
  virtual void Send(SendParams *params) = 0;
  virtual PhyType GetPhyType() const = 0;
  void SetDuplex(uint64_t rxFrequency, uint64_t txFrequency);
  void SetSimplex(uint64_t frequency);
  uint64_t GetRxFrequency() const;
  uint64_t GetTxFrequency() const;
  uint64_t GetScanningFrequency() const;
  void SetNrCarriers(uint8_t nrCarriers);
  uint8_t GetNrCarriers() const;
  void SetFrameDuration(Time frameDuration);
  Time GetFrameDurationSec() const;
  Time GetFrameDuration() const;
  void SetFrequency(uint32_t frequency);
  uint32_t GetFrequency() const;
  void SetChannelBandwidth(uint32_t channelBandwidth);
  uint32_t GetChannelBandwidth() const;
  uint16_t GetNfft() const;
  double GetSamplingFactor() const;
  double GetSamplingFrequency() const;
  void SetPsDuration(Time psDuration);
  Time GetPsDuration() const;
  void SetSymbolDuration(Time symbolDuration);
  Time GetSymbolDuration() const;
  double GetGValue() const;
  void SetPsPerSymbol(uint16_t psPerSymbol);
  uint16_t GetPsPerSymbol() const;

  void SetPsPerFrame(uint16_t psPerFrame);
  uint16_t GetPsPerFrame() const;
  void SetSymbolsPerFrame(uint32_t symbolsPerFrame);
  uint32_t GetSymbolsPerFrame() const;
  bool IsDuplex() const;
  void SetState(PhyState state);
  PhyState GetState() const;
  void StartScanning(uint64_t frequency, Time timeout,
                     Callback<void, bool, uint64_t> callback);

  void SetScanningCallback() const;
  EventId GetChnlSrchTimeoutEvent() const;
  void SetDataRates();
  uint32_t GetDataRate(ModulationType modulationType) const;
  Time GetTransmissionTime(uint32_t size, ModulationType modulationType) const;
  uint64_t GetNrSymbols(uint32_t size, ModulationType modulationType) const;
  uint64_t GetNrBytes(uint32_t symbols, ModulationType modulationType) const;
  uint16_t GetTtg() const;
  uint16_t GetRtg() const;
  uint8_t GetFrameDurationCode() const;
  Time GetFrameDuration(uint8_t frameDurationCode) const;
  void SetPhyParameters();
  void DoDispose() override;

  virtual int64_t AssignStreams(int64_t stream) = 0;

private:
  void GetModulationFecParams(ModulationType modulationType,
                              uint8_t &bitsPerSymbol, double &fecCode) const;
  void EndScanning();
  virtual Time DoGetTransmissionTime(uint32_t size,
                                     ModulationType modulationType) const = 0;
  virtual void DoAttach(Ptr<WimaxChannel> channel) = 0;
  virtual void DoSetDataRates() = 0;
  virtual uint32_t DoGetDataRate(ModulationType modulationType) const = 0;
  virtual uint64_t DoGetNrSymbols(uint32_t size,
                                  ModulationType modulationType) const = 0;
  virtual uint64_t DoGetNrBytes(uint32_t symbols,
                                ModulationType modulationType) const = 0;
  virtual uint16_t DoGetTtg() const = 0;
  virtual uint16_t DoGetRtg() const = 0;

  virtual uint8_t DoGetFrameDurationCode() const = 0;
  virtual Time DoGetFrameDuration(uint8_t frameDurationCode) const = 0;
  virtual void DoSetPhyParameters() = 0;
  virtual double DoGetSamplingFactor() const = 0;
  virtual uint16_t DoGetNfft() const = 0;
  virtual double DoGetSamplingFrequency() const = 0;
  virtual double DoGetGValue() const = 0;

  Ptr<WimaxNetDevice> m_device;
  Ptr<WimaxChannel> m_channel;

  uint64_t m_txFrequency;
  uint64_t m_rxFrequency;
  uint64_t m_scanningFrequency;
  EventId m_dlChnlSrchTimeoutEvent;
  bool m_duplex;
  PhyState m_state;

  Callback<void, Ptr<const PacketBurst>> m_rxCallback;
  Callback<void, bool, uint64_t> m_scanningCallback;

  uint8_t m_nrCarriers;
  Time m_frameDuration;
  uint32_t m_frequency;
  uint32_t m_channelBandwidth;
  Time m_psDuration;
  Time m_symbolDuration;
  uint16_t m_psPerSymbol;
  uint16_t m_psPerFrame;
  uint32_t m_symbolsPerFrame;
};

} // namespace ns3

#endif
