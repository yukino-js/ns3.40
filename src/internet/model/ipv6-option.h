
#ifndef IPV6_OPTION_H
#define IPV6_OPTION_H

#include "ipv6-header.h"
#include "ipv6-interface.h"

#include "ns3/buffer.h"
#include "ns3/ipv6-address.h"
#include "ns3/node.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"

#include <map>

namespace ns3 {

class Ipv6Option : public Object {
public:
  static TypeId GetTypeId();

  ~Ipv6Option() override;

  void SetNode(Ptr<Node> node);

  virtual uint8_t GetOptionNumber() const = 0;

  virtual uint8_t Process(Ptr<Packet> packet, uint8_t offset,
                          const Ipv6Header &ipv6Header, bool &isDropped) = 0;

private:
  Ptr<Node> m_node;
};

class Ipv6OptionPad1 : public Ipv6Option {
public:
  static const uint8_t OPT_NUMBER = 0;

  static TypeId GetTypeId();

  Ipv6OptionPad1();

  ~Ipv6OptionPad1() override;

  uint8_t GetOptionNumber() const override;

  uint8_t Process(Ptr<Packet> packet, uint8_t offset,
                  const Ipv6Header &ipv6Header, bool &isDropped) override;
};

class Ipv6OptionPadn : public Ipv6Option {
public:
  static const uint8_t OPT_NUMBER = 60;

  static TypeId GetTypeId();

  Ipv6OptionPadn();

  ~Ipv6OptionPadn() override;

  uint8_t GetOptionNumber() const override;

  uint8_t Process(Ptr<Packet> packet, uint8_t offset,
                  const Ipv6Header &ipv6Header, bool &isDropped) override;
};

class Ipv6OptionJumbogram : public Ipv6Option {
public:
  static const uint8_t OPT_NUMBER = 44;

  static TypeId GetTypeId();

  Ipv6OptionJumbogram();

  ~Ipv6OptionJumbogram() override;

  uint8_t GetOptionNumber() const override;

  uint8_t Process(Ptr<Packet> packet, uint8_t offset,
                  const Ipv6Header &ipv6Header, bool &isDropped) override;
};

class Ipv6OptionRouterAlert : public Ipv6Option {
public:
  static const uint8_t OPT_NUMBER = 43;

  static TypeId GetTypeId();

  Ipv6OptionRouterAlert();

  ~Ipv6OptionRouterAlert() override;

  uint8_t GetOptionNumber() const override;

  uint8_t Process(Ptr<Packet> packet, uint8_t offset,
                  const Ipv6Header &ipv6Header, bool &isDropped) override;
};

} // namespace ns3

#endif
