
#include "ipv4-end-point-demux.h"

#include "ipv4-end-point.h"
#include "ipv4-interface-address.h"

#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Ipv4EndPointDemux");

Ipv4EndPointDemux::Ipv4EndPointDemux()
    : m_ephemeral(49152), m_portLast(65535), m_portFirst(49152) {
  NS_LOG_FUNCTION(this);
}

Ipv4EndPointDemux::~Ipv4EndPointDemux() {
  NS_LOG_FUNCTION(this);
  for (auto i = m_endPoints.begin(); i != m_endPoints.end(); i++) {
    Ipv4EndPoint *endPoint = *i;
    delete endPoint;
  }
  m_endPoints.clear();
}

bool Ipv4EndPointDemux::LookupPortLocal(uint16_t port) {
  NS_LOG_FUNCTION(this << port);
  for (auto i = m_endPoints.begin(); i != m_endPoints.end(); i++) {
    if ((*i)->GetLocalPort() == port) {
      return true;
    }
  }
  return false;
}

bool Ipv4EndPointDemux::LookupLocal(Ptr<NetDevice> boundNetDevice,
                                    Ipv4Address addr, uint16_t port) {
  NS_LOG_FUNCTION(this << addr << port);
  for (auto i = m_endPoints.begin(); i != m_endPoints.end(); i++) {
    if ((*i)->GetLocalPort() == port && (*i)->GetLocalAddress() == addr &&
        (*i)->GetBoundNetDevice() == boundNetDevice) {
      return true;
    }
  }
  return false;
}

Ipv4EndPoint *Ipv4EndPointDemux::Allocate() {
  NS_LOG_FUNCTION(this);
  uint16_t port = AllocateEphemeralPort();
  if (port == 0) {
    NS_LOG_WARN("Ephemeral port allocation failed.");
    return nullptr;
  }
  auto endPoint = new Ipv4EndPoint(Ipv4Address::GetAny(), port);
  m_endPoints.push_back(endPoint);
  NS_LOG_DEBUG("Now have >>" << m_endPoints.size() << "<< endpoints.");
  return endPoint;
}

Ipv4EndPoint *Ipv4EndPointDemux::Allocate(Ipv4Address address) {
  NS_LOG_FUNCTION(this << address);
  uint16_t port = AllocateEphemeralPort();
  if (port == 0) {
    NS_LOG_WARN("Ephemeral port allocation failed.");
    return nullptr;
  }
  auto endPoint = new Ipv4EndPoint(address, port);
  m_endPoints.push_back(endPoint);
  NS_LOG_DEBUG("Now have >>" << m_endPoints.size() << "<< endpoints.");
  return endPoint;
}

Ipv4EndPoint *Ipv4EndPointDemux::Allocate(Ptr<NetDevice> boundNetDevice,
                                          uint16_t port) {
  NS_LOG_FUNCTION(this << port << boundNetDevice);

  return Allocate(boundNetDevice, Ipv4Address::GetAny(), port);
}

Ipv4EndPoint *Ipv4EndPointDemux::Allocate(Ptr<NetDevice> boundNetDevice,
                                          Ipv4Address address, uint16_t port) {
  NS_LOG_FUNCTION(this << address << port << boundNetDevice);
  if (LookupLocal(boundNetDevice, address, port) ||
      LookupLocal(nullptr, address, port)) {
    NS_LOG_WARN("Duplicated endpoint.");
    return nullptr;
  }
  auto endPoint = new Ipv4EndPoint(address, port);
  m_endPoints.push_back(endPoint);
  NS_LOG_DEBUG("Now have >>" << m_endPoints.size() << "<< endpoints.");
  return endPoint;
}

