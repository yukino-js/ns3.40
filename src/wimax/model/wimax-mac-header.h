
#ifndef MAC_HEADER_TYPE_H
#define MAC_HEADER_TYPE_H

#include "ns3/header.h"

#include <stdint.h>

namespace ns3 {

class MacHeaderType : public Header {
public:
  enum HeaderType { HEADER_TYPE_GENERIC, HEADER_TYPE_BANDWIDTH };

  MacHeaderType();
  MacHeaderType(uint8_t type);
  ~MacHeaderType() override;
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

#ifndef GENERIC_MAC_HEADER_H
#define GENERIC_MAC_HEADER_H

#include "cid.h"

#include "ns3/header.h"

#include <stdint.h>

namespace ns3 {

class GenericMacHeader : public Header {
public:
  GenericMacHeader();
  ~GenericMacHeader() override;

  void SetEc(uint8_t ec);
  void SetType(uint8_t type);
  void SetCi(uint8_t ci);
  void SetEks(uint8_t eks);
  void SetLen(uint16_t len);
  void SetCid(Cid cid);
  void SetHcs(uint8_t hcs);
  void SetHt(uint8_t ht);

  uint8_t GetEc() const;
  uint8_t GetType() const;
  uint8_t GetCi() const;
  uint8_t GetEks() const;
  uint16_t GetLen() const;
  Cid GetCid() const;
  uint8_t GetHcs() const;
  uint8_t GetHt() const;
  std::string GetName() const;
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  bool check_hcs() const;

private:
  uint8_t m_ht;
  uint8_t m_ec;
  uint8_t m_type;
  uint8_t m_esf;
  uint8_t m_ci;
  uint8_t m_eks;
  uint8_t m_rsv1;
  uint16_t m_len;
  Cid m_cid;
  uint8_t m_hcs;
  uint8_t c_hcs;
};

} // namespace ns3

#endif

#ifndef BANDWIDTH_REQUEST_HEADER_H
#define BANDWIDTH_REQUEST_HEADER_H

#include "cid.h"

#include "ns3/header.h"

#include <stdint.h>

namespace ns3 {
class BandwidthRequestHeader : public Header {
public:
  enum HeaderType { HEADER_TYPE_INCREMENTAL, HEADER_TYPE_AGGREGATE };

  BandwidthRequestHeader();
  ~BandwidthRequestHeader() override;

  void SetHt(uint8_t ht);
  void SetEc(uint8_t ec);
  void SetType(uint8_t type);
  void SetBr(uint32_t br);
  void SetCid(Cid cid);
  void SetHcs(uint8_t hcs);

  uint8_t GetHt() const;
  uint8_t GetEc() const;
  uint8_t GetType() const;
  uint32_t GetBr() const;
  Cid GetCid() const;
  uint8_t GetHcs() const;

  std::string GetName() const;
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  bool check_hcs() const;

private:
  uint8_t m_ht;
  uint8_t m_ec;
  uint8_t m_type;
  uint32_t m_br;
  Cid m_cid;
  uint8_t m_hcs;
  uint8_t c_hcs;
};

} // namespace ns3

#endif

#ifndef GRANT_MANAGEMENT_SUBHEADER_H
#define GRANT_MANAGEMENT_SUBHEADER_H

#include "ns3/header.h"

#include <stdint.h>

namespace ns3 {

class GrantManagementSubheader : public Header {
public:
  GrantManagementSubheader();
  ~GrantManagementSubheader() override;

  void SetSi(uint8_t si);
  void SetPm(uint8_t pm);
  void SetPbr(uint16_t pbr);

  uint8_t GetSi() const;
  uint8_t GetPm() const;
  uint16_t GetPbr() const;

  std::string GetName() const;
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint8_t m_si;
  uint8_t m_pm;
  uint16_t m_pbr;
};

} // namespace ns3

#endif

#ifndef FRAGMENTATION_SUBHEADER_H
#define FRAGMENTATION_SUBHEADER_H

#include "ns3/header.h"

#include <stdint.h>

namespace ns3 {
class FragmentationSubheader : public Header {
public:
  FragmentationSubheader();
  ~FragmentationSubheader() override;

  void SetFc(uint8_t fc);
  void SetFsn(uint8_t fsn);

  uint8_t GetFc() const;
  uint8_t GetFsn() const;

  std::string GetName() const;
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint8_t m_fc;
  uint8_t m_fsn;
};
} // namespace ns3

#endif
