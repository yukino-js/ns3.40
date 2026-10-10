
#ifndef FF_MAC_SCHEDULER_H
#define FF_MAC_SCHEDULER_H

#include "ff-mac-common.h"

#include <ns3/object.h>

namespace ns3 {

class FfMacCschedSapUser;
class FfMacSchedSapUser;
class FfMacCschedSapProvider;
class FfMacSchedSapProvider;
class LteFfrSapProvider;
class LteFfrSapUser;

using DlHarqProcessesStatus_t = std::vector<uint8_t>;

using DlHarqProcessesTimer_t = std::vector<uint8_t>;

using DlHarqProcessesDciBuffer_t = std::vector<DlDciListElement_s>;

using RlcPduList_t = std::vector<std::vector<RlcPduListElement_s>>;

using DlHarqRlcPduListBuffer_t = std::vector<RlcPduList_t>;

using UlHarqProcessesDciBuffer_t = std::vector<UlDciListElement_s>;

using UlHarqProcessesStatus_t = std::vector<uint8_t>;

constexpr double NO_SINR = -5000;

constexpr uint32_t HARQ_PROC_NUM = 8;

constexpr uint32_t HARQ_DL_TIMEOUT = 11;

class FfMacScheduler : public Object {
public:
  enum UlCqiFilter_t { SRS_UL_CQI, PUSCH_UL_CQI };

  FfMacScheduler();
  ~FfMacScheduler() override;

  void DoDispose() override;
  static TypeId GetTypeId();

  virtual void SetFfMacCschedSapUser(FfMacCschedSapUser *s) = 0;

  virtual void SetFfMacSchedSapUser(FfMacSchedSapUser *s) = 0;

  virtual FfMacCschedSapProvider *GetFfMacCschedSapProvider() = 0;

  virtual FfMacSchedSapProvider *GetFfMacSchedSapProvider() = 0;

  virtual void SetLteFfrSapProvider(LteFfrSapProvider *s) = 0;

  virtual LteFfrSapUser *GetLteFfrSapUser() = 0;

protected:
  UlCqiFilter_t m_ulCqiFilter;
};

} // namespace ns3

#endif
