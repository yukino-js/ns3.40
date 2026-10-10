
#ifndef EPC_ENB_S1_SAP_H
#define EPC_ENB_S1_SAP_H

#include "eps-bearer.h"

#include <ns3/ipv4-address.h>

#include <list>

namespace ns3 {

class EpcEnbS1SapProvider {
public:
  virtual ~EpcEnbS1SapProvider();

  virtual void InitialUeMessage(uint64_t imsi, uint16_t rnti) = 0;

  virtual void DoSendReleaseIndication(uint64_t imsi, uint16_t rnti,
                                       uint8_t bearerId) = 0;

  struct BearerToBeSwitched {
    uint8_t epsBearerId;
    uint32_t teid;
  };

  struct PathSwitchRequestParameters {
    uint16_t rnti;
    uint16_t cellId;
    uint32_t mmeUeS1Id;
    std::list<BearerToBeSwitched> bearersToBeSwitched;
  };

  virtual void PathSwitchRequest(PathSwitchRequestParameters params) = 0;

  virtual void UeContextRelease(uint16_t rnti) = 0;
};

class EpcEnbS1SapUser {
public:
  virtual ~EpcEnbS1SapUser();

  struct InitialContextSetupRequestParameters {
    uint16_t rnti;
  };

  virtual void
  InitialContextSetupRequest(InitialContextSetupRequestParameters params) = 0;

  struct DataRadioBearerSetupRequestParameters {
    uint16_t rnti;
    EpsBearer bearer;
    uint8_t bearerId;
    uint32_t gtpTeid;
    Ipv4Address transportLayerAddress;
  };

  virtual void
  DataRadioBearerSetupRequest(DataRadioBearerSetupRequestParameters params) = 0;

  struct PathSwitchRequestAcknowledgeParameters {
    uint16_t rnti;
  };

  virtual void PathSwitchRequestAcknowledge(
      PathSwitchRequestAcknowledgeParameters params) = 0;
};

template <class C>
class MemberEpcEnbS1SapProvider : public EpcEnbS1SapProvider {
public:
  MemberEpcEnbS1SapProvider(C *owner);

  MemberEpcEnbS1SapProvider() = delete;

  void InitialUeMessage(uint64_t imsi, uint16_t rnti) override;
  void DoSendReleaseIndication(uint64_t imsi, uint16_t rnti,
                               uint8_t bearerId) override;

  void PathSwitchRequest(PathSwitchRequestParameters params) override;
  void UeContextRelease(uint16_t rnti) override;

private:
  C *m_owner;
};

template <class C>
MemberEpcEnbS1SapProvider<C>::MemberEpcEnbS1SapProvider(C *owner)
    : m_owner(owner) {}

template <class C>
void MemberEpcEnbS1SapProvider<C>::InitialUeMessage(uint64_t imsi,
                                                    uint16_t rnti) {
  m_owner->DoInitialUeMessage(imsi, rnti);
}

template <class C>
void MemberEpcEnbS1SapProvider<C>::DoSendReleaseIndication(uint64_t imsi,
                                                           uint16_t rnti,
                                                           uint8_t bearerId) {
  m_owner->DoReleaseIndication(imsi, rnti, bearerId);
}

template <class C>
void MemberEpcEnbS1SapProvider<C>::PathSwitchRequest(
    PathSwitchRequestParameters params) {
  m_owner->DoPathSwitchRequest(params);
}

template <class C>
void MemberEpcEnbS1SapProvider<C>::UeContextRelease(uint16_t rnti) {
  m_owner->DoUeContextRelease(rnti);
}

template <class C> class MemberEpcEnbS1SapUser : public EpcEnbS1SapUser {
public:
  MemberEpcEnbS1SapUser(C *owner);

  MemberEpcEnbS1SapUser() = delete;

  void InitialContextSetupRequest(
      InitialContextSetupRequestParameters params) override;
  void DataRadioBearerSetupRequest(
      DataRadioBearerSetupRequestParameters params) override;
  void PathSwitchRequestAcknowledge(
      PathSwitchRequestAcknowledgeParameters params) override;

private:
  C *m_owner;
};

template <class C>
MemberEpcEnbS1SapUser<C>::MemberEpcEnbS1SapUser(C *owner) : m_owner(owner) {}

template <class C>
void MemberEpcEnbS1SapUser<C>::InitialContextSetupRequest(
    InitialContextSetupRequestParameters params) {
  m_owner->DoInitialContextSetupRequest(params);
}

template <class C>
void MemberEpcEnbS1SapUser<C>::DataRadioBearerSetupRequest(
    DataRadioBearerSetupRequestParameters params) {
  m_owner->DoDataRadioBearerSetupRequest(params);
}

template <class C>
void MemberEpcEnbS1SapUser<C>::PathSwitchRequestAcknowledge(
    PathSwitchRequestAcknowledgeParameters params) {
  m_owner->DoPathSwitchRequestAcknowledge(params);
}

} // namespace ns3

#endif
