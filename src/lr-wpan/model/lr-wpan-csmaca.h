
#ifndef LR_WPAN_CSMACA_H
#define LR_WPAN_CSMACA_H

#include "lr-wpan-mac.h"

#include <ns3/event-id.h>
#include <ns3/object.h>

namespace ns3 {

class UniformRandomVariable;

typedef Callback<void, LrWpanMacState> LrWpanMacStateCallback;
typedef Callback<void, uint32_t> LrWpanMacTransCostCallback;

class LrWpanCsmaCa : public Object {
public:
  static TypeId GetTypeId();
  LrWpanCsmaCa();
  ~LrWpanCsmaCa() override;

  LrWpanCsmaCa(const LrWpanCsmaCa &) = delete;
  LrWpanCsmaCa &operator=(const LrWpanCsmaCa &) = delete;

  void SetMac(Ptr<LrWpanMac> mac);
  Ptr<LrWpanMac> GetMac() const;

  void SetSlottedCsmaCa();
  void SetUnSlottedCsmaCa();
  bool IsSlottedCsmaCa() const;
  bool IsUnSlottedCsmaCa() const;
  void SetMacMinBE(uint8_t macMinBE);
  uint8_t GetMacMinBE() const;
  void SetMacMaxBE(uint8_t macMaxBE);
  uint8_t GetMacMaxBE() const;
  void SetMacMaxCSMABackoffs(uint8_t macMaxCSMABackoffs);

  uint8_t GetMacMaxCSMABackoffs() const;
  Time GetTimeToNextSlot() const;
  void Start();
  void Cancel();
  void RandomBackoffDelay();
  void CanProceed();
  void RequestCCA();
  void DeferCsmaTimeout();
  void PlmeCcaConfirm(LrWpanPhyEnumeration status);
  void SetLrWpanMacTransCostCallback(LrWpanMacTransCostCallback trans);
  void SetLrWpanMacStateCallback(LrWpanMacStateCallback macState);
  void SetBatteryLifeExtension(bool batteryLifeExtension);
  int64_t AssignStreams(int64_t stream);
  uint8_t GetNB() const;
  bool GetBatteryLifeExtension() const;

private:
  void DoDispose() override;
  Time GetTimeLeftInCap();
  LrWpanMacTransCostCallback m_lrWpanMacTransCostCallback;
  LrWpanMacStateCallback m_lrWpanMacStateCallback;
  bool m_isSlotted;
  Ptr<LrWpanMac> m_mac;
  uint8_t m_NB;
  uint8_t m_CW;
  uint8_t m_BE;
  bool m_macBattLifeExt;
  uint8_t m_macMinBE;
  uint8_t m_macMaxBE;
  uint8_t m_macMaxCSMABackoffs;
  uint64_t m_randomBackoffPeriodsLeft;
  Ptr<UniformRandomVariable> m_random;
  EventId m_randomBackoffEvent;
  EventId m_endCapEvent;
  EventId m_requestCcaEvent;
  EventId m_canProceedEvent;
  bool m_ccaRequestRunning;
  bool m_coorDest;
};

} // namespace ns3

#endif
