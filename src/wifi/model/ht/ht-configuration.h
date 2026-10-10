
#ifndef HT_CONFIGURATION_H
#define HT_CONFIGURATION_H

#include "ns3/object.h"

namespace ns3 {

class HtConfiguration : public Object {
public:
  HtConfiguration();
  ~HtConfiguration() override;

  static TypeId GetTypeId();

  void SetShortGuardIntervalSupported(bool enable);
  bool GetShortGuardIntervalSupported() const;
  void SetLdpcSupported(bool enable);
  bool GetLdpcSupported() const;
  void Set40MHzOperationSupported(bool enable);
  bool Get40MHzOperationSupported() const;

private:
  bool m_sgiSupported;
  bool m_ldpcSupported;
  bool m_40MHzSupported;
};

} // namespace ns3

#endif
