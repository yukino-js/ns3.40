
#ifndef EPC_S11_SAP_H
#define EPC_S11_SAP_H

#include "epc-tft.h"
#include "eps-bearer.h"

#include <ns3/address.h>
#include <ns3/object.h>
#include <ns3/ptr.h>

#include <list>

namespace ns3 {

class EpcS11Sap {
public:
  virtual ~EpcS11Sap();

  struct GtpcMessage {
    uint32_t teid;
  };

  struct Fteid {
    uint32_t teid;
    Ipv4Address address;
  };

  struct Uli {
    uint16_t gci;
  };
};

class EpcS11SapMme : public EpcS11Sap {
public:
  struct BearerContextCreated {
    EpcS11Sap::Fteid sgwFteid;
    uint8_t epsBearerId;
    EpsBearer bearerLevelQos;
    Ptr<EpcTft> tft;
  };

  struct CreateSessionResponseMessage : public GtpcMessage {
    std::list<BearerContextCreated> bearerContextsCreated;
  };

  virtual void CreateSessionResponse(CreateSessionResponseMessage msg) = 0;

  struct BearerContextRemoved {
    uint8_t epsBearerId;
  };

  struct DeleteBearerRequestMessage : public GtpcMessage {
    std::list<BearerContextRemoved> bearerContextsRemoved;
  };

  virtual void DeleteBearerRequest(DeleteBearerRequestMessage msg) = 0;

  struct ModifyBearerResponseMessage : public GtpcMessage {
    enum Cause {
      REQUEST_ACCEPTED = 0,
      REQUEST_ACCEPTED_PARTIALLY,
      REQUEST_REJECTED,
      CONTEXT_NOT_FOUND
    };

    Cause cause;
  };

  virtual void ModifyBearerResponse(ModifyBearerResponseMessage msg) = 0;
};

class EpcS11SapSgw : public EpcS11Sap {
public:
  struct BearerContextToBeCreated {
    EpcS11Sap::Fteid sgwFteid;
    uint8_t epsBearerId;
    EpsBearer bearerLevelQos;
    Ptr<EpcTft> tft;
  };

  struct CreateSessionRequestMessage : public GtpcMessage {
    uint64_t imsi;
    Uli uli;
    std::list<BearerContextToBeCreated> bearerContextsToBeCreated;
  };

  virtual void CreateSessionRequest(CreateSessionRequestMessage msg) = 0;

  struct BearerContextToBeRemoved {
    uint8_t epsBearerId;
  };

  struct DeleteBearerCommandMessage : public GtpcMessage {
    std::list<BearerContextToBeRemoved> bearerContextsToBeRemoved;
  };

  virtual void DeleteBearerCommand(DeleteBearerCommandMessage msg) = 0;

  struct BearerContextRemovedSgwPgw {
    uint8_t epsBearerId;
  };

  struct DeleteBearerResponseMessage : public GtpcMessage {
    std::list<BearerContextRemovedSgwPgw> bearerContextsRemoved;
  };

  virtual void DeleteBearerResponse(DeleteBearerResponseMessage msg) = 0;

  struct ModifyBearerRequestMessage : public GtpcMessage {
    Uli uli;
  };

  virtual void ModifyBearerRequest(ModifyBearerRequestMessage msg) = 0;
};

template <class C> class MemberEpcS11SapMme : public EpcS11SapMme {
public:
  MemberEpcS11SapMme(C *owner);

  MemberEpcS11SapMme() = delete;

  void CreateSessionResponse(CreateSessionResponseMessage msg) override;
  void ModifyBearerResponse(ModifyBearerResponseMessage msg) override;
  void DeleteBearerRequest(DeleteBearerRequestMessage msg) override;

private:
  C *m_owner;
};

template <class C>
MemberEpcS11SapMme<C>::MemberEpcS11SapMme(C *owner) : m_owner(owner) {}

template <class C>
void MemberEpcS11SapMme<C>::CreateSessionResponse(
    CreateSessionResponseMessage msg) {
  m_owner->DoCreateSessionResponse(msg);
}

template <class C>
void MemberEpcS11SapMme<C>::DeleteBearerRequest(
    DeleteBearerRequestMessage msg) {
  m_owner->DoDeleteBearerRequest(msg);
}

template <class C>
void MemberEpcS11SapMme<C>::ModifyBearerResponse(
    ModifyBearerResponseMessage msg) {
  m_owner->DoModifyBearerResponse(msg);
}

template <class C> class MemberEpcS11SapSgw : public EpcS11SapSgw {
public:
  MemberEpcS11SapSgw(C *owner);

  MemberEpcS11SapSgw() = delete;

  void CreateSessionRequest(CreateSessionRequestMessage msg) override;
  void ModifyBearerRequest(ModifyBearerRequestMessage msg) override;
  void DeleteBearerCommand(DeleteBearerCommandMessage msg) override;
  void DeleteBearerResponse(DeleteBearerResponseMessage msg) override;

private:
  C *m_owner;
};

template <class C>
MemberEpcS11SapSgw<C>::MemberEpcS11SapSgw(C *owner) : m_owner(owner) {}

template <class C>
void MemberEpcS11SapSgw<C>::CreateSessionRequest(
    CreateSessionRequestMessage msg) {
  m_owner->DoCreateSessionRequest(msg);
}

template <class C>
void MemberEpcS11SapSgw<C>::ModifyBearerRequest(
    ModifyBearerRequestMessage msg) {
  m_owner->DoModifyBearerRequest(msg);
}

template <class C>
void MemberEpcS11SapSgw<C>::DeleteBearerCommand(
    DeleteBearerCommandMessage msg) {
  m_owner->DoDeleteBearerCommand(msg);
}

template <class C>
void MemberEpcS11SapSgw<C>::DeleteBearerResponse(
    DeleteBearerResponseMessage msg) {
  m_owner->DoDeleteBearerResponse(msg);
}

} // namespace ns3

#endif
