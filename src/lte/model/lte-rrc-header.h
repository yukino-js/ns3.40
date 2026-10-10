
#ifndef RRC_HEADER_H
#define RRC_HEADER_H

#include "lte-asn1-header.h"
#include "lte-rrc-sap.h"

#include "ns3/header.h"

#include <bitset>
#include <string>

namespace ns3 {

class RrcAsn1Header : public Asn1Header {
public:
  RrcAsn1Header();
  int GetMessageType() const;

protected:
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t Deserialize(Buffer::Iterator bIterator) override = 0;
  void PreSerialize() const override = 0;

  int BandwidthToEnum(uint16_t bandwidth) const;
  uint16_t EnumToBandwidth(int n) const;

  void SerializeSrbToAddModList(
      std::list<LteRrcSap::SrbToAddMod> srbToAddModList) const;
  void SerializeDrbToAddModList(
      std::list<LteRrcSap::DrbToAddMod> drbToAddModList) const;
  void SerializeLogicalChannelConfig(
      LteRrcSap::LogicalChannelConfig logicalChannelConfig) const;
  void SerializeRadioResourceConfigDedicated(
      LteRrcSap::RadioResourceConfigDedicated radioResourceConfigDedicated)
      const;
  void SerializePhysicalConfigDedicated(
      LteRrcSap::PhysicalConfigDedicated physicalConfigDedicated) const;
  void SerializePhysicalConfigDedicatedSCell(
      LteRrcSap::PhysicalConfigDedicatedSCell pcdsc) const;
  void SerializeSystemInformationBlockType1(
      LteRrcSap::SystemInformationBlockType1 systemInformationBlockType1) const;
  void SerializeSystemInformationBlockType2(
      LteRrcSap::SystemInformationBlockType2 systemInformationBlockType2) const;
  void SerializeRadioResourceConfigCommon(
      LteRrcSap::RadioResourceConfigCommon radioResourceConfigCommon) const;
  void SerializeRadioResourceConfigCommonSib(
      LteRrcSap::RadioResourceConfigCommonSib radioResourceConfigCommonSib)
      const;
  void SerializeMeasResults(LteRrcSap::MeasResults measResults) const;
  void SerializePlmnIdentity(uint32_t plmnId) const;
  void
  SerializeRachConfigCommon(LteRrcSap::RachConfigCommon rachConfigCommon) const;
  void SerializeMeasConfig(LteRrcSap::MeasConfig measConfig) const;
  void SerializeNonCriticalExtensionConfiguration(
      LteRrcSap::NonCriticalExtensionConfiguration
          nonCriticalExtensionConfiguration) const;
  void SerializeRadioResourceConfigCommonSCell(
      LteRrcSap::RadioResourceConfigCommonSCell rrccsc) const;
  void SerializeRadioResourceDedicatedSCell(
      LteRrcSap::RadioResourceConfigDedicatedSCell rrcdsc) const;
  void SerializeQoffsetRange(int8_t qOffsetRange) const;
  void SerializeThresholdEutra(LteRrcSap::ThresholdEutra thresholdEutra) const;

