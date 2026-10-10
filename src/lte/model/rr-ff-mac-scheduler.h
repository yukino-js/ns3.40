
#ifndef RR_FF_MAC_SCHEDULER_H
#define RR_FF_MAC_SCHEDULER_H

#include "ff-mac-csched-sap.h"
#include "ff-mac-sched-sap.h"
#include "ff-mac-scheduler.h"
#include "lte-amc.h"
#include "lte-ffr-sap.h"

#include <map>
#include <vector>

namespace ns3 {

class RrFfMacScheduler : public FfMacScheduler {
public:
  RrFfMacScheduler();

  ~RrFfMacScheduler() override;

  void DoDispose() override;
  static TypeId GetTypeId();

  void SetFfMacCschedSapUser(FfMacCschedSapUser *s) override;
  void SetFfMacSchedSapUser(FfMacSchedSapUser *s) override;
  FfMacCschedSapProvider *GetFfMacCschedSapProvider() override;
  FfMacSchedSapProvider *GetFfMacSchedSapProvider() override;

  void SetLteFfrSapProvider(LteFfrSapProvider *s) override;
  LteFfrSapUser *GetLteFfrSapUser() override;

  friend class MemberCschedSapProvider<RrFfMacScheduler>;
  friend class MemberSchedSapProvider<RrFfMacScheduler>;

  void TransmissionModeConfigurationUpdate(uint16_t rnti, uint8_t txMode);

private:
  void DoCschedCellConfigReq(
      const FfMacCschedSapProvider::CschedCellConfigReqParameters &params);

  void DoCschedUeConfigReq(
      const FfMacCschedSapProvider::CschedUeConfigReqParameters &params);

  void DoCschedLcConfigReq(
      const FfMacCschedSapProvider::CschedLcConfigReqParameters &params);

  void DoCschedLcReleaseReq(
      const FfMacCschedSapProvider::CschedLcReleaseReqParameters &params);

  void DoCschedUeReleaseReq(
      const FfMacCschedSapProvider::CschedUeReleaseReqParameters &params);

  void DoSchedDlRlcBufferReq(
      const FfMacSchedSapProvider::SchedDlRlcBufferReqParameters &params);

  void DoSchedDlPagingBufferReq(
      const FfMacSchedSapProvider::SchedDlPagingBufferReqParameters &params);

  void DoSchedDlMacBufferReq(
      const FfMacSchedSapProvider::SchedDlMacBufferReqParameters &params);

  void DoSchedDlTriggerReq(
      const FfMacSchedSapProvider::SchedDlTriggerReqParameters &params);

  void DoSchedDlRachInfoReq(
      const FfMacSchedSapProvider::SchedDlRachInfoReqParameters &params);

  void DoSchedDlCqiInfoReq(
      const FfMacSchedSapProvider::SchedDlCqiInfoReqParameters &params);

  void DoSchedUlTriggerReq(
      const FfMacSchedSapProvider::SchedUlTriggerReqParameters &params);

  void DoSchedUlNoiseInterferenceReq(
      const FfMacSchedSapProvider::SchedUlNoiseInterferenceReqParameters
          &params);

  void DoSchedUlSrInfoReq(
      const FfMacSchedSapProvider::SchedUlSrInfoReqParameters &params);

  void DoSchedUlMacCtrlInfoReq(
      const FfMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters &params);

  void DoSchedUlCqiInfoReq(
      const FfMacSchedSapProvider::SchedUlCqiInfoReqParameters &params);

  int GetRbgSize(int dlbandwidth);

  static bool
  SortRlcBufferReq(FfMacSchedSapProvider::SchedDlRlcBufferReqParameters i,
                   FfMacSchedSapProvider::SchedDlRlcBufferReqParameters j);

  void RefreshDlCqiMaps();
  void RefreshUlCqiMaps();

  void UpdateDlRlcBufferInfo(uint16_t rnti, uint8_t lcid, uint16_t size);
  void UpdateUlRlcBufferInfo(uint16_t rnti, uint16_t size);

  uint8_t UpdateHarqProcessId(uint16_t rnti);

  bool HarqProcessAvailability(uint16_t rnti);

  void RefreshHarqProcesses();

  Ptr<LteAmc> m_amc;

  std::list<FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>
      m_rlcBufferReq;

  std::map<uint16_t, uint8_t> m_p10CqiRxed;
  std::map<uint16_t, uint32_t> m_p10CqiTimers;

  std::map<uint16_t, std::vector<uint16_t>> m_allocationMaps;

  std::map<uint16_t, std::vector<double>> m_ueCqi;
  std::map<uint16_t, uint32_t> m_ueCqiTimers;

  std::map<uint16_t, uint32_t> m_ceBsrRxed;

  FfMacCschedSapUser *m_cschedSapUser;
  FfMacSchedSapUser *m_schedSapUser;
  FfMacCschedSapProvider *m_cschedSapProvider;
  FfMacSchedSapProvider *m_schedSapProvider;

  LteFfrSapUser *m_ffrSapUser;
  LteFfrSapProvider *m_ffrSapProvider;

  FfMacCschedSapProvider::CschedCellConfigReqParameters m_cschedCellConfig;

  uint16_t m_nextRntiDl;
  uint16_t m_nextRntiUl;

  uint32_t m_cqiTimersThreshold;

  std::map<uint16_t, uint8_t> m_uesTxMode;

  bool m_harqOn;
  std::map<uint16_t, uint8_t> m_dlHarqCurrentProcessId;
  std::map<uint16_t, DlHarqProcessesStatus_t> m_dlHarqProcessesStatus;
  std::map<uint16_t, DlHarqProcessesTimer_t> m_dlHarqProcessesTimer;
  std::map<uint16_t, DlHarqProcessesDciBuffer_t> m_dlHarqProcessesDciBuffer;
  std::map<uint16_t, DlHarqRlcPduListBuffer_t>
      m_dlHarqProcessesRlcPduListBuffer;
  std::vector<DlInfoListElement_s> m_dlInfoListBuffered;

  std::map<uint16_t, uint8_t> m_ulHarqCurrentProcessId;
  std::map<uint16_t, UlHarqProcessesStatus_t> m_ulHarqProcessesStatus;
  std::map<uint16_t, UlHarqProcessesDciBuffer_t> m_ulHarqProcessesDciBuffer;

  std::vector<RachListElement_s> m_rachList;
  std::vector<uint16_t> m_rachAllocationMap;
  uint8_t m_ulGrantMcs;
};

} // namespace ns3

#endif
