
#ifndef SIXLOWPANHEADER_H_
#define SIXLOWPANHEADER_H_

#include "ns3/header.h"
#include "ns3/ipv6-address.h"

namespace ns3 {

class SixLowPanDispatch {
public:
  enum Dispatch_e {
    LOWPAN_NALP = 0x0,
    LOWPAN_NALP_N = 0x3F,
    LOWPAN_IPv6 = 0x41,
    LOWPAN_HC1 = 0x42,
    LOWPAN_BC0 = 0x50,
    LOWPAN_IPHC = 0x60,
    LOWPAN_IPHC_N = 0x7F,
    LOWPAN_MESH = 0x80,
    LOWPAN_MESH_N = 0xBF,
    LOWPAN_FRAG1 = 0xC0,
    LOWPAN_FRAG1_N = 0xC7,
    LOWPAN_FRAGN = 0xE0,
    LOWPAN_FRAGN_N = 0xE7,
    LOWPAN_UNSUPPORTED = 0xFF
  };

  enum NhcDispatch_e {
    LOWPAN_NHC = 0xE0,
    LOWPAN_NHC_N = 0xEF,
    LOWPAN_UDPNHC = 0xF0,
    LOWPAN_UDPNHC_N = 0xF7,
    LOWPAN_NHCUNSUPPORTED = 0xFF
  };

  SixLowPanDispatch();

  static Dispatch_e GetDispatchType(uint8_t dispatch);

  static NhcDispatch_e GetNhcDispatchType(uint8_t dispatch);
};

class SixLowPanHc1 : public Header {
public:
  enum LowPanHc1Addr_e {
    HC1_PIII = 0x00,
    HC1_PIIC = 0x01,
    HC1_PCII = 0x02,
    HC1_PCIC = 0x03
  };

  enum LowPanHc1NextHeader_e {
    HC1_NC = 0x00,
    HC1_UDP = 0x01,
    HC1_ICMP = 0x02,
    HC1_TCP = 0x03
  };

  SixLowPanHc1();

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

  void SetHopLimit(uint8_t limit);

  uint8_t GetHopLimit() const;

  LowPanHc1Addr_e GetDstCompression() const;

  const uint8_t *GetDstInterface() const;

  const uint8_t *GetDstPrefix() const;

  uint32_t GetFlowLabel() const;

  uint8_t GetNextHeader() const;

  LowPanHc1Addr_e GetSrcCompression() const;

  const uint8_t *GetSrcInterface() const;

  const uint8_t *GetSrcPrefix() const;

  uint8_t GetTrafficClass() const;

  bool IsTcflCompression() const;

  bool IsHc2HeaderPresent() const;

  void SetDstCompression(LowPanHc1Addr_e dstCompression);

  void SetDstInterface(const uint8_t *dstInterface);

  void SetDstPrefix(const uint8_t *dstPrefix);

  void SetFlowLabel(uint32_t flowLabel);

  void SetNextHeader(uint8_t nextHeader);

  void SetSrcCompression(LowPanHc1Addr_e srcCompression);

  void SetSrcInterface(const uint8_t *srcInterface);

  void SetSrcPrefix(const uint8_t *srcPrefix);

  void SetTcflCompression(bool tcflCompression);

  void SetHc2HeaderPresent(bool hc2HeaderPresent);

  void SetTrafficClass(uint8_t trafficClass);

private:
  uint8_t m_hopLimit;
  uint8_t m_srcPrefix[8];
  uint8_t m_srcInterface[8];
  uint8_t m_dstPrefix[8];
  uint8_t m_dstInterface[8];
  uint8_t m_trafficClass;
  uint32_t m_flowLabel;
  uint8_t m_nextHeader;
  LowPanHc1Addr_e m_srcCompression;
  LowPanHc1Addr_e m_dstCompression;
  bool m_tcflCompression;
  LowPanHc1NextHeader_e m_nextHeaderCompression;
  bool m_hc2HeaderPresent;
};

std::ostream &operator<<(std::ostream &os, const SixLowPanHc1 &header);

class SixLowPanFrag1 : public Header {
public:
  SixLowPanFrag1();

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

  void SetDatagramSize(uint16_t datagramSize);

  uint16_t GetDatagramSize() const;

  void SetDatagramTag(uint16_t datagramTag);

  uint16_t GetDatagramTag() const;

private:
  uint16_t m_datagramSize;
  uint16_t m_datagramTag;
};

std::ostream &operator<<(std::ostream &os, const SixLowPanFrag1 &header);

class SixLowPanFragN : public Header {
public:
  SixLowPanFragN();

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

  void SetDatagramSize(uint16_t datagramSize);

  uint16_t GetDatagramSize() const;

  void SetDatagramTag(uint16_t datagramTag);

  uint16_t GetDatagramTag() const;

  void SetDatagramOffset(uint8_t datagramOffset);

  uint8_t GetDatagramOffset() const;

private:
  uint16_t m_datagramSize;
  uint16_t m_datagramTag;
  uint8_t m_datagramOffset;
};

std::ostream &operator<<(std::ostream &os, const SixLowPanFragN &header);

class SixLowPanIpv6 : public Header {
public:
  SixLowPanIpv6();

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;
};

std::ostream &operator<<(std::ostream &os, const SixLowPanIpv6 &header);

class SixLowPanIphc : public Header {
public:
  enum TrafficClassFlowLabel_e {
    TF_FULL = 0,
    TF_DSCP_ELIDED,
    TF_FL_ELIDED,
    TF_ELIDED
  };

  enum Hlim_e { HLIM_INLINE = 0, HLIM_COMPR_1, HLIM_COMPR_64, HLIM_COMPR_255 };

