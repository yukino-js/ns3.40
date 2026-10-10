
#ifndef WIFI_PROTECTION_H
#define WIFI_PROTECTION_H

#include "ctrl-headers.h"
#include "wifi-tx-vector.h"

#include "ns3/nstime.h"

#include <memory>

namespace ns3 {

struct WifiProtection {
  enum Method { NONE = 0, RTS_CTS, CTS_TO_SELF, MU_RTS_CTS };

  WifiProtection(Method m);
  virtual ~WifiProtection();

  virtual std::unique_ptr<WifiProtection> Copy() const = 0;

  virtual void Print(std::ostream &os) const = 0;

  const Method method;
  Time protectionTime;
};

struct WifiNoProtection : public WifiProtection {
  WifiNoProtection();

  std::unique_ptr<WifiProtection> Copy() const override;
  void Print(std::ostream &os) const override;
};

struct WifiRtsCtsProtection : public WifiProtection {
  WifiRtsCtsProtection();

  std::unique_ptr<WifiProtection> Copy() const override;
  void Print(std::ostream &os) const override;

  WifiTxVector rtsTxVector;
  WifiTxVector ctsTxVector;
};

struct WifiCtsToSelfProtection : public WifiProtection {
  WifiCtsToSelfProtection();

  std::unique_ptr<WifiProtection> Copy() const override;
  void Print(std::ostream &os) const override;

  WifiTxVector ctsTxVector;
};

struct WifiMuRtsCtsProtection : public WifiProtection {
  WifiMuRtsCtsProtection();

  std::unique_ptr<WifiProtection> Copy() const override;
  void Print(std::ostream &os) const override;

  CtrlTriggerHeader muRts;
  WifiTxVector muRtsTxVector;
};

std::ostream &operator<<(std::ostream &os, const WifiProtection *protection);

} // namespace ns3

#endif
