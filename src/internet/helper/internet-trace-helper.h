
#ifndef INTERNET_TRACE_HELPER_H
#define INTERNET_TRACE_HELPER_H

#include "ipv4-interface-container.h"
#include "ipv6-interface-container.h"

#include "ns3/assert.h"
#include "ns3/ipv4.h"
#include "ns3/ipv6.h"
#include "ns3/trace-helper.h"

namespace ns3 {

class PcapHelperForIpv4 {
public:
  PcapHelperForIpv4() {}

  virtual ~PcapHelperForIpv4() {}

  virtual void EnablePcapIpv4Internal(std::string prefix, Ptr<Ipv4> ipv4,
                                      uint32_t interface,
                                      bool explicitFilename) = 0;

  void EnablePcapIpv4(std::string prefix, Ptr<Ipv4> ipv4, uint32_t interface,
                      bool explicitFilename = false);

  void EnablePcapIpv4(std::string prefix, std::string ipv4Name,
                      uint32_t interface, bool explicitFilename = false);

  void EnablePcapIpv4(std::string prefix, Ipv4InterfaceContainer c);

  void EnablePcapIpv4(std::string prefix, NodeContainer n);

  void EnablePcapIpv4(std::string prefix, uint32_t nodeid, uint32_t interface,
                      bool explicitFilename);

  void EnablePcapIpv4All(std::string prefix);
};

class AsciiTraceHelperForIpv4 {
public:
  AsciiTraceHelperForIpv4() {}

  virtual ~AsciiTraceHelperForIpv4() {}

  virtual void EnableAsciiIpv4Internal(Ptr<OutputStreamWrapper> stream,
                                       std::string prefix, Ptr<Ipv4> ipv4,
                                       uint32_t interface,
                                       bool explicitFilename) = 0;

  void EnableAsciiIpv4(std::string prefix, Ptr<Ipv4> ipv4, uint32_t interface,
                       bool explicitFilename = false);

  void EnableAsciiIpv4(Ptr<OutputStreamWrapper> stream, Ptr<Ipv4> ipv4,
                       uint32_t interface);

  void EnableAsciiIpv4(std::string prefix, std::string ipv4Name,
                       uint32_t interface, bool explicitFilename = false);

  void EnableAsciiIpv4(Ptr<OutputStreamWrapper> stream, std::string ipv4Name,
                       uint32_t interface);

  void EnableAsciiIpv4(std::string prefix, Ipv4InterfaceContainer c);

  void EnableAsciiIpv4(Ptr<OutputStreamWrapper> stream,
                       Ipv4InterfaceContainer c);

  void EnableAsciiIpv4(std::string prefix, NodeContainer n);

  void EnableAsciiIpv4(Ptr<OutputStreamWrapper> stream, NodeContainer n);

  void EnableAsciiIpv4All(std::string prefix);

  void EnableAsciiIpv4All(Ptr<OutputStreamWrapper> stream);

  void EnableAsciiIpv4(std::string prefix, uint32_t nodeid, uint32_t deviceid,
                       bool explicitFilename);

  void EnableAsciiIpv4(Ptr<OutputStreamWrapper> stream, uint32_t nodeid,
                       uint32_t interface, bool explicitFilename);

private:
  void EnableAsciiIpv4Impl(Ptr<OutputStreamWrapper> stream, std::string prefix,
                           uint32_t nodeid, uint32_t interface,
                           bool explicitFilename);

  void EnableAsciiIpv4Impl(Ptr<OutputStreamWrapper> stream, std::string prefix,
                           NodeContainer n);

  void EnableAsciiIpv4Impl(Ptr<OutputStreamWrapper> stream, std::string prefix,
                           Ipv4InterfaceContainer c);

  void EnableAsciiIpv4Impl(Ptr<OutputStreamWrapper> stream, std::string prefix,
                           std::string ipv4Name, uint32_t interface,
                           bool explicitFilename);

