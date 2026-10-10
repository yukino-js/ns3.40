
#ifndef RADVD_PREFIX_H
#define RADVD_PREFIX_H

#include "ns3/ipv6-address.h"
#include "ns3/simple-ref-count.h"

#include <stdint.h>

namespace ns3 {

class RadvdPrefix : public SimpleRefCount<RadvdPrefix> {
public:
  RadvdPrefix(Ipv6Address network, uint8_t prefixLength,
              uint32_t preferredLifeTime = 604800,
              uint32_t validLifeTime = 2592000, bool onLinkFlag = true,
              bool autonomousFlag = true, bool routerAddrFlag = false);

  ~RadvdPrefix();

  Ipv6Address GetNetwork() const;

  void SetNetwork(Ipv6Address network);

  uint8_t GetPrefixLength() const;

  void SetPrefixLength(uint8_t prefixLength);

  uint32_t GetPreferredLifeTime() const;

  void SetPreferredLifeTime(uint32_t preferredLifeTime);

  uint32_t GetValidLifeTime() const;

  void SetValidLifeTime(uint32_t validLifeTime);

  bool IsOnLinkFlag() const;

  void SetOnLinkFlag(bool onLinkFlag);

  bool IsAutonomousFlag() const;

  void SetAutonomousFlag(bool autonomousFlag);

  bool IsRouterAddrFlag() const;

  void SetRouterAddrFlag(bool routerAddrFlag);

private:
  Ipv6Address m_network;

  uint8_t m_prefixLength;

  uint32_t m_preferredLifeTime;

  uint32_t m_validLifeTime;

  bool m_onLinkFlag;

  bool m_autonomousFlag;

  bool m_routerAddrFlag;
};

} // namespace ns3

#endif