Ipv4EndPoint *Ipv4EndPointDemux::Allocate(Ptr<NetDevice> boundNetDevice,
                                          Ipv4Address localAddress,
                                          uint16_t localPort,
                                          Ipv4Address peerAddress,
                                          uint16_t peerPort) {
  NS_LOG_FUNCTION(this << localAddress << localPort << peerAddress << peerPort
                       << boundNetDevice);
  for (auto i = m_endPoints.begin(); i != m_endPoints.end(); i++) {
    if ((*i)->GetLocalPort() == localPort &&
        (*i)->GetLocalAddress() == localAddress &&
        (*i)->GetPeerPort() == peerPort &&
        (*i)->GetPeerAddress() == peerAddress &&
        ((*i)->GetBoundNetDevice() == boundNetDevice ||
         !(*i)->GetBoundNetDevice())) {
      NS_LOG_WARN("Duplicated endpoint.");
      return nullptr;
    }
  }
  auto endPoint = new Ipv4EndPoint(localAddress, localPort);
  endPoint->SetPeer(peerAddress, peerPort);
  m_endPoints.push_back(endPoint);

  NS_LOG_DEBUG("Now have >>" << m_endPoints.size() << "<< endpoints.");

  return endPoint;
}

void Ipv4EndPointDemux::DeAllocate(Ipv4EndPoint *endPoint) {
  NS_LOG_FUNCTION(this << endPoint);
  for (auto i = m_endPoints.begin(); i != m_endPoints.end(); i++) {
    if (*i == endPoint) {
      delete endPoint;
      m_endPoints.erase(i);
      break;
    }
  }
}

Ipv4EndPointDemux::EndPoints Ipv4EndPointDemux::GetAllEndPoints() {
  NS_LOG_FUNCTION(this);
  EndPoints ret;

  for (auto i = m_endPoints.begin(); i != m_endPoints.end(); i++) {
    Ipv4EndPoint *endP = *i;
    ret.push_back(endP);
  }
  return ret;
}

Ipv4EndPointDemux::EndPoints
Ipv4EndPointDemux::Lookup(Ipv4Address daddr, uint16_t dport, Ipv4Address saddr,
                          uint16_t sport,
                          Ptr<Ipv4Interface> incomingInterface) {
  NS_LOG_FUNCTION(this << daddr << dport << saddr << sport
                       << incomingInterface);

  EndPoints retval1;
  EndPoints retval2;
  EndPoints retval3;
  EndPoints retval4;

  NS_LOG_DEBUG("Looking up endpoint for destination address " << daddr << ":"
                                                              << dport);
  for (auto i = m_endPoints.begin(); i != m_endPoints.end(); i++) {
    Ipv4EndPoint *endP = *i;

    NS_LOG_DEBUG("Looking at endpoint dport="
                 << endP->GetLocalPort() << " daddr=" << endP->GetLocalAddress()
                 << " sport=" << endP->GetPeerPort()
                 << " saddr=" << endP->GetPeerAddress());

    if (!endP->IsRxEnabled()) {
      NS_LOG_LOGIC("Skipping endpoint "
                   << &endP << " because endpoint can not receive packets");
      continue;
    }

    if (endP->GetLocalPort() != dport) {
      NS_LOG_LOGIC("Skipping endpoint " << &endP << " because endpoint dport "
                                        << endP->GetLocalPort()
                                        << " does not match packet dport "
                                        << dport);
      continue;
    }
    if (endP->GetBoundNetDevice()) {
      if (endP->GetBoundNetDevice() != incomingInterface->GetDevice()) {
        NS_LOG_LOGIC(
            "Skipping endpoint "
            << &endP << " because endpoint is bound to specific device and"
            << endP->GetBoundNetDevice() << " does not match packet device "
            << incomingInterface->GetDevice());
        continue;
      }
    }

    bool localAddressMatchesExact = false;
    bool localAddressIsAny = false;
    bool localAddressIsSubnetAny = false;

    if (endP->GetLocalAddress() == daddr) {
      localAddressMatchesExact = true;
    } else if (endP->GetLocalAddress() == Ipv4Address::GetAny()) {
      localAddressIsAny = true;
    } else {
      for (uint32_t i = 0; i < incomingInterface->GetNAddresses(); i++) {
        Ipv4InterfaceAddress addr = incomingInterface->GetAddress(i);

        Ipv4Address addrNetpart = addr.GetLocal().CombineMask(addr.GetMask());
        if (endP->GetLocalAddress() == addrNetpart) {
          NS_LOG_LOGIC("Endpoint is SubnetDirectedAny "
                       << endP->GetLocalAddress() << "/"
                       << addr.GetMask().GetPrefixLength());

          Ipv4Address daddrNetPart = daddr.CombineMask(addr.GetMask());
          if (addrNetpart == daddrNetPart) {
            localAddressIsSubnetAny = true;
          }
        }
      }

      if (!localAddressIsSubnetAny) {
        continue;
      }
    }

    bool remotePortMatchesExact = endP->GetPeerPort() == sport;
    bool remotePortMatchesWildCard = endP->GetPeerPort() == 0;
    bool remoteAddressMatchesExact = endP->GetPeerAddress() == saddr;
    bool remoteAddressMatchesWildCard =
        endP->GetPeerAddress() == Ipv4Address::GetAny();

    if (!(remotePortMatchesExact || remotePortMatchesWildCard)) {
      continue;
    }
    if (!(remoteAddressMatchesExact || remoteAddressMatchesWildCard)) {
      continue;
    }

    bool localAddressMatchesWildCard =
        localAddressIsAny || localAddressIsSubnetAny;

    if (localAddressMatchesExact && remoteAddressMatchesExact &&
        remotePortMatchesExact) {
      NS_LOG_LOGIC("Found an endpoint for case 4, adding "
                   << endP->GetLocalAddress() << ":" << endP->GetLocalPort());
      retval4.push_back(endP);
    }
    if (localAddressMatchesWildCard && remoteAddressMatchesExact &&
        remotePortMatchesExact) {
      NS_LOG_LOGIC("Found an endpoint for case 3, adding "
                   << endP->GetLocalAddress() << ":" << endP->GetLocalPort());
      retval3.push_back(endP);
    }
    if (localAddressMatchesExact && remoteAddressMatchesWildCard &&
        remotePortMatchesWildCard) {
      NS_LOG_LOGIC("Found an endpoint for case 2, adding "
                   << endP->GetLocalAddress() << ":" << endP->GetLocalPort());
      retval2.push_back(endP);
    }
    if (localAddressMatchesWildCard && remoteAddressMatchesWildCard &&
        remotePortMatchesWildCard) {
      NS_LOG_LOGIC("Found an endpoint for case 1, adding "
                   << endP->GetLocalAddress() << ":" << endP->GetLocalPort());
      retval1.push_back(endP);
    }
  }

  EndPoints retval;
  if (!retval4.empty()) {
    retval = retval4;
  } else if (!retval3.empty()) {
    retval = retval3;
  } else if (!retval2.empty()) {
    retval = retval2;
  } else {
    retval = retval1;
  }

  NS_ABORT_MSG_IF(retval.size() > 1, "Too many endpoints - perhaps you created "
                                     "too many sockets without binding "
                                     "them to different NetDevices.");
  return retval;
}

