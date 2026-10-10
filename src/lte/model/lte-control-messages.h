
#ifndef LTE_CONTROL_MESSAGES_H
#define LTE_CONTROL_MESSAGES_H

#include "ff-mac-common.h"
#include "lte-rrc-sap.h"

#include <ns3/ptr.h>
#include <ns3/simple-ref-count.h>

#include <list>

namespace ns3 {

class LteNetDevice;

class LteControlMessage : public SimpleRefCount<LteControlMessage> {
public:
  enum MessageType {
    DL_DCI,
    UL_DCI,
    DL_CQI,
    UL_CQI,
    BSR,
    DL_HARQ,
    RACH_PREAMBLE,
    RAR,
    MIB,
    SIB1,
  };

  LteControlMessage();
  virtual ~LteControlMessage();

  void SetMessageType(MessageType type);
  MessageType GetMessageType();

private:
  MessageType m_type;
};

class DlDciLteControlMessage : public LteControlMessage {
public:
  DlDciLteControlMessage();
  ~DlDciLteControlMessage() override;

  void SetDci(DlDciListElement_s dci);

  const DlDciListElement_s &GetDci();

private:
  DlDciListElement_s m_dci;
};

class UlDciLteControlMessage : public LteControlMessage {
public:
  UlDciLteControlMessage();
  ~UlDciLteControlMessage() override;

  void SetDci(UlDciListElement_s dci);

  const UlDciListElement_s &GetDci();

private:
  UlDciListElement_s m_dci;
};

class DlCqiLteControlMessage : public LteControlMessage {
public:
  DlCqiLteControlMessage();
  ~DlCqiLteControlMessage() override;

  void SetDlCqi(CqiListElement_s dlcqi);

  CqiListElement_s GetDlCqi();

private:
  CqiListElement_s m_dlCqi;
};

class BsrLteControlMessage : public LteControlMessage {
public:
  BsrLteControlMessage();
  ~BsrLteControlMessage() override;

  void SetBsr(MacCeListElement_s bsr);

  MacCeListElement_s GetBsr();

private:
  MacCeListElement_s m_bsr;
};

class DlHarqFeedbackLteControlMessage : public LteControlMessage {
public:
  DlHarqFeedbackLteControlMessage();
  ~DlHarqFeedbackLteControlMessage() override;

  void SetDlHarqFeedback(DlInfoListElement_s m);

  DlInfoListElement_s GetDlHarqFeedback();

private:
  DlInfoListElement_s m_dlInfoListElement;
};

class RachPreambleLteControlMessage : public LteControlMessage {
public:
  RachPreambleLteControlMessage();

  void SetRapId(uint32_t rapid);

  uint32_t GetRapId() const;

private:
  uint32_t m_rapId;
};

class RarLteControlMessage : public LteControlMessage {
public:
  RarLteControlMessage();

  void SetRaRnti(uint16_t raRnti);

  uint16_t GetRaRnti() const;

  struct Rar {
    uint8_t rapId;
    BuildRarListElement_s rarPayload;
  };

  void AddRar(Rar rar);

  std::list<Rar>::const_iterator RarListBegin() const;

  std::list<Rar>::const_iterator RarListEnd() const;

private:
  std::list<Rar> m_rarList;
  uint16_t m_raRnti;
};

class MibLteControlMessage : public LteControlMessage {
public:
  MibLteControlMessage();

  void SetMib(LteRrcSap::MasterInformationBlock mib);

  LteRrcSap::MasterInformationBlock GetMib() const;

private:
  LteRrcSap::MasterInformationBlock m_mib;
};

class Sib1LteControlMessage : public LteControlMessage {
public:
  Sib1LteControlMessage();

  void SetSib1(LteRrcSap::SystemInformationBlockType1 sib1);

  LteRrcSap::SystemInformationBlockType1 GetSib1() const;

private:
  LteRrcSap::SystemInformationBlockType1 m_sib1;
};

} // namespace ns3

#endif
