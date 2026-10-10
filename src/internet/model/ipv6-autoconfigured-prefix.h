
#ifndef IPV6_AUTOCONFIGURED_PREFIX_H
#define IPV6_AUTOCONFIGURED_PREFIX_H

#include "ns3/ipv6-address.h"
#include "ns3/timer.h"

#include <list>
#include <ostream>
#include <stdint.h>
#include <vector>

namespace ns3 {
class Node;

class Ipv6AutoconfiguredPrefix : public Object {
public:
  Ipv6AutoconfiguredPrefix(Ptr<Node> node, uint32_t interface,
                           Ipv6Address prefix, Ipv6Prefix mask,
                           uint32_t preferredLifeTime, uint32_t validLifeTime,
                           Ipv6Address router = Ipv6Address("::"));

  ~Ipv6AutoconfiguredPrefix() override;

  void SetDefaultGatewayRouter(Ipv6Address router);

  Ipv6Address GetDefaultGatewayRouter() const;

  uint32_t GetInterface() const;

  void SetInterface(uint32_t interface);

  uint32_t GetPreferredLifeTime() const;

  void SetPreferredLifeTime(uint32_t p);

  uint32_t GetValidLifeTime() const;

  void SetValidLifeTime(uint32_t v);

  bool IsPreferred() const;

  bool IsValid() const;

  void SetPreferred();

  void SetValid();

  void StartPreferredTimer();

  void StartValidTimer();

  void StopPreferredTimer();

  void StopValidTimer();

  void MarkPreferredTime();

  void MarkValidTime();

  void FunctionPreferredTimeout();

  void FunctionValidTimeout();

  void RemoveMe();

  uint32_t GetId() const;

  Ipv6Address GetPrefix() const;

  void SetPrefix(Ipv6Address prefix);

  Ipv6Prefix GetMask() const;

  void SetMask(Ipv6Prefix mask);

private:
  static uint32_t m_prefixId;

  uint32_t m_id;

  Ptr<Node> m_node;

  Ipv6Address m_prefix;

  Ipv6Prefix m_mask;

  Ipv6Address m_defaultGatewayRouter;

  uint32_t m_interface;

  uint32_t m_validLifeTime;

  uint32_t m_preferredLifeTime;

  bool m_preferred;

  bool m_valid;

  Timer m_preferredTimer;

  Timer m_validTimer;
};

} // namespace ns3

#endif
