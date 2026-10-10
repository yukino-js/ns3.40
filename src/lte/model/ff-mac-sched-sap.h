
#ifndef FF_MAC_SCHED_SAP_H
#define FF_MAC_SCHED_SAP_H

#include "ff-mac-common.h"

#include <stdint.h>
#include <vector>

namespace ns3 {

class FfMacSchedSapProvider {
public:
  virtual ~FfMacSchedSapProvider();

  struct SchedDlRlcBufferReqParameters {
    uint16_t m_rnti;
    uint8_t m_logicalChannelIdentity;
    uint32_t m_rlcTransmissionQueueSize;
    uint16_t m_rlcTransmissionQueueHolDelay;
    uint32_t m_rlcRetransmissionQueueSize;
    uint16_t m_rlcRetransmissionHolDelay;
    uint16_t m_rlcStatusPduSize;

    std::vector<VendorSpecificListElement_s> m_vendorSpecificList;
  };

  struct SchedDlPagingBufferReqParameters {
    uint16_t m_rnti;
    std::vector<PagingInfoListElement_s> m_pagingInfoList;

    std::vector<VendorSpecificListElement_s> m_vendorSpecificList;
  };

  struct SchedDlMacBufferReqParameters {
    uint16_t m_rnti;
    CeBitmap_e m_ceBitmap;

    std::vector<VendorSpecificListElement_s> m_vendorSpecificList;
  };

  struct SchedDlTriggerReqParameters {
    uint16_t m_sfnSf;
    std::vector<DlInfoListElement_s> m_dlInfoList;

    std::vector<VendorSpecificListElement_s> m_vendorSpecificList;
  };

  struct SchedDlRachInfoReqParameters {
    uint16_t m_sfnSf;
    std::vector<RachListElement_s> m_rachList;

    std::vector<VendorSpecificListElement_s> m_vendorSpecificList;
  };

  struct SchedDlCqiInfoReqParameters {
    uint16_t m_sfnSf;
    std::vector<CqiListElement_s> m_cqiList;

    std::vector<VendorSpecificListElement_s> m_vendorSpecificList;
  };

  struct SchedUlTriggerReqParameters {
    uint16_t m_sfnSf;
    std::vector<UlInfoListElement_s> m_ulInfoList;

    std::vector<VendorSpecificListElement_s> m_vendorSpecificList;
  };

  struct SchedUlNoiseInterferenceReqParameters {
    uint16_t m_sfnSf;
    uint16_t m_rip;
    uint16_t m_tnp;

    std::vector<VendorSpecificListElement_s> m_vendorSpecificList;
  };

  struct SchedUlSrInfoReqParameters {
    uint16_t m_sfnSf;
    std::vector<SrListElement_s> m_srList;

    std::vector<VendorSpecificListElement_s> m_vendorSpecificList;
  };

  struct SchedUlMacCtrlInfoReqParameters {
    uint16_t m_sfnSf;
    std::vector<MacCeListElement_s> m_macCeList;

    std::vector<VendorSpecificListElement_s> m_vendorSpecificList;
  };

  struct SchedUlCqiInfoReqParameters {
    uint16_t m_sfnSf;
    UlCqi_s m_ulCqi;

    std::vector<VendorSpecificListElement_s> m_vendorSpecificList;
  };

  virtual void
  SchedDlRlcBufferReq(const SchedDlRlcBufferReqParameters &params) = 0;

  virtual void
  SchedDlPagingBufferReq(const SchedDlPagingBufferReqParameters &params) = 0;

  virtual void
  SchedDlMacBufferReq(const SchedDlMacBufferReqParameters &params) = 0;

  virtual void SchedDlTriggerReq(const SchedDlTriggerReqParameters &params) = 0;

  virtual void
  SchedDlRachInfoReq(const SchedDlRachInfoReqParameters &params) = 0;

  virtual void SchedDlCqiInfoReq(const SchedDlCqiInfoReqParameters &params) = 0;

  virtual void SchedUlTriggerReq(const SchedUlTriggerReqParameters &params) = 0;

  virtual void SchedUlNoiseInterferenceReq(
      const SchedUlNoiseInterferenceReqParameters &params) = 0;

  virtual void SchedUlSrInfoReq(const SchedUlSrInfoReqParameters &params) = 0;

  virtual void
  SchedUlMacCtrlInfoReq(const SchedUlMacCtrlInfoReqParameters &params) = 0;

  virtual void SchedUlCqiInfoReq(const SchedUlCqiInfoReqParameters &params) = 0;

private:
};

class FfMacSchedSapUser {
public:
  virtual ~FfMacSchedSapUser();

