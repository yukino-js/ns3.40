
#include "ipv6-address-generator.h"

#include "ns3/abort.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/simulation-singleton.h"

#include <list>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Ipv6AddressGenerator");

class Ipv6AddressGeneratorImpl {
public:
  Ipv6AddressGeneratorImpl();
  virtual ~Ipv6AddressGeneratorImpl();

  void Init(const Ipv6Address net, const Ipv6Prefix prefix,
            const Ipv6Address interfaceId);

  Ipv6Address NextNetwork(const Ipv6Prefix prefix);

  Ipv6Address GetNetwork(const Ipv6Prefix prefix) const;

  void InitAddress(const Ipv6Address interfaceId, const Ipv6Prefix prefix);

  Ipv6Address GetAddress(const Ipv6Prefix prefix) const;

  Ipv6Address NextAddress(const Ipv6Prefix prefix);

  void Reset();

  bool AddAllocated(const Ipv6Address addr);

  bool IsAddressAllocated(const Ipv6Address addr);

  bool IsNetworkAllocated(const Ipv6Address addr, const Ipv6Prefix prefix);

  void TestMode();

private:
  static const uint32_t N_BITS = 128;
  static const uint32_t MOST_SIGNIFICANT_BIT = 0x80;

  uint32_t PrefixToIndex(Ipv6Prefix prefix) const;

  class NetworkState {
  public:
    uint8_t prefix[16];
    uint32_t shift;
    uint8_t network[16];
    uint8_t addr[16];
    uint8_t addrMax[16];
  };

  NetworkState m_netTable[N_BITS];

  class Entry {
  public:
    uint8_t addrLow[16];
    uint8_t addrHigh[16];
  };

  std::list<Entry> m_entries;
  Ipv6Address m_base;
  bool m_test;
};

Ipv6AddressGeneratorImpl::Ipv6AddressGeneratorImpl()
    : m_entries(), m_base("::1"), m_test(false) {
  NS_LOG_FUNCTION(this);
  Reset();
}

void Ipv6AddressGeneratorImpl::Reset() {
  NS_LOG_FUNCTION(this);

  uint8_t prefix[16] = {0};

  for (uint32_t i = 0; i < N_BITS; ++i) {
    for (uint32_t j = 0; j < 16; ++j) {
      m_netTable[i].prefix[j] = prefix[j];
    }
    for (uint32_t j = 0; j < 15; ++j) {
      prefix[15 - j] >>= 1;
      prefix[15 - j] |= (prefix[15 - j - 1] & 1);
    }
    prefix[0] |= MOST_SIGNIFICANT_BIT;
    for (uint32_t j = 0; j < 15; ++j) {
      m_netTable[i].network[j] = 0;
    }
    m_netTable[i].network[15] = 1;
    for (uint32_t j = 0; j < 15; ++j) {
      m_netTable[i].addr[j] = 0;
    }
    m_netTable[i].addr[15] = 1;
    for (uint32_t j = 0; j < 16; ++j) {
      m_netTable[i].addrMax[j] = ~prefix[j];
    }
    m_netTable[i].shift = N_BITS - i;
  }
  m_entries.clear();
  m_base = Ipv6Address("::1");
  m_test = false;
}

Ipv6AddressGeneratorImpl::~Ipv6AddressGeneratorImpl() { NS_LOG_FUNCTION(this); }

void Ipv6AddressGeneratorImpl::Init(const Ipv6Address net,
                                    const Ipv6Prefix prefix,
                                    const Ipv6Address interfaceId) {
  NS_LOG_FUNCTION(this << net << prefix << interfaceId);

  m_base = interfaceId;
  uint8_t prefixBits[16];
  prefix.GetBytes(prefixBits);
  uint8_t netBits[16];
  net.GetBytes(netBits);
  uint8_t interfaceIdBits[16];
  interfaceId.GetBytes(interfaceIdBits);
  uint32_t index = PrefixToIndex(prefix);
  NS_LOG_DEBUG("Index " << index);
  uint32_t a = m_netTable[index].shift / 8;
  uint32_t b = m_netTable[index].shift % 8;
  for (int32_t j = 15 - a; j >= 0; j--) {
    m_netTable[index].network[j + a] = netBits[j];
  }
  for (uint32_t j = 0; j < a; j++) {
    m_netTable[index].network[j] = 0;
  }
  for (uint32_t j = 15; j >= a; j--) {
    m_netTable[index].network[j] = m_netTable[index].network[j] >> b;
    m_netTable[index].network[j] |= m_netTable[index].network[j - 1] << (8 - b);
  }
  for (int32_t j = 0; j < 16; j++) {
    m_netTable[index].addr[j] = interfaceIdBits[j];
  }
}

