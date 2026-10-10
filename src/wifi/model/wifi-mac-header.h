
#ifndef WIFI_MAC_HEADER_H
#define WIFI_MAC_HEADER_H

#include "ns3/header.h"
#include "ns3/mac48-address.h"

namespace ns3 {

class Time;

enum WifiMacType {
  WIFI_MAC_CTL_TRIGGER = 0,
  WIFI_MAC_CTL_CTLWRAPPER,
  WIFI_MAC_CTL_PSPOLL,
  WIFI_MAC_CTL_RTS,
  WIFI_MAC_CTL_CTS,
  WIFI_MAC_CTL_ACK,
  WIFI_MAC_CTL_BACKREQ,
  WIFI_MAC_CTL_BACKRESP,
  WIFI_MAC_CTL_END,
  WIFI_MAC_CTL_END_ACK,

  WIFI_MAC_CTL_DMG_POLL,
  WIFI_MAC_CTL_DMG_SPR,
  WIFI_MAC_CTL_DMG_GRANT,
  WIFI_MAC_CTL_DMG_CTS,
  WIFI_MAC_CTL_DMG_DTS,
  WIFI_MAC_CTL_DMG_SSW,
  WIFI_MAC_CTL_DMG_SSW_FBCK,
  WIFI_MAC_CTL_DMG_SSW_ACK,
  WIFI_MAC_CTL_DMG_GRANT_ACK,

  WIFI_MAC_MGT_BEACON,
  WIFI_MAC_MGT_ASSOCIATION_REQUEST,
  WIFI_MAC_MGT_ASSOCIATION_RESPONSE,
  WIFI_MAC_MGT_DISASSOCIATION,
  WIFI_MAC_MGT_REASSOCIATION_REQUEST,
  WIFI_MAC_MGT_REASSOCIATION_RESPONSE,
  WIFI_MAC_MGT_PROBE_REQUEST,
  WIFI_MAC_MGT_PROBE_RESPONSE,
  WIFI_MAC_MGT_AUTHENTICATION,
  WIFI_MAC_MGT_DEAUTHENTICATION,
  WIFI_MAC_MGT_ACTION,
  WIFI_MAC_MGT_ACTION_NO_ACK,
  WIFI_MAC_MGT_MULTIHOP_ACTION,

  WIFI_MAC_DATA,
  WIFI_MAC_DATA_CFACK,
  WIFI_MAC_DATA_CFPOLL,
  WIFI_MAC_DATA_CFACK_CFPOLL,
  WIFI_MAC_DATA_NULL,
  WIFI_MAC_DATA_NULL_CFACK,
  WIFI_MAC_DATA_NULL_CFPOLL,
  WIFI_MAC_DATA_NULL_CFACK_CFPOLL,
  WIFI_MAC_QOSDATA,
  WIFI_MAC_QOSDATA_CFACK,
  WIFI_MAC_QOSDATA_CFPOLL,
  WIFI_MAC_QOSDATA_CFACK_CFPOLL,
  WIFI_MAC_QOSDATA_NULL,
  WIFI_MAC_QOSDATA_NULL_CFPOLL,
  WIFI_MAC_QOSDATA_NULL_CFACK_CFPOLL,

  WIFI_MAC_EXTENSION_DMG_BEACON,
};

class WifiMacHeader : public Header {
public:
  enum QosAckPolicy {
    NORMAL_ACK = 0,
    NO_ACK = 1,
    NO_EXPLICIT_ACK = 2,
    BLOCK_ACK = 3,
  };

  enum AddressType { ADDR1, ADDR2, ADDR3, ADDR4 };

  WifiMacHeader();
  WifiMacHeader(WifiMacType type);
  ~WifiMacHeader() override;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