Ipv4EndPoint *Ipv4EndPointDemux::SimpleLookup(Ipv4Address daddr, uint16_t dport,
                                              Ipv4Address saddr,
                                              uint16_t sport) {
  NS_LOG_FUNCTION(this << daddr << dport << saddr << sport);

  uint32_t genericity = 3;
  Ipv4EndPoint *generic = nullptr;
  for (auto i = m_endPoints.begin(); i != m_endPoints.end(); i++) {
    if ((*i)->GetLocalPort() != dport) {
      continue;
    }
    if ((*i)->GetLocalAddress() == daddr && (*i)->GetPeerPort() == sport &&
        (*i)->GetPeerAddress() == saddr) {
      return *i;
    }
    uint32_t tmp = 0;
    if ((*i)->GetLocalAddress() == Ipv4Address::GetAny()) {
      tmp++;
    }
    if ((*i)->GetPeerAddress() == Ipv4Address::GetAny()) {
      tmp++;
    }
    if (tmp < genericity) {
      generic = (*i);
      genericity = tmp;
    }
  }
  return generic;
}

uint16_t Ipv4EndPointDemux::AllocateEphemeralPort() {
  NS_LOG_FUNCTION(this);
  uint16_t port = m_ephemeral;
  int count = m_portLast - m_portFirst;
  do {
    if (count-- < 0) {
      return 0;
    }
    ++port;
    if (port < m_portFirst || port > m_portLast) {
      port = m_portFirst;
    }
  } while (LookupPortLocal(port));
  m_ephemeral = port;
  return port;
}

} // namespace ns3
