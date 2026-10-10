
#ifndef EPC_S1AP_SAP_H
#define EPC_S1AP_SAP_H

#include "eps-bearer.h"

#include <ns3/address.h>
#include <ns3/ipv4-address.h>
#include <ns3/object.h>
#include <ns3/ptr.h>

#include <list>

namespace ns3 {

class EpcS1apSap {
public:
  virtual ~EpcS1apSap();
};

class EpcS1apSapMme : public EpcS1apSap {
public:
  virtual void InitialUeMessage(uint64_t mmeUeS1Id, uint16_t enbUeS1Id,
                                uint64_t stmsi, uint16_t ecgi) = 0;

  struct ErabToBeReleasedIndication {
    uint8_t erabId;
  };

  virtual void ErabReleaseIndication(
      uint64_t mmeUeS1Id, uint16_t enbUeS1Id,
      std::list<ErabToBeReleasedIndication> erabToBeReleaseIndication) = 0;

  struct ErabSetupItem {
    uint16_t erabId;
    Ipv4Address enbTransportLayerAddress;
    uint32_t enbTeid;
  };

  virtual void
  InitialContextSetupResponse(uint64_t mmeUeS1Id, uint16_t enbUeS1Id,
                              std::list<ErabSetupItem> erabSetupList) = 0;

  struct ErabSwitchedInDownlinkItem {
    uint16_t erabId;
    Ipv4Address enbTransportLayerAddress;
    uint32_t enbTeid;
  };

  virtual void PathSwitchRequest(
      uint64_t enbUeS1Id, uint64_t mmeUeS1Id, uint16_t gci,
      std::list<ErabSwitchedInDownlinkItem> erabToBeSwitchedInDownlinkList) = 0;
};

class EpcS1apSapEnb : public EpcS1apSap {
public:
  struct ErabToBeSetupItem {
    uint8_t erabId;
    EpsBearer erabLevelQosParameters;
    Ipv4Address transportLayerAddress;
    uint32_t sgwTeid;
  };

  virtual void InitialContextSetupRequest(
      uint64_t mmeUeS1Id, uint16_t enbUeS1Id,
      std::list<ErabToBeSetupItem> erabToBeSetupList) = 0;

  struct ErabSwitchedInUplinkItem {
    uint8_t erabId;
    Ipv4Address transportLayerAddress;
    uint32_t enbTeid;
  };

  virtual void PathSwitchRequestAcknowledge(
      uint64_t enbUeS1Id, uint64_t mmeUeS1Id, uint16_t cgi,
      std::list<ErabSwitchedInUplinkItem> erabToBeSwitchedInUplinkList) = 0;
};

template <class C> class MemberEpcS1apSapMme : public EpcS1apSapMme {
public:
  MemberEpcS1apSapMme(C *owner);

  MemberEpcS1apSapMme() = delete;

  void InitialUeMessage(uint64_t mmeUeS1Id, uint16_t enbUeS1Id, uint64_t imsi,
                        uint16_t ecgi) override;
  void ErabReleaseIndication(
      uint64_t mmeUeS1Id, uint16_t enbUeS1Id,
      std::list<ErabToBeReleasedIndication> erabToBeReleaseIndication) override;

  void
  InitialContextSetupResponse(uint64_t mmeUeS1Id, uint16_t enbUeS1Id,
                              std::list<ErabSetupItem> erabSetupList) override;
  void PathSwitchRequest(uint64_t enbUeS1Id, uint64_t mmeUeS1Id, uint16_t cgi,
                         std::list<ErabSwitchedInDownlinkItem>
                             erabToBeSwitchedInDownlinkList) override;

private:
  C *m_owner;
};

template <class C>
MemberEpcS1apSapMme<C>::MemberEpcS1apSapMme(C *owner) : m_owner(owner) {}

template <class C>
void MemberEpcS1apSapMme<C>::InitialUeMessage(uint64_t mmeUeS1Id,
                                              uint16_t enbUeS1Id, uint64_t imsi,
                                              uint16_t ecgi) {
  m_owner->DoInitialUeMessage(mmeUeS1Id, enbUeS1Id, imsi, ecgi);
}

template <class C>
void MemberEpcS1apSapMme<C>::ErabReleaseIndication(
    uint64_t mmeUeS1Id, uint16_t enbUeS1Id,
    std::list<ErabToBeReleasedIndication> erabToBeReleaseIndication) {
  m_owner->DoErabReleaseIndication(mmeUeS1Id, enbUeS1Id,
                                   erabToBeReleaseIndication);
}

template <class C>
void MemberEpcS1apSapMme<C>::InitialContextSetupResponse(
    uint64_t mmeUeS1Id, uint16_t enbUeS1Id,
    std::list<ErabSetupItem> erabSetupList) {
  m_owner->DoInitialContextSetupResponse(mmeUeS1Id, enbUeS1Id, erabSetupList);
}

template <class C>
void MemberEpcS1apSapMme<C>::PathSwitchRequest(
    uint64_t enbUeS1Id, uint64_t mmeUeS1Id, uint16_t cgi,
    std::list<ErabSwitchedInDownlinkItem> erabToBeSwitchedInDownlinkList) {
  m_owner->DoPathSwitchRequest(enbUeS1Id, mmeUeS1Id, cgi,
                               erabToBeSwitchedInDownlinkList);
}

template <class C> class MemberEpcS1apSapEnb : public EpcS1apSapEnb {
public:
  MemberEpcS1apSapEnb(C *owner);

  MemberEpcS1apSapEnb() = delete;

  void InitialContextSetupRequest(
      uint64_t mmeUeS1Id, uint16_t enbUeS1Id,
      std::list<ErabToBeSetupItem> erabToBeSetupList) override;
  void PathSwitchRequestAcknowledge(uint64_t enbUeS1Id, uint64_t mmeUeS1Id,
                                    uint16_t cgi,
                                    std::list<ErabSwitchedInUplinkItem>
                                        erabToBeSwitchedInUplinkList) override;

private:
  C *m_owner;
};

template <class C>
MemberEpcS1apSapEnb<C>::MemberEpcS1apSapEnb(C *owner) : m_owner(owner) {}

template <class C>
void MemberEpcS1apSapEnb<C>::InitialContextSetupRequest(
    uint64_t mmeUeS1Id, uint16_t enbUeS1Id,
    std::list<ErabToBeSetupItem> erabToBeSetupList) {
  m_owner->DoInitialContextSetupRequest(mmeUeS1Id, enbUeS1Id,
                                        erabToBeSetupList);
}

template <class C>
void MemberEpcS1apSapEnb<C>::PathSwitchRequestAcknowledge(
    uint64_t enbUeS1Id, uint64_t mmeUeS1Id, uint16_t cgi,
    std::list<ErabSwitchedInUplinkItem> erabToBeSwitchedInUplinkList) {
  m_owner->DoPathSwitchRequestAcknowledge(enbUeS1Id, mmeUeS1Id, cgi,
                                          erabToBeSwitchedInUplinkList);
}

} // namespace ns3

#endif