  enum HeaderCompression_e {
    HC_INLINE = 0,
    HC_COMPR_64,
    HC_COMPR_16,
    HC_COMPR_0
  };

  SixLowPanIphc();
  SixLowPanIphc(uint8_t dispatch);

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

  void SetTf(TrafficClassFlowLabel_e tfField);

  TrafficClassFlowLabel_e GetTf() const;

  void SetNh(bool nhField);

  bool GetNh() const;

  void SetHlim(Hlim_e hlimField);

  Hlim_e GetHlim() const;

  void SetCid(bool cidField);

  bool GetCid() const;

  void SetSac(bool sacField);

  bool GetSac() const;

  void SetSam(HeaderCompression_e samField);

  HeaderCompression_e GetSam() const;

  void SetSrcInlinePart(uint8_t srcInlinePart[16], uint8_t size);

  const uint8_t *GetSrcInlinePart() const;

  void SetM(bool mField);

  bool GetM() const;

  void SetDac(bool dacField);

  bool GetDac() const;

  void SetDam(HeaderCompression_e damField);

  HeaderCompression_e GetDam() const;

  void SetDstInlinePart(uint8_t dstInlinePart[16], uint8_t size);

  const uint8_t *GetDstInlinePart() const;

  void SetSrcContextId(uint8_t srcContextId);

  uint8_t GetSrcContextId() const;

  void SetDstContextId(uint8_t dstContextId);

  uint8_t GetDstContextId() const;

  void SetEcn(uint8_t ecn);

  uint8_t GetEcn() const;

  void SetDscp(uint8_t dscp);

  uint8_t GetDscp() const;

  void SetFlowLabel(uint32_t flowLabel);

  uint32_t GetFlowLabel() const;

  void SetNextHeader(uint8_t nextHeader);

  uint8_t GetNextHeader() const;

  void SetHopLimit(uint8_t hopLimit);

  uint8_t GetHopLimit() const;

private:
  uint16_t m_baseFormat;
  uint8_t m_srcdstContextId;
  uint8_t m_ecn : 2;
  uint8_t m_dscp : 6;
  uint32_t m_flowLabel : 20;
  uint8_t m_nextHeader;
  uint8_t m_hopLimit;
  uint8_t m_srcInlinePart[16];
  uint8_t m_dstInlinePart[16];
};

std::ostream &operator<<(std::ostream &os, const SixLowPanIphc &header);

class SixLowPanNhcExtension : public Header {
public:
  enum Eid_e {
    EID_HOPBYHOP_OPTIONS_H = 0,
    EID_ROUTING_H,
    EID_FRAGMENTATION_H,
    EID_DESTINATION_OPTIONS_H,
    EID_MOBILITY_H,
    EID_IPv6_H = 7
  };

  SixLowPanNhcExtension();

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

  virtual SixLowPanDispatch::NhcDispatch_e GetNhcDispatchType() const;

  void SetEid(Eid_e extensionHeaderType);

  Eid_e GetEid() const;

  void SetNextHeader(uint8_t nextHeader);

  uint8_t GetNextHeader() const;

  void SetNh(bool nhField);

  bool GetNh() const;

  void SetBlob(const uint8_t *blob, uint32_t size);

  uint32_t CopyBlob(uint8_t *blob, uint32_t size) const;

private:
  uint8_t m_nhcExtensionHeader;
  uint8_t m_nhcNextHeader;
  uint8_t m_nhcBlobLength;
  uint8_t m_nhcBlob[256];
};

std::ostream &operator<<(std::ostream &os, const SixLowPanNhcExtension &header);

class SixLowPanUdpNhcExtension : public Header {
public:
  enum Ports_e {
    PORTS_INLINE = 0,
    PORTS_ALL_SRC_LAST_DST,
    PORTS_LAST_SRC_ALL_DST,
    PORTS_LAST_SRC_LAST_DST
  };

  SixLowPanUdpNhcExtension();

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

  virtual SixLowPanDispatch::NhcDispatch_e GetNhcDispatchType() const;

  void SetPorts(Ports_e port);

  Ports_e GetPorts() const;

  void SetSrcPort(uint16_t port);

  uint16_t GetSrcPort() const;

  void SetDstPort(uint16_t port);

  uint16_t GetDstPort() const;

  void SetC(bool cField);

  bool GetC() const;

  void SetChecksum(uint16_t checksum);

  uint16_t GetChecksum() const;

private:
  uint8_t m_baseFormat;
  uint16_t m_checksum;
  uint16_t m_srcPort;
  uint16_t m_dstPort;
};

std::ostream &operator<<(std::ostream &os,
                         const SixLowPanUdpNhcExtension &header);

class SixLowPanBc0 : public Header {
public:
  SixLowPanBc0();

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

  void SetSequenceNumber(uint8_t seqNumber);

  uint8_t GetSequenceNumber() const;

private:
  uint8_t m_seqNumber;
};

std::ostream &operator<<(std::ostream &os, const SixLowPanBc0 &header);

class SixLowPanMesh : public Header {
public:
  SixLowPanMesh();

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  void Print(std::ostream &os) const override;

  uint32_t GetSerializedSize() const override;

  void Serialize(Buffer::Iterator start) const override;

  uint32_t Deserialize(Buffer::Iterator start) override;

  void SetHopsLeft(uint8_t hopsLeft);

  uint8_t GetHopsLeft() const;

  void SetOriginator(Address originator);

  Address GetOriginator() const;

  void SetFinalDst(Address finalDst);

  Address GetFinalDst() const;

private:
  uint8_t m_hopsLeft;
  bool m_v;
  bool m_f;
  Address m_src;
  Address m_dst;
};

std::ostream &operator<<(std::ostream &os, const SixLowPanMesh &header);

} // namespace ns3

#endif