  Buffer::Iterator
  DeserializeDrbToAddModList(std::list<LteRrcSap::DrbToAddMod> *drbToAddModLis,
                             Buffer::Iterator bIterator);
  Buffer::Iterator
  DeserializeSrbToAddModList(std::list<LteRrcSap::SrbToAddMod> *srbToAddModList,
                             Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeLogicalChannelConfig(
      LteRrcSap::LogicalChannelConfig *logicalChannelConfig,
      Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeRadioResourceConfigDedicated(
      LteRrcSap::RadioResourceConfigDedicated *radioResourceConfigDedicated,
      Buffer::Iterator bIterator);
  Buffer::Iterator DeserializePhysicalConfigDedicated(
      LteRrcSap::PhysicalConfigDedicated *physicalConfigDedicated,
      Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeSystemInformationBlockType1(
      LteRrcSap::SystemInformationBlockType1 *systemInformationBlockType1,
      Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeSystemInformationBlockType2(
      LteRrcSap::SystemInformationBlockType2 *systemInformationBlockType2,
      Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeRadioResourceConfigCommon(
      LteRrcSap::RadioResourceConfigCommon *radioResourceConfigCommon,
      Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeRadioResourceConfigCommonSib(
      LteRrcSap::RadioResourceConfigCommonSib *radioResourceConfigCommonSib,
      Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeMeasResults(LteRrcSap::MeasResults *measResults,
                                          Buffer::Iterator bIterator);
  Buffer::Iterator DeserializePlmnIdentity(uint32_t *plmnId,
                                           Buffer::Iterator bIterator);
  Buffer::Iterator
  DeserializeRachConfigCommon(LteRrcSap::RachConfigCommon *rachConfigCommon,
                              Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeMeasConfig(LteRrcSap::MeasConfig *measConfig,
                                         Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeQoffsetRange(int8_t *qOffsetRange,
                                           Buffer::Iterator bIterator);
  Buffer::Iterator
  DeserializeThresholdEutra(LteRrcSap::ThresholdEutra *thresholdEutra,
                            Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeNonCriticalExtensionConfig(
      LteRrcSap::NonCriticalExtensionConfiguration *nonCriticalExtension,
      Buffer::Iterator bIterator);
  Buffer::Iterator
  DeserializeCellIdentification(LteRrcSap::CellIdentification *ci,
                                Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeRadioResourceConfigCommonSCell(
      LteRrcSap::RadioResourceConfigCommonSCell *rrccsc,
      Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeRadioResourceConfigDedicatedSCell(
      LteRrcSap::RadioResourceConfigDedicatedSCell *rrcdsc,
      Buffer::Iterator bIterator);
  Buffer::Iterator DeserializePhysicalConfigDedicatedSCell(
      LteRrcSap::PhysicalConfigDedicatedSCell *pcdsc,
      Buffer::Iterator bIterator);

  void Print(std::ostream &os) const override;
  void Print(std::ostream &os, LteRrcSap::RadioResourceConfigDedicated
                                   radioResourceConfigDedicated) const;

  int m_messageType;
};

class RrcUlDcchMessage : public RrcAsn1Header {
public:
  RrcUlDcchMessage();
  ~RrcUlDcchMessage() override;

  uint32_t Deserialize(Buffer::Iterator bIterator) override;
  void Print(std::ostream &os) const override;
  void PreSerialize() const override;

protected:
  void SerializeUlDcchMessage(int msgType) const;
  Buffer::Iterator DeserializeUlDcchMessage(Buffer::Iterator bIterator);
};

class RrcDlDcchMessage : public RrcAsn1Header {
public:
  RrcDlDcchMessage();
  ~RrcDlDcchMessage() override;

  uint32_t Deserialize(Buffer::Iterator bIterator) override;
  void Print(std::ostream &os) const override;
  void PreSerialize() const override;

protected:
  void SerializeDlDcchMessage(int msgType) const;
  Buffer::Iterator DeserializeDlDcchMessage(Buffer::Iterator bIterator);
};

class RrcUlCcchMessage : public RrcAsn1Header {
public:
  RrcUlCcchMessage();
  ~RrcUlCcchMessage() override;

  uint32_t Deserialize(Buffer::Iterator bIterator) override;
  void Print(std::ostream &os) const override;
  void PreSerialize() const override;

protected:
  void SerializeUlCcchMessage(int msgType) const;
  Buffer::Iterator DeserializeUlCcchMessage(Buffer::Iterator bIterator);
};

class RrcDlCcchMessage : public RrcAsn1Header {
public:
  RrcDlCcchMessage();
  ~RrcDlCcchMessage() override;

  uint32_t Deserialize(Buffer::Iterator bIterator) override;
  void Print(std::ostream &os) const override;
  void PreSerialize() const override;

protected:
  void SerializeDlCcchMessage(int msgType) const;
  Buffer::Iterator DeserializeDlCcchMessage(Buffer::Iterator bIterator);
};

class RrcConnectionRequestHeader : public RrcUlCcchMessage {
public:
  RrcConnectionRequestHeader();
  ~RrcConnectionRequestHeader() override;

  static TypeId GetTypeId();
  void PreSerialize() const override;
  uint32_t Deserialize(Buffer::Iterator bIterator) override;
  void Print(std::ostream &os) const override;

  void SetMessage(LteRrcSap::RrcConnectionRequest msg);

  LteRrcSap::RrcConnectionRequest GetMessage() const;

  std::bitset<8> GetMmec() const;

  std::bitset<32> GetMtmsi() const;

private:
  std::bitset<8> m_mmec;
  std::bitset<32> m_mTmsi;

  enum {
    EMERGENCY = 0,
    HIGHPRIORITYACCESS,
    MT_ACCESS,
    MO_SIGNALLING,
    MO_DATA,
    SPARE3,
    SPARE2,
    SPARE1
  } m_establishmentCause;

  std::bitset<1> m_spare;
};

class RrcConnectionSetupHeader : public RrcDlCcchMessage {
public:
  RrcConnectionSetupHeader();
  ~RrcConnectionSetupHeader() override;

  void PreSerialize() const override;
  uint32_t Deserialize(Buffer::Iterator bIterator) override;
  void Print(std::ostream &os) const override;

  void SetMessage(LteRrcSap::RrcConnectionSetup msg);

  LteRrcSap::RrcConnectionSetup GetMessage() const;

  uint8_t GetRrcTransactionIdentifier() const;

  LteRrcSap::RadioResourceConfigDedicated
  GetRadioResourceConfigDedicated() const;

  bool HavePhysicalConfigDedicated() const;

  LteRrcSap::PhysicalConfigDedicated GetPhysicalConfigDedicated() const;

  std::list<LteRrcSap::SrbToAddMod> GetSrbToAddModList() const;

  std::list<LteRrcSap::DrbToAddMod> GetDrbToAddModList() const;

  std::list<uint8_t> GetDrbToReleaseList() const;

private:
  uint8_t m_rrcTransactionIdentifier;
  mutable LteRrcSap::RadioResourceConfigDedicated
      m_radioResourceConfigDedicated;
};

class RrcConnectionSetupCompleteHeader : public RrcUlDcchMessage {
public:
  RrcConnectionSetupCompleteHeader();
  ~RrcConnectionSetupCompleteHeader() override;

  void PreSerialize() const override;
  uint32_t Deserialize(Buffer::Iterator bIterator) override;
  void Print(std::ostream &os) const override;

  void SetMessage(LteRrcSap::RrcConnectionSetupCompleted msg);

  LteRrcSap::RrcConnectionSetupCompleted GetMessage() const;

  uint8_t GetRrcTransactionIdentifier() const;

private:
  uint8_t m_rrcTransactionIdentifier;
};

class RrcConnectionReconfigurationCompleteHeader : public RrcUlDcchMessage {
public:
  RrcConnectionReconfigurationCompleteHeader();
  ~RrcConnectionReconfigurationCompleteHeader() override;

  void PreSerialize() const override;
  uint32_t Deserialize(Buffer::Iterator bIterator) override;
  void Print(std::ostream &os) const override;

  void SetMessage(LteRrcSap::RrcConnectionReconfigurationCompleted msg);

  LteRrcSap::RrcConnectionReconfigurationCompleted GetMessage() const;

  uint8_t GetRrcTransactionIdentifier() const;

private:
  uint8_t m_rrcTransactionIdentifier;
};

class RrcConnectionReconfigurationHeader : public RrcDlDcchMessage {
public:
  RrcConnectionReconfigurationHeader();
  ~RrcConnectionReconfigurationHeader() override;

  void PreSerialize() const override;
  uint32_t Deserialize(Buffer::Iterator bIterator) override;
  void Print(std::ostream &os) const override;

  void SetMessage(LteRrcSap::RrcConnectionReconfiguration msg);

  LteRrcSap::RrcConnectionReconfiguration GetMessage() const;

  bool GetHaveMeasConfig() const;

  LteRrcSap::MeasConfig GetMeasConfig();

  bool GetHaveMobilityControlInfo() const;

  LteRrcSap::MobilityControlInfo GetMobilityControlInfo();

  bool GetHaveRadioResourceConfigDedicated() const;

  LteRrcSap::RadioResourceConfigDedicated GetRadioResourceConfigDedicated();

  uint8_t GetRrcTransactionIdentifier() const;

  LteRrcSap::RadioResourceConfigDedicated
  GetRadioResourceConfigDedicated() const;

  bool GetHaveNonCriticalExtensionConfig() const;

  LteRrcSap::NonCriticalExtensionConfiguration GetNonCriticalExtensionConfig();

  bool HavePhysicalConfigDedicated() const;

  LteRrcSap::PhysicalConfigDedicated GetPhysicalConfigDedicated() const;

  std::list<LteRrcSap::SrbToAddMod> GetSrbToAddModList() const;

  std::list<LteRrcSap::DrbToAddMod> GetDrbToAddModList() const;

  std::list<uint8_t> GetDrbToReleaseList() const;

private:
  uint8_t m_rrcTransactionIdentifier;
  bool m_haveMeasConfig;
  LteRrcSap::MeasConfig m_measConfig;
  bool m_haveMobilityControlInfo;
  LteRrcSap::MobilityControlInfo m_mobilityControlInfo;
  bool m_haveRadioResourceConfigDedicated;
  LteRrcSap::RadioResourceConfigDedicated m_radioResourceConfigDedicated;
  bool m_haveNonCriticalExtension;
  LteRrcSap::NonCriticalExtensionConfiguration m_nonCriticalExtension;
};

class HandoverPreparationInfoHeader : public RrcAsn1Header {
public:
  HandoverPreparationInfoHeader();

  void PreSerialize() const override;
  uint32_t Deserialize(Buffer::Iterator bIterator) override;
  void Print(std::ostream &os) const override;

  void SetMessage(LteRrcSap::HandoverPreparationInfo msg);

  LteRrcSap::HandoverPreparationInfo GetMessage() const;

  LteRrcSap::AsConfig GetAsConfig() const;

private:
  LteRrcSap::AsConfig m_asConfig;
};

class RrcConnectionReestablishmentRequestHeader : public RrcUlCcchMessage {
public:
  RrcConnectionReestablishmentRequestHeader();
  ~RrcConnectionReestablishmentRequestHeader() override;

  void PreSerialize() const override;
  uint32_t Deserialize(Buffer::Iterator bIterator) override;
  void Print(std::ostream &os) const override;

  void SetMessage(LteRrcSap::RrcConnectionReestablishmentRequest msg);

  LteRrcSap::RrcConnectionReestablishmentRequest GetMessage() const;

  LteRrcSap::ReestabUeIdentity GetUeIdentity() const;

  LteRrcSap::ReestablishmentCause GetReestablishmentCause() const;

private:
  LteRrcSap::ReestabUeIdentity m_ueIdentity;
  LteRrcSap::ReestablishmentCause m_reestablishmentCause;
};

class RrcConnectionReestablishmentHeader : public RrcDlCcchMessage {
public:
  RrcConnectionReestablishmentHeader();
  ~RrcConnectionReestablishmentHeader() override;

  void PreSerialize() const override;
  uint32_t Deserialize(Buffer::Iterator bIterator) override;
  void Print(std::ostream &os) const override;

  void SetMessage(LteRrcSap::RrcConnectionReestablishment msg);

  LteRrcSap::RrcConnectionReestablishment GetMessage() const;

  uint8_t GetRrcTransactionIdentifier() const;

  LteRrcSap::RadioResourceConfigDedicated
  GetRadioResourceConfigDedicated() const;

private:
  uint8_t m_rrcTransactionIdentifier;
  LteRrcSap::RadioResourceConfigDedicated m_radioResourceConfigDedicated;
};

class RrcConnectionReestablishmentCompleteHeader : public RrcUlDcchMessage {
public:
  RrcConnectionReestablishmentCompleteHeader();

  void PreSerialize() const override;
  uint32_t Deserialize(Buffer::Iterator bIterator) override;
  void Print(std::ostream &os) const override;

  void SetMessage(LteRrcSap::RrcConnectionReestablishmentComplete msg);

  LteRrcSap::RrcConnectionReestablishmentComplete GetMessage() const;

  uint8_t GetRrcTransactionIdentifier() const;

private:
  uint8_t m_rrcTransactionIdentifier;
};

class RrcConnectionReestablishmentRejectHeader : public RrcDlCcchMessage {
public:
  RrcConnectionReestablishmentRejectHeader();
  ~RrcConnectionReestablishmentRejectHeader() override;

  void PreSerialize() const override;
  uint32_t Deserialize(Buffer::Iterator bIterator) override;
  void Print(std::ostream &os) const override;

  void SetMessage(LteRrcSap::RrcConnectionReestablishmentReject msg);

  LteRrcSap::RrcConnectionReestablishmentReject GetMessage() const;

private:
  LteRrcSap::RrcConnectionReestablishmentReject
      m_rrcConnectionReestablishmentReject;
};

class RrcConnectionReleaseHeader : public RrcDlDcchMessage {
public:
  RrcConnectionReleaseHeader();
  ~RrcConnectionReleaseHeader() override;

  void PreSerialize() const override;
  uint32_t Deserialize(Buffer::Iterator bIterator) override;
  void Print(std::ostream &os) const override;

  void SetMessage(LteRrcSap::RrcConnectionRelease msg);

  LteRrcSap::RrcConnectionRelease GetMessage() const;

private:
  LteRrcSap::RrcConnectionRelease m_rrcConnectionRelease;
};

class RrcConnectionRejectHeader : public RrcDlCcchMessage {
public:
  RrcConnectionRejectHeader();
  ~RrcConnectionRejectHeader() override;

  void PreSerialize() const override;
  uint32_t Deserialize(Buffer::Iterator bIterator) override;
  void Print(std::ostream &os) const override;

  void SetMessage(LteRrcSap::RrcConnectionReject msg);

  LteRrcSap::RrcConnectionReject GetMessage() const;

private:
  LteRrcSap::RrcConnectionReject m_rrcConnectionReject;
};

class MeasurementReportHeader : public RrcUlDcchMessage {
public:
  MeasurementReportHeader();
  ~MeasurementReportHeader() override;

  void PreSerialize() const override;
  uint32_t Deserialize(Buffer::Iterator bIterator) override;
  void Print(std::ostream &os) const override;

  void SetMessage(LteRrcSap::MeasurementReport msg);

  LteRrcSap::MeasurementReport GetMessage() const;

private:
  LteRrcSap::MeasurementReport m_measurementReport;
};

} // namespace ns3

#endif