Ipv6Address
Ipv6AddressGeneratorImpl::GetNetwork(const Ipv6Prefix prefix) const {
  NS_LOG_FUNCTION(this);
  uint8_t nw[16] = {0};
  uint32_t index = PrefixToIndex(prefix);
  uint32_t a = m_netTable[index].shift / 8;
  uint32_t b = m_netTable[index].shift % 8;
  for (uint32_t j = 0; j < 16 - a; ++j) {
    nw[j] = m_netTable[index].network[j + a];
  }
  for (uint32_t j = 0; j < 15; j++) {
    nw[j] = nw[j] << b;
    nw[j] |= nw[j + 1] >> (8 - b);
  }
  nw[15] = nw[15] << b;

  return Ipv6Address(nw);
}

Ipv6Address Ipv6AddressGeneratorImpl::NextNetwork(const Ipv6Prefix prefix) {
  NS_LOG_FUNCTION(this);

  uint32_t index = PrefixToIndex(prefix);
  uint8_t interfaceIdBits[16];
  m_base.GetBytes(interfaceIdBits);
  for (int32_t j = 0; j < 16; j++) {
    m_netTable[index].addr[j] = interfaceIdBits[j];
  }

  for (int32_t j = 15; j >= 0; j--) {
    if (m_netTable[index].network[j] < 0xff) {
      ++m_netTable[index].network[j];
      break;
    } else {
      ++m_netTable[index].network[j];
    }
  }

  uint8_t nw[16];
  uint32_t a = m_netTable[index].shift / 8;
  uint32_t b = m_netTable[index].shift % 8;
  for (uint32_t j = 0; j < 16 - a; ++j) {
    nw[j] = m_netTable[index].network[j + a];
  }
  for (uint32_t j = 16 - a; j < 16; ++j) {
    nw[j] = 0;
  }
  for (uint32_t j = 0; j < 15; j++) {
    nw[j] = nw[j] << b;
    nw[j] |= nw[j + 1] >> (8 - b);
  }
  nw[15] = nw[15] << b;

  return Ipv6Address(nw);
}

void Ipv6AddressGeneratorImpl::InitAddress(const Ipv6Address interfaceId,
                                           const Ipv6Prefix prefix) {
  NS_LOG_FUNCTION(this);

  uint32_t index = PrefixToIndex(prefix);
  uint8_t interfaceIdBits[16];
  interfaceId.GetBytes(interfaceIdBits);

  for (uint32_t j = 0; j < 16; ++j) {
    m_netTable[index].addr[j] = interfaceIdBits[j];
  }
}

Ipv6Address
Ipv6AddressGeneratorImpl::GetAddress(const Ipv6Prefix prefix) const {
  NS_LOG_FUNCTION(this);

  uint32_t index = PrefixToIndex(prefix);

  uint8_t nw[16] = {0};
  uint32_t a = m_netTable[index].shift / 8;
  uint32_t b = m_netTable[index].shift % 8;
  for (uint32_t j = 0; j < 16 - a; ++j) {
    nw[j] = m_netTable[index].network[j + a];
  }
  for (uint32_t j = 0; j < 15; j++) {
    nw[j] = nw[j] << b;
    nw[j] |= nw[j + 1] >> (8 - b);
  }
  nw[15] = nw[15] << b;
  for (uint32_t j = 0; j < 16; j++) {
    nw[j] |= m_netTable[index].addr[j];
  }

  return Ipv6Address(nw);
}

Ipv6Address Ipv6AddressGeneratorImpl::NextAddress(const Ipv6Prefix prefix) {
  NS_LOG_FUNCTION(this);

  uint32_t index = PrefixToIndex(prefix);

  uint8_t ad[16] = {0};
  uint32_t a = m_netTable[index].shift / 8;
  uint32_t b = m_netTable[index].shift % 8;
  for (uint32_t j = 0; j < 16 - a; ++j) {
    ad[j] = m_netTable[index].network[j + a];
  }
  for (uint32_t j = 0; j < 15; j++) {
    ad[j] = ad[j] << b;
    ad[j] |= ad[j + 1] >> (8 - b);
  }
  ad[15] = ad[15] << b;
  for (uint32_t j = 0; j < 16; j++) {
    ad[j] |= m_netTable[index].addr[j];
  }
  Ipv6Address addr = Ipv6Address(ad);

  for (int32_t j = 15; j >= 0; j--) {
    if (m_netTable[index].addr[j] < 0xff) {
      ++m_netTable[index].addr[j];
      break;
    } else {
      ++m_netTable[index].addr[j];
    }
  }

  AddAllocated(addr);
  return addr;
}

