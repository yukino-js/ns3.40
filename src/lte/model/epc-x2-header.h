
#ifndef EPC_X2_HEADER_H
#define EPC_X2_HEADER_H

#include "epc-x2-sap.h"

#include "ns3/header.h"

#include <vector>

namespace ns3 {

class EpcX2Header : public Header {
public:
  EpcX2Header();
  ~EpcX2Header() override;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;

  uint8_t GetMessageType() const;
  void SetMessageType(uint8_t messageType);

  uint8_t GetProcedureCode() const;
  void SetProcedureCode(uint8_t procedureCode);

  void SetLengthOfIes(uint32_t lengthOfIes);
  void SetNumberOfIes(uint32_t numberOfIes);

  enum ProcedureCode_t {
    HandoverPreparation = 0,
    HandoverCancel = 1,
    LoadIndication = 2,
    SnStatusTransfer = 4,
    UeContextRelease = 5,
    ResourceStatusReporting = 10
  };

  enum TypeOfMessage_t {
    InitiatingMessage = 0,
    SuccessfulOutcome = 1,
    UnsuccessfulOutcome = 2
  };

private:
  uint8_t m_messageType;
  uint8_t m_procedureCode;

  uint32_t m_lengthOfIes;
  uint32_t m_numberOfIes;
};

class EpcX2HandoverRequestHeader : public Header {
public:
  EpcX2HandoverRequestHeader();
  ~EpcX2HandoverRequestHeader() override;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;

  uint16_t GetOldEnbUeX2apId() const;
  void SetOldEnbUeX2apId(uint16_t x2apId);

  uint16_t GetCause() const;
  void SetCause(uint16_t cause);

  uint16_t GetTargetCellId() const;
  void SetTargetCellId(uint16_t targetCellId);

  uint32_t GetMmeUeS1apId() const;
  void SetMmeUeS1apId(uint32_t mmeUeS1apId);

  std::vector<EpcX2Sap::ErabToBeSetupItem> GetBearers() const;
  void SetBearers(std::vector<EpcX2Sap::ErabToBeSetupItem> bearers);

  uint64_t GetUeAggregateMaxBitRateDownlink() const;
  void SetUeAggregateMaxBitRateDownlink(uint64_t bitRate);

  uint64_t GetUeAggregateMaxBitRateUplink() const;
  void SetUeAggregateMaxBitRateUplink(uint64_t bitRate);

  uint32_t GetLengthOfIes() const;
  uint32_t GetNumberOfIes() const;

private:
  uint32_t m_numberOfIes;
  uint32_t m_headerLength;

  uint16_t m_oldEnbUeX2apId;
  uint16_t m_cause;
  uint16_t m_targetCellId;
  uint32_t m_mmeUeS1apId;
  uint64_t m_ueAggregateMaxBitRateDownlink;
  uint64_t m_ueAggregateMaxBitRateUplink;
  std::vector<EpcX2Sap::ErabToBeSetupItem> m_erabsToBeSetupList;
};

class EpcX2HandoverRequestAckHeader : public Header {
public:
  EpcX2HandoverRequestAckHeader();
  ~EpcX2HandoverRequestAckHeader() override;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;

  uint16_t GetOldEnbUeX2apId() const;
  void SetOldEnbUeX2apId(uint16_t x2apId);

  uint16_t GetNewEnbUeX2apId() const;
  void SetNewEnbUeX2apId(uint16_t x2apId);

  std::vector<EpcX2Sap::ErabAdmittedItem> GetAdmittedBearers() const;
  void SetAdmittedBearers(std::vector<EpcX2Sap::ErabAdmittedItem> bearers);

  std::vector<EpcX2Sap::ErabNotAdmittedItem> GetNotAdmittedBearers() const;
  void
  SetNotAdmittedBearers(std::vector<EpcX2Sap::ErabNotAdmittedItem> bearers);

  uint32_t GetLengthOfIes() const;
  uint32_t GetNumberOfIes() const;

private:
  uint32_t m_numberOfIes;
  uint32_t m_headerLength;

  uint16_t m_oldEnbUeX2apId;
  uint16_t m_newEnbUeX2apId;
  std::vector<EpcX2Sap::ErabAdmittedItem> m_erabsAdmittedList;
  std::vector<EpcX2Sap::ErabNotAdmittedItem> m_erabsNotAdmittedList;
};

class EpcX2HandoverPreparationFailureHeader : public Header {
public:
  EpcX2HandoverPreparationFailureHeader();
  ~EpcX2HandoverPreparationFailureHeader() override;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;

  uint16_t GetOldEnbUeX2apId() const;
  void SetOldEnbUeX2apId(uint16_t x2apId);

  uint16_t GetCause() const;
  void SetCause(uint16_t cause);

  uint16_t GetCriticalityDiagnostics() const;
  void SetCriticalityDiagnostics(uint16_t criticalityDiagnostics);

