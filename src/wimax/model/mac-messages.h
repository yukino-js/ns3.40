

#ifndef MANAGEMENT_MESSAGE_TYPE_H
#define MANAGEMENT_MESSAGE_TYPE_H

#include "ns3/header.h"

#include <stdint.h>

namespace ns3 {

class ManagementMessageType : public Header {
public:
  enum MessageType {
    MESSAGE_TYPE_UCD = 0,
    MESSAGE_TYPE_DCD = 1,
    MESSAGE_TYPE_DL_MAP = 2,
    MESSAGE_TYPE_UL_MAP = 3,
    MESSAGE_TYPE_RNG_REQ = 4,
    MESSAGE_TYPE_RNG_RSP = 5,
    MESSAGE_TYPE_REG_REQ = 6,
    MESSAGE_TYPE_REG_RSP = 7,
    MESSAGE_TYPE_DSA_REQ = 11,
    MESSAGE_TYPE_DSA_RSP = 12,
    MESSAGE_TYPE_DSA_ACK = 13
  };

  ManagementMessageType();
  ManagementMessageType(uint8_t type);
  ~ManagementMessageType() override;
  void SetType(uint8_t type);
  uint8_t GetType() const;

  std::string GetName() const;
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint8_t m_type;
};

} // namespace ns3

#endif

#ifndef RNG_RSP_H
#define RNG_RSP_H

#include "cid.h"
#include "service-flow.h"

#include "ns3/header.h"
#include "ns3/mac48-address.h"

#include <stdint.h>

namespace ns3 {

class RngRsp : public Header {
public:
  RngRsp();
  ~RngRsp() override;
  void SetTimingAdjust(uint32_t timingAdjust);
  void SetPowerLevelAdjust(uint8_t powerLevelAdjust);
  void SetOffsetFreqAdjust(uint32_t offsetFreqAdjust);
  void SetRangStatus(uint8_t rangStatus);
  void SetDlFreqOverride(uint32_t dlFreqOverride);
  void SetUlChnlIdOverride(uint8_t ulChnlIdOverride);
  void SetDlOperBurstProfile(uint16_t dlOperBurstProfile);
  void SetMacAddress(Mac48Address macAddress);

  void SetBasicCid(Cid basicCid);
  void SetPrimaryCid(Cid primaryCid);

  void SetAasBdcastPermission(uint8_t aasBdcastPermission);
  void SetFrameNumber(uint32_t frameNumber);
  void SetInitRangOppNumber(uint8_t initRangOppNumber);
  void SetRangSubchnl(uint8_t rangSubchnl);
  uint32_t GetTimingAdjust() const;
  uint8_t GetPowerLevelAdjust() const;
  uint32_t GetOffsetFreqAdjust() const;
  uint8_t GetRangStatus() const;
  uint32_t GetDlFreqOverride() const;
  uint8_t GetUlChnlIdOverride() const;
  uint16_t GetDlOperBurstProfile() const;
  Mac48Address GetMacAddress() const;
  Cid GetBasicCid() const;
  Cid GetPrimaryCid() const;
  uint8_t GetAasBdcastPermission() const;
  uint32_t GetFrameNumber() const;
  uint8_t GetInitRangOppNumber() const;
  uint8_t GetRangSubchnl() const;

  std::string GetName() const;
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint8_t m_reserved;

  uint32_t m_timingAdjust;

  uint8_t m_powerLevelAdjust;

  uint32_t m_offsetFreqAdjust;

  uint8_t m_rangStatus;

  uint32_t m_dlFreqOverride;

  uint8_t m_ulChnlIdOverride;

  uint16_t m_dlOperBurstProfile;

  Mac48Address m_macAddress;
  Cid m_basicCid;
  Cid m_primaryCid;
  uint8_t m_aasBdcastPermission;

  uint32_t m_frameNumber;

  uint8_t m_initRangOppNumber;

  uint8_t m_rangSubchnl;
};

} // namespace ns3

#endif

#ifndef DSA_REQ_H
#define DSA_REQ_H

#include "cid.h"
#include "service-flow.h"

#include "ns3/buffer.h"
#include "ns3/header.h"

#include <stdint.h>

namespace ns3 {
class DsaReq : public Header {
public:
  DsaReq();
  ~DsaReq() override;
  DsaReq(ServiceFlow sf);
  void SetTransactionId(uint16_t transactionId);
  void SetSfid(uint32_t sfid);
  void SetCid(Cid cid);
  void SetServiceFlow(ServiceFlow sf);
  ServiceFlow GetServiceFlow() const;
  uint16_t GetTransactionId() const;
  uint32_t GetSfid() const;
  Cid GetCid() const;
  std::string GetName() const;
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint16_t m_transactionId;
  uint32_t m_sfid;
  Cid m_cid;
  ServiceFlow m_serviceFlow;
};

} // namespace ns3

#endif

#ifndef DSA_RSP_H
#define DSA_RSP_H

#include "cid.h"

#include "ns3/buffer.h"
#include "ns3/header.h"

#include <stdint.h>

namespace ns3 {

class DsaRsp : public Header {
public:
  DsaRsp();
  ~DsaRsp() override;

  void SetTransactionId(uint16_t transactionId);
  uint16_t GetTransactionId() const;

  void SetConfirmationCode(uint16_t confirmationCode);
  uint16_t GetConfirmationCode() const;
  void SetSfid(uint32_t sfid);
  uint32_t GetSfid() const;
  void SetCid(Cid cid);
  Cid GetCid() const;
  void SetServiceFlow(ServiceFlow sf);
  ServiceFlow GetServiceFlow() const;

  std::string GetName() const;
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint16_t m_transactionId;
  uint8_t m_confirmationCode;
  ServiceFlow m_serviceFlow;
  uint32_t m_sfid;
  Cid m_cid;
};

} // namespace ns3

#endif

#ifndef DSA_ACK_H
#define DSA_ACK_H

#include "ns3/buffer.h"
#include "ns3/header.h"

#include <stdint.h>

namespace ns3 {

class DsaAck : public Header {
public:
  DsaAck();
  ~DsaAck() override;

  void SetTransactionId(uint16_t transactionId);
  uint16_t GetTransactionId() const;

  void SetConfirmationCode(uint16_t confirmationCode);
  uint16_t GetConfirmationCode() const;

  std::string GetName() const;
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint16_t m_transactionId;
  uint8_t m_confirmationCode;
};

} // namespace ns3

#endif

#ifndef RNG_REQ_H
#define RNG_REQ_H

#include "service-flow.h"

#include "ns3/header.h"
#include "ns3/mac48-address.h"

#include <stdint.h>

namespace ns3 {

class RngReq : public Header {
public:
  RngReq();
  ~RngReq() override;

  void SetReqDlBurstProfile(uint8_t reqDlBurstProfile);
  void SetMacAddress(Mac48Address macAddress);
  void SetRangingAnomalies(uint8_t rangingAnomalies);

  uint8_t GetReqDlBurstProfile() const;
  Mac48Address GetMacAddress() const;
  uint8_t GetRangingAnomalies() const;

  std::string GetName() const;
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  void PrintDebug() const;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint8_t m_reserved;

  uint8_t m_reqDlBurstProfile;
  Mac48Address m_macAddress;
  uint8_t m_rangingAnomalies;
};

} // namespace ns3

#endif
