
#ifndef CSMA_CHANNEL_H
#define CSMA_CHANNEL_H

#include "ns3/channel.h"
#include "ns3/data-rate.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"

namespace ns3 {

class Packet;

class CsmaNetDevice;

class CsmaDeviceRec {
public:
  Ptr<CsmaNetDevice> devicePtr;
  bool active;

  CsmaDeviceRec();

  CsmaDeviceRec(Ptr<CsmaNetDevice> device);

  CsmaDeviceRec(const CsmaDeviceRec &o);

  bool IsActive() const;
};

enum WireState { IDLE, TRANSMITTING, PROPAGATING };

class CsmaChannel : public Channel {
public:
  static TypeId GetTypeId();

  CsmaChannel();
  ~CsmaChannel() override;

  CsmaChannel(const CsmaChannel &) = delete;
  CsmaChannel &operator=(const CsmaChannel &) = delete;

  int32_t Attach(Ptr<CsmaNetDevice> device);

  bool Detach(Ptr<CsmaNetDevice> device);

  bool Detach(uint32_t deviceId);

  bool Reattach(uint32_t deviceId);

  bool Reattach(Ptr<CsmaNetDevice> device);

  bool TransmitStart(Ptr<const Packet> p, uint32_t srcId);

  bool TransmitEnd();

  void PropagationCompleteEvent();

  int32_t GetDeviceNum(Ptr<CsmaNetDevice> device);

  WireState GetState();

  bool IsBusy();

  bool IsActive(uint32_t deviceId);

  uint32_t GetNumActDevices();

  std::size_t GetNDevices() const override;

  Ptr<NetDevice> GetDevice(std::size_t i) const override;

  Ptr<CsmaNetDevice> GetCsmaDevice(std::size_t i) const;

  DataRate GetDataRate();

  Time GetDelay();

private:
  DataRate m_bps;

  Time m_delay;

  std::vector<CsmaDeviceRec> m_deviceList;

  Ptr<Packet> m_currentPkt;

  uint32_t m_currentSrc;

  WireState m_state;
};

} // namespace ns3

#endif