  void EnableAsciiIpv4Impl(Ptr<OutputStreamWrapper> stream, std::string prefix,
                           Ptr<Ipv4> ipv4, uint32_t interface,
                           bool explicitFilename);
};

class PcapHelperForIpv6 {
public:
  PcapHelperForIpv6() {}

  virtual ~PcapHelperForIpv6() {}

  virtual void EnablePcapIpv6Internal(std::string prefix, Ptr<Ipv6> ipv6,
                                      uint32_t interface,
                                      bool explicitFilename) = 0;

  void EnablePcapIpv6(std::string prefix, Ptr<Ipv6> ipv6, uint32_t interface,
                      bool explicitFilename = false);

  void EnablePcapIpv6(std::string prefix, std::string ipv6Name,
                      uint32_t interface, bool explicitFilename = false);

  void EnablePcapIpv6(std::string prefix, Ipv6InterfaceContainer c);

  void EnablePcapIpv6(std::string prefix, NodeContainer n);

  void EnablePcapIpv6(std::string prefix, uint32_t nodeid, uint32_t interface,
                      bool explicitFilename);

  void EnablePcapIpv6All(std::string prefix);
};

class AsciiTraceHelperForIpv6 {
public:
  AsciiTraceHelperForIpv6() {}

  virtual ~AsciiTraceHelperForIpv6() {}

  virtual void EnableAsciiIpv6Internal(Ptr<OutputStreamWrapper> stream,
                                       std::string prefix, Ptr<Ipv6> ipv6,
                                       uint32_t interface,
                                       bool explicitFilename) = 0;

  void EnableAsciiIpv6(std::string prefix, Ptr<Ipv6> ipv6, uint32_t interface,
                       bool explicitFilename = false);

  void EnableAsciiIpv6(Ptr<OutputStreamWrapper> stream, Ptr<Ipv6> ipv6,
                       uint32_t interface);

  void EnableAsciiIpv6(std::string prefix, std::string ipv6Name,
                       uint32_t interface, bool explicitFilename = false);

  void EnableAsciiIpv6(Ptr<OutputStreamWrapper> stream, std::string ipv6Name,
                       uint32_t interface);

  void EnableAsciiIpv6(std::string prefix, Ipv6InterfaceContainer c);

  void EnableAsciiIpv6(Ptr<OutputStreamWrapper> stream,
                       Ipv6InterfaceContainer c);

  void EnableAsciiIpv6(std::string prefix, NodeContainer n);

  void EnableAsciiIpv6(Ptr<OutputStreamWrapper> stream, NodeContainer n);

  void EnableAsciiIpv6(std::string prefix, uint32_t nodeid, uint32_t interface,
                       bool explicitFilename);

  void EnableAsciiIpv6(Ptr<OutputStreamWrapper> stream, uint32_t nodeid,
                       uint32_t interface);

  void EnableAsciiIpv6All(std::string prefix);

  void EnableAsciiIpv6All(Ptr<OutputStreamWrapper> stream);

private:
  void EnableAsciiIpv6Impl(Ptr<OutputStreamWrapper> stream, std::string prefix,
                           uint32_t nodeid, uint32_t interface,
                           bool explicitFilename);

  void EnableAsciiIpv6Impl(Ptr<OutputStreamWrapper> stream, std::string prefix,
                           NodeContainer n);

  void EnableAsciiIpv6Impl(Ptr<OutputStreamWrapper> stream, std::string prefix,
                           Ipv6InterfaceContainer c);

  void EnableAsciiIpv6Impl(Ptr<OutputStreamWrapper> stream, std::string prefix,
                           std::string ipv6Name, uint32_t interface,
                           bool explicitFilename);

  void EnableAsciiIpv6Impl(Ptr<OutputStreamWrapper> stream, std::string prefix,
                           Ptr<Ipv6> ipv6, uint32_t interface,
                           bool explicitFilename);
};

} // namespace ns3

#endif