  struct SchedDlConfigIndParameters {
    std::vector<BuildDataListElement_s> m_buildDataList;
    std::vector<BuildRarListElement_s> m_buildRarList;
    std::vector<BuildBroadcastListElement_s> m_buildBroadcastList;

    uint8_t m_nrOfPdcchOfdmSymbols;

    std::vector<VendorSpecificListElement_s> m_vendorSpecificList;
  };

  struct SchedUlConfigIndParameters {
    std::vector<UlDciListElement_s> m_dciList;
    std::vector<PhichListElement_s> m_phichList;

    std::vector<VendorSpecificListElement_s> m_vendorSpecificList;
  };

  virtual void SchedDlConfigInd(const SchedDlConfigIndParameters &params) = 0;

  virtual void SchedUlConfigInd(const SchedUlConfigIndParameters &params) = 0;

private:
};

template <class C> class MemberSchedSapProvider : public FfMacSchedSapProvider {
public:
  MemberSchedSapProvider(C *scheduler);

  MemberSchedSapProvider() = delete;

  void
  SchedDlRlcBufferReq(const SchedDlRlcBufferReqParameters &params) override;
  void SchedDlPagingBufferReq(
      const SchedDlPagingBufferReqParameters &params) override;
  void
  SchedDlMacBufferReq(const SchedDlMacBufferReqParameters &params) override;
  void SchedDlTriggerReq(const SchedDlTriggerReqParameters &params) override;
  void SchedDlRachInfoReq(const SchedDlRachInfoReqParameters &params) override;
  void SchedDlCqiInfoReq(const SchedDlCqiInfoReqParameters &params) override;
  void SchedUlTriggerReq(const SchedUlTriggerReqParameters &params) override;
  void SchedUlNoiseInterferenceReq(
      const SchedUlNoiseInterferenceReqParameters &params) override;
  void SchedUlSrInfoReq(const SchedUlSrInfoReqParameters &params) override;
  void
  SchedUlMacCtrlInfoReq(const SchedUlMacCtrlInfoReqParameters &params) override;
  void SchedUlCqiInfoReq(const SchedUlCqiInfoReqParameters &params) override;

private:
  C *m_scheduler;
};

template <class C>
MemberSchedSapProvider<C>::MemberSchedSapProvider(C *scheduler)
    : m_scheduler(scheduler) {}

template <class C>
void MemberSchedSapProvider<C>::SchedDlRlcBufferReq(
    const SchedDlRlcBufferReqParameters &params) {
  m_scheduler->DoSchedDlRlcBufferReq(params);
}

template <class C>
void MemberSchedSapProvider<C>::SchedDlPagingBufferReq(
    const SchedDlPagingBufferReqParameters &params) {
  m_scheduler->DoSchedDlPagingBufferReq(params);
}

template <class C>
void MemberSchedSapProvider<C>::SchedDlMacBufferReq(
    const SchedDlMacBufferReqParameters &params) {
  m_scheduler->DoSchedDlMacBufferReq(params);
}

template <class C>
void MemberSchedSapProvider<C>::SchedDlTriggerReq(
    const SchedDlTriggerReqParameters &params) {
  m_scheduler->DoSchedDlTriggerReq(params);
}

template <class C>
void MemberSchedSapProvider<C>::SchedDlRachInfoReq(
    const SchedDlRachInfoReqParameters &params) {
  m_scheduler->DoSchedDlRachInfoReq(params);
}

template <class C>
void MemberSchedSapProvider<C>::SchedDlCqiInfoReq(
    const SchedDlCqiInfoReqParameters &params) {
  m_scheduler->DoSchedDlCqiInfoReq(params);
}

template <class C>
void MemberSchedSapProvider<C>::SchedUlTriggerReq(
    const SchedUlTriggerReqParameters &params) {
  m_scheduler->DoSchedUlTriggerReq(params);
}

template <class C>
void MemberSchedSapProvider<C>::SchedUlNoiseInterferenceReq(
    const SchedUlNoiseInterferenceReqParameters &params) {
  m_scheduler->DoSchedUlNoiseInterferenceReq(params);
}

template <class C>
void MemberSchedSapProvider<C>::SchedUlSrInfoReq(
    const SchedUlSrInfoReqParameters &params) {
  m_scheduler->DoSchedUlSrInfoReq(params);
}

template <class C>
void MemberSchedSapProvider<C>::SchedUlMacCtrlInfoReq(
    const SchedUlMacCtrlInfoReqParameters &params) {
  m_scheduler->DoSchedUlMacCtrlInfoReq(params);
}

template <class C>
void MemberSchedSapProvider<C>::SchedUlCqiInfoReq(
    const SchedUlCqiInfoReqParameters &params) {
  m_scheduler->DoSchedUlCqiInfoReq(params);
}

} // namespace ns3

#endif
