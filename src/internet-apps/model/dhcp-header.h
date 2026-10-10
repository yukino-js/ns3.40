
#ifndef DHCP_HEADER_H
#define DHCP_HEADER_H

#include "ns3/address.h"
#include "ns3/buffer.h"
#include "ns3/header.h"
#include "ns3/ipv4-address.h"

namespace ns3 {

class DhcpHeader : public Header {
public:
  static TypeId GetTypeId();

  DhcpHeader();

  ~DhcpHeader() override;

  enum Options {
    OP_MASK = 1,
    OP_ROUTE = 3,
    OP_ADDREQ = 50,
    OP_LEASE = 51,
    OP_MSGTYPE = 53,
    OP_SERVID = 54,
    OP_RENEW = 58,
    OP_REBIND = 59,
    OP_END = 255
  };

  enum Messages {
    DHCPDISCOVER = 0,
    DHCPOFFER = 1,
    DHCPREQ = 2,
    DHCPACK = 4,
    DHCPNACK = 5
  };

  void SetType(uint8_t type);

  uint8_t GetType() const;

  void SetHWType(uint8_t htype, uint8_t hlen);

  void SetTran(uint32_t tran);

  uint32_t GetTran() const;

  void SetTime();

  void SetChaddr(Address addr);

  void SetChaddr(uint8_t *addr, uint8_t len);

  Address GetChaddr();

  void SetYiaddr(Ipv4Address addr);

  Ipv4Address GetYiaddr() const;

  void SetDhcps(Ipv4Address addr);

  Ipv4Address GetDhcps() const;

  void SetReq(Ipv4Address addr);

  Ipv4Address GetReq() const;

  void SetMask(uint32_t addr);

  uint32_t GetMask() const;

  void SetRouter(Ipv4Address addr);

  Ipv4Address GetRouter() const;

  void SetLease(uint32_t time);

  uint32_t GetLease() const;

  void SetRenew(uint32_t time);

  uint32_t GetRenew() const;

  void SetRebind(uint32_t time);

  uint32_t GetRebind() const;

  void ResetOpt();

private:
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream &os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

  uint8_t m_op;
  uint8_t m_bootp;
  uint8_t m_hType;
  uint8_t m_hLen;
  uint8_t m_hops;
  uint32_t m_xid;
  uint32_t m_mask;
  uint32_t m_len;
  uint16_t m_secs;
  uint16_t m_flags;
  uint8_t m_chaddr[16];
  Ipv4Address m_yiAddr;
  Ipv4Address m_ciAddr;
  Ipv4Address m_siAddr;
  Ipv4Address m_giAddr;
  Ipv4Address m_dhcps;
  Ipv4Address m_req;
  Ipv4Address m_route;
  uint8_t m_sname[64];
  uint8_t m_file[128];
  uint8_t m_magic_cookie[4];
  uint32_t m_lease;
  uint32_t m_renew;
  uint32_t m_rebind;
  bool m_opt[255];
};

} // namespace ns3

#endif
