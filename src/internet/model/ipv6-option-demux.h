
#ifndef IPV6_OPTION_DEMUX_H
#define IPV6_OPTION_DEMUX_H

#include "ns3/object.h"
#include "ns3/ptr.h"

#include <list>

namespace ns3 {

class Ipv6Option;
class Node;

class Ipv6OptionDemux : public Object {
public:
  static TypeId GetTypeId();

  Ipv6OptionDemux();

  ~Ipv6OptionDemux() override;

  void SetNode(Ptr<Node> node);

  void Insert(Ptr<Ipv6Option> option);

  Ptr<Ipv6Option> GetOption(int optionNumber);

  void Remove(Ptr<Ipv6Option> option);

protected:
  void DoDispose() override;

private:
  typedef std::list<Ptr<Ipv6Option>> Ipv6OptionList_t;

  Ipv6OptionList_t m_options;

  Ptr<Node> m_node;
};

} // namespace ns3

#endif