  void SetDsFrom();
  void SetDsNotFrom();
  void SetDsTo();
  void SetDsNotTo();
  void SetAddr1(Mac48Address address);
  void SetAddr2(Mac48Address address);
  void SetAddr3(Mac48Address address);
  void SetAddr4(Mac48Address address);
  virtual void SetType(WifiMacType type, bool resetToDsFromDs = true);
  void SetRawDuration(uint16_t duration);
  void SetDuration(Time duration);
  void SetId(uint16_t id);
  void SetSequenceNumber(uint16_t seq);
  void SetFragmentNumber(uint8_t frag);
  void SetNoMoreFragments();
  void SetMoreFragments();
  void SetRetry();
  void SetNoRetry();
  void SetPowerManagement();
  void SetNoPowerManagement();
  void SetQosTid(uint8_t tid);
  void SetQosEosp();
  void SetQosNoEosp();
  void SetQosAckPolicy(QosAckPolicy policy);
  void SetQosAmsdu();
  void SetQosNoAmsdu();
  void SetQosTxopLimit(uint8_t txop);
  void SetQosQueueSize(uint8_t size);
  void SetQosMeshControlPresent();
  void SetQosNoMeshControlPresent();
  void SetOrder();
  void SetNoOrder();

  Mac48Address GetAddr1() const;
  Mac48Address GetAddr2() const;
  Mac48Address GetAddr3() const;
  Mac48Address GetAddr4() const;
  virtual WifiMacType GetType() const;
  bool IsFromDs() const;
  bool IsToDs() const;
  bool IsData() const;
  bool IsQosData() const;
  bool HasData() const;
  bool IsCtl() const;
  bool IsMgt() const;
  bool IsCfPoll() const;
  bool IsCfAck() const;
  bool IsCfEnd() const;
  bool IsPsPoll() const;
  bool IsRts() const;
  bool IsCts() const;
  bool IsAck() const;
  bool IsBlockAckReq() const;
  bool IsBlockAck() const;
  bool IsTrigger() const;
  bool IsAssocReq() const;
  bool IsAssocResp() const;
  bool IsReassocReq() const;
  bool IsReassocResp() const;
  bool IsProbeReq() const;
  bool IsProbeResp() const;
  bool IsBeacon() const;
  bool IsDisassociation() const;
  bool IsAuthentication() const;
  bool IsDeauthentication() const;
  bool IsAction() const;
  bool IsActionNoAck() const;
  bool IsMultihopAction() const;
  uint16_t GetRawDuration() const;
  Time GetDuration() const;
  uint16_t GetSequenceControl() const;
  uint16_t GetSequenceNumber() const;
  uint8_t GetFragmentNumber() const;
  bool IsRetry() const;
  bool IsPowerManagement() const;
  bool IsMoreData() const;
  bool IsMoreFragments() const;
  bool IsQosBlockAck() const;
  bool IsQosNoAck() const;
  bool IsQosAck() const;
  bool IsQosEosp() const;
  bool IsQosAmsdu() const;
  uint8_t GetQosTid() const;
  QosAckPolicy GetQosAckPolicy() const;
  uint8_t GetQosQueueSize() const;
  virtual uint32_t GetSize() const;
  virtual const char *GetTypeString() const;

  typedef void (*TracedCallback)(const WifiMacHeader &header);

protected:
  virtual uint16_t GetFrameControl() const;
  virtual uint16_t GetQosControl() const;
  virtual void SetFrameControl(uint16_t control);
  void SetSequenceControl(uint16_t seq);
  virtual void SetQosControl(uint16_t qos);
  void PrintFrameControl(std::ostream &os) const;

  uint8_t m_ctrlType;
  uint8_t m_ctrlSubtype;
  uint8_t m_ctrlToDs;
  uint8_t m_ctrlFromDs;
  uint8_t m_ctrlMoreFrag;
  uint8_t m_ctrlRetry;
  uint8_t m_ctrlPowerManagement;
  uint8_t m_ctrlMoreData;
  uint8_t m_ctrlWep;
  uint8_t m_ctrlOrder;
  uint16_t m_duration;
  Mac48Address m_addr1;
  Mac48Address m_addr2;
  Mac48Address m_addr3;
  uint8_t m_seqFrag;
  uint16_t m_seqSeq;
  Mac48Address m_addr4;
  uint8_t m_qosTid;
  uint8_t m_qosEosp;
  uint8_t m_qosAckPolicy;
  uint8_t m_amsduPresent;
  uint8_t m_qosStuff;
};

} // namespace ns3

#endif