bool Ipv6AddressGeneratorImpl::AddAllocated(const Ipv6Address address) {
  NS_LOG_FUNCTION(this << address);

  uint8_t addr[16];
  address.GetBytes(addr);

  std::list<Entry>::iterator i;

  for (i = m_entries.begin(); i != m_entries.end(); ++i) {
    NS_LOG_LOGIC("examine entry: " << Ipv6Address((*i).addrLow) << " to "
                                   << Ipv6Address((*i).addrHigh));
    if (!(Ipv6Address(addr) < Ipv6Address((*i).addrLow)) &&
        ((Ipv6Address(addr) < Ipv6Address((*i).addrHigh)) ||
         (Ipv6Address(addr) == Ipv6Address((*i).addrHigh)))) {
      NS_LOG_LOGIC("Ipv6AddressGeneratorImpl::Add(): Address Collision: "
                   << Ipv6Address(addr));
      if (!m_test) {
        NS_FATAL_ERROR("Ipv6AddressGeneratorImpl::Add(): Address Collision: "
                       << Ipv6Address(addr));
      }
      return false;
    }
    uint8_t taddr[16];
    for (uint32_t j = 0; j < 16; j++) {
      taddr[j] = (*i).addrLow[j];
    }
    taddr[15] -= 1;
    if (Ipv6Address(addr) < Ipv6Address(taddr)) {
      break;
    }
    for (uint32_t j = 0; j < 16; j++) {
      taddr[j] = (*i).addrLow[j];
    }
    taddr[15] += 1;
    if (Ipv6Address(addr) == Ipv6Address(taddr)) {
      auto j = i;
      ++j;

      if (j != m_entries.end()) {
        if (Ipv6Address(addr) == Ipv6Address((*j).addrLow)) {
          NS_LOG_LOGIC("Ipv6AddressGeneratorImpl::Add(): "
                       "Address Collision: "
                       << Ipv6Address(addr));
          if (!m_test) {
            NS_FATAL_ERROR(
                "Ipv6AddressGeneratorImpl::Add(): Address Collision: "
                << Ipv6Address(addr));
          }
          return false;
        }
      }

      NS_LOG_LOGIC("New addrHigh = " << Ipv6Address(addr));
      for (uint32_t j = 0; j < 16; j++) {
        (*i).addrHigh[j] = addr[j];
      }
      return true;
    }
    for (uint32_t j = 0; j < 16; j++) {
      taddr[j] = (*i).addrLow[j];
    }
    taddr[15] -= 1;
    if ((Ipv6Address(addr) == Ipv6Address(taddr))) {
      NS_LOG_LOGIC("New addrLow = " << Ipv6Address(addr));
      for (uint32_t j = 0; j < 16; j++) {
        (*i).addrLow[j] = addr[j];
      }
      return true;
    }
  }

  Entry entry;
  for (uint32_t j = 0; j < 16; j++) {
    entry.addrLow[j] = entry.addrHigh[j] = addr[j];
  }
  m_entries.insert(i, entry);
  return true;
}

bool Ipv6AddressGeneratorImpl::IsAddressAllocated(const Ipv6Address address) {
  NS_LOG_FUNCTION(this << address);

  uint8_t addr[16];
  address.GetBytes(addr);

  for (auto i = m_entries.begin(); i != m_entries.end(); ++i) {
    NS_LOG_LOGIC("examine entry: " << Ipv6Address((*i).addrLow) << " to "
                                   << Ipv6Address((*i).addrHigh));

    if (!(Ipv6Address(addr) < Ipv6Address((*i).addrLow)) &&
        ((Ipv6Address(addr) < Ipv6Address((*i).addrHigh)) ||
         (Ipv6Address(addr) == Ipv6Address((*i).addrHigh)))) {
      NS_LOG_LOGIC(
          "Ipv6AddressGeneratorImpl::IsAddressAllocated(): Address Collision: "
          << Ipv6Address(addr));
      return true;
    }
  }
  return false;
}

bool Ipv6AddressGeneratorImpl::IsNetworkAllocated(const Ipv6Address address,
                                                  const Ipv6Prefix prefix) {
  NS_LOG_FUNCTION(this << address << prefix);

  NS_ABORT_MSG_UNLESS(address == address.CombinePrefix(prefix),
                      "Ipv6AddressGeneratorImpl::IsNetworkAllocated(): network "
                      "address and mask don't match "
                          << address << " " << prefix);

  for (auto i = m_entries.begin(); i != m_entries.end(); ++i) {
    NS_LOG_LOGIC("examine entry: " << Ipv6Address((*i).addrLow) << " to "
                                   << Ipv6Address((*i).addrHigh));
    Ipv6Address low = Ipv6Address((*i).addrLow);
    Ipv6Address high = Ipv6Address((*i).addrHigh);

    if (address == low.CombinePrefix(prefix) ||
        address == high.CombinePrefix(prefix)) {
      NS_LOG_LOGIC("Ipv6AddressGeneratorImpl::IsNetworkAllocated(): Network "
                   "already allocated: "
                   << address << " " << low << "-" << high);
      return false;
    }
  }
  return true;
}