  uint32_t GetLengthOfIes() const;
  uint32_t GetNumberOfIes() const;

private:
  uint32_t m_numberOfIes;
  uint32_t m_headerLength;

  uint16_t m_oldEnbUeX2apId;
  uint16_t m_cause;
  uint16_t m_criticalityDiagnostics;
};

class EpcX2SnStatusTransferHeader : public Header {
public:
  EpcX2SnStatusTransferHeader();
  ~EpcX2SnStatusTransferHeader() override;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;

  uint16_t GetOldEnbUeX2apId() const;
  void SetOldEnbUeX2apId(uint16_t x2apId);

  uint16_t GetNewEnbUeX2apId() const;
  void SetNewEnbUeX2apId(uint16_t x2apId);

  std::vector<EpcX2Sap::ErabsSubjectToStatusTransferItem>
  GetErabsSubjectToStatusTransferList() const;
  void SetErabsSubjectToStatusTransferList(
      std::vector<EpcX2Sap::ErabsSubjectToStatusTransferItem> erabs);

  uint32_t GetLengthOfIes() const;
  uint32_t GetNumberOfIes() const;

private:
  uint32_t m_numberOfIes;
  uint32_t m_headerLength;

  uint16_t m_oldEnbUeX2apId;
  uint16_t m_newEnbUeX2apId;
  std::vector<EpcX2Sap::ErabsSubjectToStatusTransferItem>
      m_erabsSubjectToStatusTransferList;
};

class EpcX2UeContextReleaseHeader : public Header {
public:
  EpcX2UeContextReleaseHeader();
  ~EpcX2UeContextReleaseHeader() override;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;

  uint16_t GetOldEnbUeX2apId() const;
  void SetOldEnbUeX2apId(uint16_t x2apId);

  uint16_t GetNewEnbUeX2apId() const;
  void SetNewEnbUeX2apId(uint16_t x2apId);

  uint32_t GetLengthOfIes() const;
  uint32_t GetNumberOfIes() const;

private:
  uint32_t m_numberOfIes;
  uint32_t m_headerLength;

  uint16_t m_oldEnbUeX2apId;
  uint16_t m_newEnbUeX2apId;
};

class EpcX2LoadInformationHeader : public Header {
public:
  EpcX2LoadInformationHeader();
  ~EpcX2LoadInformationHeader() override;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;

  std::vector<EpcX2Sap::CellInformationItem> GetCellInformationList() const;
  void SetCellInformationList(
      std::vector<EpcX2Sap::CellInformationItem> cellInformationList);

  uint32_t GetLengthOfIes() const;
  uint32_t GetNumberOfIes() const;

private:
  uint32_t m_numberOfIes;
  uint32_t m_headerLength;

  std::vector<EpcX2Sap::CellInformationItem> m_cellInformationList;
};

class EpcX2ResourceStatusUpdateHeader : public Header {
public:
  EpcX2ResourceStatusUpdateHeader();
  ~EpcX2ResourceStatusUpdateHeader() override;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;

  uint16_t GetEnb1MeasurementId() const;
  void SetEnb1MeasurementId(uint16_t enb1MeasurementId);

  uint16_t GetEnb2MeasurementId() const;
  void SetEnb2MeasurementId(uint16_t enb2MeasurementId);

  std::vector<EpcX2Sap::CellMeasurementResultItem>
  GetCellMeasurementResultList() const;
  void
  SetCellMeasurementResultList(std::vector<EpcX2Sap::CellMeasurementResultItem>
                                   cellMeasurementResultList);

  uint32_t GetLengthOfIes() const;
  uint32_t GetNumberOfIes() const;

private:
  uint32_t m_numberOfIes;
  uint32_t m_headerLength;

  uint16_t m_enb1MeasurementId;
  uint16_t m_enb2MeasurementId;
  std::vector<EpcX2Sap::CellMeasurementResultItem> m_cellMeasurementResultList;
};

class EpcX2HandoverCancelHeader : public Header {
public:
  EpcX2HandoverCancelHeader();
  ~EpcX2HandoverCancelHeader() override;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;

  uint16_t GetOldEnbUeX2apId() const;
  void SetOldEnbUeX2apId(uint16_t x2apId);

  uint16_t GetNewEnbUeX2apId() const;
  void SetNewEnbUeX2apId(uint16_t x2apId);

  uint16_t GetCause() const;
  void SetCause(uint16_t cause);

  uint32_t GetLengthOfIes() const;
  uint32_t GetNumberOfIes() const;

private:
  uint32_t m_numberOfIes;
  uint32_t m_headerLength;

  uint16_t m_oldEnbUeX2apId;
  uint16_t m_newEnbUeX2apId;
  uint16_t m_cause;
};

} // namespace ns3

#endif
