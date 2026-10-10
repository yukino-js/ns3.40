
#ifndef SS_MANAGER_H
#define SS_MANAGER_H

#include "cid.h"
#include "ss-record.h"

#include <stdint.h>

namespace ns3 {

class SSManager : public Object {
public:
  static TypeId GetTypeId();
  SSManager();
  ~SSManager() override;
  SSRecord *CreateSSRecord(const Mac48Address &macAddress);
  SSRecord *GetSSRecord(const Mac48Address &macAddress) const;
  SSRecord *GetSSRecord(Cid cid) const;
  std::vector<SSRecord *> *GetSSRecords() const;
  bool IsInRecord(const Mac48Address &macAddress) const;
  bool IsRegistered(const Mac48Address &macAddress) const;
  void DeleteSSRecord(Cid cid);
  Mac48Address GetMacAddress(Cid cid) const;
  uint32_t GetNSSs() const;
  uint32_t GetNRegisteredSSs() const;

private:
  std::vector<SSRecord *> *m_ssRecords;
};

} // namespace ns3

#endif