void Ipv6AddressGeneratorImpl::TestMode() {
  NS_LOG_FUNCTION(this);
  m_test = true;
}

uint32_t Ipv6AddressGeneratorImpl::PrefixToIndex(Ipv6Prefix prefix) const {
  uint8_t prefixBits[16];
  prefix.GetBytes(prefixBits);

  for (int32_t i = 15; i >= 0; --i) {
    for (uint32_t j = 0; j < 8; ++j) {
      if (prefixBits[i] & 1) {
        uint32_t index = N_BITS - (15 - i) * 8 - j;
        NS_ABORT_MSG_UNLESS(
            index > 0 && index < N_BITS,
            "Ip64AddressGenerator::PrefixToIndex(): Illegal Prefix");
        return index;
      }
      prefixBits[i] >>= 1;
    }
  }
  NS_ASSERT_MSG(false, "Ipv6AddressGenerator::PrefixToIndex(): Impossible");
  return 0;
}

void Ipv6AddressGenerator::Init(const Ipv6Address net, const Ipv6Prefix prefix,
                                const Ipv6Address interfaceId) {
  NS_LOG_FUNCTION(net << prefix << interfaceId);

  SimulationSingleton<Ipv6AddressGeneratorImpl>::Get()->Init(net, prefix,
                                                             interfaceId);
}

Ipv6Address Ipv6AddressGenerator::NextNetwork(const Ipv6Prefix prefix) {
  NS_LOG_FUNCTION(prefix);

  return SimulationSingleton<Ipv6AddressGeneratorImpl>::Get()->NextNetwork(
      prefix);
}

Ipv6Address Ipv6AddressGenerator::GetNetwork(const Ipv6Prefix prefix) {
  NS_LOG_FUNCTION(prefix);

  return SimulationSingleton<Ipv6AddressGeneratorImpl>::Get()->GetNetwork(
      prefix);
}

void Ipv6AddressGenerator::InitAddress(const Ipv6Address interfaceId,
                                       const Ipv6Prefix prefix) {
  NS_LOG_FUNCTION(interfaceId << prefix);

  SimulationSingleton<Ipv6AddressGeneratorImpl>::Get()->InitAddress(interfaceId,
                                                                    prefix);
}

Ipv6Address Ipv6AddressGenerator::GetAddress(const Ipv6Prefix prefix) {
  NS_LOG_FUNCTION(prefix);

  return SimulationSingleton<Ipv6AddressGeneratorImpl>::Get()->GetAddress(
      prefix);
}

Ipv6Address Ipv6AddressGenerator::NextAddress(const Ipv6Prefix prefix) {
  NS_LOG_FUNCTION(prefix);

  return SimulationSingleton<Ipv6AddressGeneratorImpl>::Get()->NextAddress(
      prefix);
}

void Ipv6AddressGenerator::Reset() {
  NS_LOG_FUNCTION_NOARGS();

  return SimulationSingleton<Ipv6AddressGeneratorImpl>::Get()->Reset();
}

bool Ipv6AddressGenerator::AddAllocated(const Ipv6Address addr) {
  NS_LOG_FUNCTION(addr);

  return SimulationSingleton<Ipv6AddressGeneratorImpl>::Get()->AddAllocated(
      addr);
}

bool Ipv6AddressGenerator::IsAddressAllocated(const Ipv6Address addr) {
  NS_LOG_FUNCTION(addr);

  return SimulationSingleton<Ipv6AddressGeneratorImpl>::Get()
      ->IsAddressAllocated(addr);
}

bool Ipv6AddressGenerator::IsNetworkAllocated(const Ipv6Address addr,
                                              const Ipv6Prefix prefix) {
  NS_LOG_FUNCTION(addr << prefix);

  return SimulationSingleton<Ipv6AddressGeneratorImpl>::Get()
      ->IsNetworkAllocated(addr, prefix);
}

void Ipv6AddressGenerator::TestMode() {
  NS_LOG_FUNCTION_NOARGS();

  SimulationSingleton<Ipv6AddressGeneratorImpl>::Get()->TestMode();
}

} // namespace ns3
