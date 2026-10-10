
#include "ipv4-address-generator.h"

#include "ns3/abort.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/simulation-singleton.h"

#include <list>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Ipv4AddressGenerator");

class Ipv4AddressGeneratorImpl {
public:
  Ipv4AddressGeneratorImpl();
  virtual ~Ipv4AddressGeneratorImpl();

  void Init(const Ipv4Address net, const Ipv4Mask mask, const Ipv4Address addr);

  Ipv4Address GetNetwork(const Ipv4Mask mask) const;

  Ipv4Address NextNetwork(const Ipv4Mask mask);

  void InitAddress(const Ipv4Address addr, const Ipv4Mask mask);

  Ipv4Address NextAddress(const Ipv4Mask mask);

  Ipv4Address GetAddress(const Ipv4Mask mask) const;

  void Reset();

  bool AddAllocated(const Ipv4Address addr);

  bool IsAddressAllocated(const Ipv4Address addr);

  bool IsNetworkAllocated(const Ipv4Address addr, const Ipv4Mask mask);

  void TestMode();

private:
  static const uint32_t N_BITS = 32;
  static const uint32_t MOST_SIGNIFICANT_BIT = 0x80000000;

  uint32_t MaskToIndex(Ipv4Mask mask) const;

  class NetworkState {
  public:
    uint32_t mask;
    uint32_t shift;
    uint32_t network;
    uint32_t addr;
    uint32_t addrMax;
  };

  NetworkState m_netTable[N_BITS];

  class Entry {
  public:
    uint32_t addrLow;
    uint32_t addrHigh;
  };

  std::list<Entry> m_entries;
  bool m_test;
};

Ipv4AddressGeneratorImpl::Ipv4AddressGeneratorImpl()
    : m_entries(), m_test(false) {
  NS_LOG_FUNCTION(this);
  Reset();
}

void Ipv4AddressGeneratorImpl::Reset() {
  NS_LOG_FUNCTION(this);

  uint32_t mask = 0;
  for (uint32_t i = 0; i < N_BITS; ++i) {
    m_netTable[i].mask = mask;
    mask >>= 1;
    mask |= MOST_SIGNIFICANT_BIT;
    m_netTable[i].network = 1;
    m_netTable[i].addr = 1;
    m_netTable[i].addrMax = ~m_netTable[i].mask;
    m_netTable[i].shift = N_BITS - i;
  }
  m_entries.clear();
  m_test = false;
}

Ipv4AddressGeneratorImpl::~Ipv4AddressGeneratorImpl() { NS_LOG_FUNCTION(this); }

void Ipv4AddressGeneratorImpl::Init(const Ipv4Address net, const Ipv4Mask mask,
                                    const Ipv4Address addr) {
  NS_LOG_FUNCTION(this << net << mask << addr);
  uint32_t maskBits = mask.Get();
  uint32_t netBits = net.Get();
  uint32_t addrBits = addr.Get();
  NS_ABORT_MSG_UNLESS(
      (netBits & ~maskBits) == 0,
      "Ipv4AddressGeneratorImpl::Init (): Inconsistent network and mask");
  NS_ABORT_MSG_UNLESS(
      (addrBits & maskBits) == 0,
      "Ipv4AddressGeneratorImpl::Init (): Inconsistent address and mask");

  uint32_t index = MaskToIndex(mask);

  m_netTable[index].network = netBits >> m_netTable[index].shift;

  NS_ABORT_MSG_UNLESS(addrBits <= m_netTable[index].addrMax,
                      "Ipv4AddressGeneratorImpl::Init(): Address overflow");
  m_netTable[index].addr = addrBits;
}

Ipv4Address Ipv4AddressGeneratorImpl::GetNetwork(const Ipv4Mask mask) const {
  NS_LOG_FUNCTION(this << mask);

  uint32_t index = MaskToIndex(mask);
  return Ipv4Address(m_netTable[index].network << m_netTable[index].shift);
}

Ipv4Address Ipv4AddressGeneratorImpl::NextNetwork(const Ipv4Mask mask) {
  NS_LOG_FUNCTION(this << mask);
  uint32_t index = MaskToIndex(mask);
  ++m_netTable[index].network;
  return Ipv4Address(m_netTable[index].network << m_netTable[index].shift);
}

void Ipv4AddressGeneratorImpl::InitAddress(const Ipv4Address addr,
                                           const Ipv4Mask mask) {
  NS_LOG_FUNCTION(this << addr << mask);

  uint32_t index = MaskToIndex(mask);
  uint32_t addrBits = addr.Get();

  NS_ABORT_MSG_UNLESS(
      addrBits <= m_netTable[index].addrMax,
      "Ipv4AddressGeneratorImpl::InitAddress(): Address overflow");
  m_netTable[index].addr = addrBits;
}

Ipv4Address Ipv4AddressGeneratorImpl::GetAddress(const Ipv4Mask mask) const {
  NS_LOG_FUNCTION(this << mask);

  uint32_t index = MaskToIndex(mask);

  return Ipv4Address((m_netTable[index].network << m_netTable[index].shift) |
                     m_netTable[index].addr);
}

Ipv4Address Ipv4AddressGeneratorImpl::NextAddress(const Ipv4Mask mask) {
  NS_LOG_FUNCTION(this << mask);
  uint32_t index = MaskToIndex(mask);

  NS_ABORT_MSG_UNLESS(
      m_netTable[index].addr <= m_netTable[index].addrMax,
      "Ipv4AddressGeneratorImpl::NextAddress(): Address overflow");

  Ipv4Address addr((m_netTable[index].network << m_netTable[index].shift) |
                   m_netTable[index].addr);

  ++m_netTable[index].addr;
  AddAllocated(addr);
  return addr;
}

bool Ipv4AddressGeneratorImpl::AddAllocated(const Ipv4Address address) {
  NS_LOG_FUNCTION(this << address);

  uint32_t addr = address.Get();

  NS_ABORT_MSG_UNLESS(addr, "Ipv4AddressGeneratorImpl::Add(): Allocating the "
                            "broadcast address is not a good idea");

  std::list<Entry>::iterator i;

  for (i = m_entries.begin(); i != m_entries.end(); ++i) {
    NS_LOG_LOGIC("examine entry: " << Ipv4Address((*i).addrLow) << " to "
                                   << Ipv4Address((*i).addrHigh));
    if (addr >= (*i).addrLow && addr <= (*i).addrHigh) {
      NS_LOG_LOGIC("Ipv4AddressGeneratorImpl::Add(): Address Collision: "
                   << Ipv4Address(addr));
      if (!m_test) {
        NS_FATAL_ERROR("Ipv4AddressGeneratorImpl::Add(): Address Collision: "
                       << Ipv4Address(addr));
      }
      return false;
    }
    if (addr < (*i).addrLow - 1) {
      break;
    }
    if (addr == (*i).addrHigh + 1) {
      auto j = i;
      ++j;

      if (j != m_entries.end()) {
        if (addr == (*j).addrLow) {
          NS_LOG_LOGIC("Ipv4AddressGeneratorImpl::Add(): "
                       "Address Collision: "
                       << Ipv4Address(addr));
          if (!m_test) {
            NS_FATAL_ERROR(
                "Ipv4AddressGeneratorImpl::Add(): Address Collision: "
                << Ipv4Address(addr));
          }
          return false;
        }
      }

      NS_LOG_LOGIC("New addrHigh = " << Ipv4Address(addr));
      (*i).addrHigh = addr;
      return true;
    }
    if (addr == (*i).addrLow - 1) {
      NS_LOG_LOGIC("New addrLow = " << Ipv4Address(addr));
      (*i).addrLow = addr;
      return true;
    }
  }

  Entry entry;
  entry.addrLow = entry.addrHigh = addr;
  m_entries.insert(i, entry);
  return true;
}

bool Ipv4AddressGeneratorImpl::IsAddressAllocated(const Ipv4Address address) {
  NS_LOG_FUNCTION(this << address);

  uint32_t addr = address.Get();

  NS_ABORT_MSG_UNLESS(addr, "Ipv4AddressGeneratorImpl::IsAddressAllocated(): "
                            "Don't check for the broadcast address...");

  for (auto i = m_entries.begin(); i != m_entries.end(); ++i) {
    NS_LOG_LOGIC("examine entry: " << Ipv4Address((*i).addrLow) << " to "
                                   << Ipv4Address((*i).addrHigh));
    if (addr >= (*i).addrLow && addr <= (*i).addrHigh) {
      NS_LOG_LOGIC(
          "Ipv4AddressGeneratorImpl::IsAddressAllocated(): Address Collision: "
          << Ipv4Address(addr));
      return true;
    }
  }
  return false;
}

bool Ipv4AddressGeneratorImpl::IsNetworkAllocated(const Ipv4Address address,
                                                  const Ipv4Mask mask) {
  NS_LOG_FUNCTION(this << address << mask);

  NS_ABORT_MSG_UNLESS(address == address.CombineMask(mask),
                      "Ipv4AddressGeneratorImpl::IsNetworkAllocated(): network "
                      "address and mask don't match "
                          << address << " " << mask);

  for (auto i = m_entries.begin(); i != m_entries.end(); ++i) {
    NS_LOG_LOGIC("examine entry: " << Ipv4Address((*i).addrLow) << " to "
                                   << Ipv4Address((*i).addrHigh));
    Ipv4Address low((*i).addrLow);
    Ipv4Address high((*i).addrHigh);

    if (address == low.CombineMask(mask) || address == high.CombineMask(mask)) {
      NS_LOG_LOGIC("Ipv4AddressGeneratorImpl::IsNetworkAllocated(): Network "
                   "already allocated: "
                   << address << " " << low << "-" << high);
      return false;
    }
  }
  return true;
}

void Ipv4AddressGeneratorImpl::TestMode() {
  NS_LOG_FUNCTION(this);
  m_test = true;
}

uint32_t Ipv4AddressGeneratorImpl::MaskToIndex(Ipv4Mask mask) const {
  NS_LOG_FUNCTION(this << mask);

  uint32_t maskBits = mask.Get();

  for (uint32_t i = 0; i < N_BITS; ++i) {
    if (maskBits & 1) {
      uint32_t index = N_BITS - i;
      NS_ABORT_MSG_UNLESS(index > 0 && index < N_BITS,
                          "Ipv4AddressGenerator::MaskToIndex(): Illegal Mask");
      return index;
    }
    maskBits >>= 1;
  }
  NS_ASSERT_MSG(false, "Ipv4AddressGenerator::MaskToIndex(): Impossible");
  return 0;
}

void Ipv4AddressGenerator::Init(const Ipv4Address net, const Ipv4Mask mask,
                                const Ipv4Address addr) {
  NS_LOG_FUNCTION(net << mask << addr);

  SimulationSingleton<Ipv4AddressGeneratorImpl>::Get()->Init(net, mask, addr);
}

Ipv4Address Ipv4AddressGenerator::NextNetwork(const Ipv4Mask mask) {
  NS_LOG_FUNCTION(mask);

  return SimulationSingleton<Ipv4AddressGeneratorImpl>::Get()->NextNetwork(
      mask);
}

Ipv4Address Ipv4AddressGenerator::GetNetwork(const Ipv4Mask mask) {
  NS_LOG_FUNCTION(mask);

  return SimulationSingleton<Ipv4AddressGeneratorImpl>::Get()->GetNetwork(mask);
}

void Ipv4AddressGenerator::InitAddress(const Ipv4Address addr,
                                       const Ipv4Mask mask) {
  NS_LOG_FUNCTION(addr << mask);

  SimulationSingleton<Ipv4AddressGeneratorImpl>::Get()->InitAddress(addr, mask);
}

Ipv4Address Ipv4AddressGenerator::GetAddress(const Ipv4Mask mask) {
  NS_LOG_FUNCTION(mask);

  return SimulationSingleton<Ipv4AddressGeneratorImpl>::Get()->GetAddress(mask);
}

Ipv4Address Ipv4AddressGenerator::NextAddress(const Ipv4Mask mask) {
  NS_LOG_FUNCTION(mask);

  return SimulationSingleton<Ipv4AddressGeneratorImpl>::Get()->NextAddress(
      mask);
}

void Ipv4AddressGenerator::Reset() {
  NS_LOG_FUNCTION_NOARGS();

  return SimulationSingleton<Ipv4AddressGeneratorImpl>::Get()->Reset();
}

bool Ipv4AddressGenerator::AddAllocated(const Ipv4Address addr) {
  NS_LOG_FUNCTION(addr);

  return SimulationSingleton<Ipv4AddressGeneratorImpl>::Get()->AddAllocated(
      addr);
}

bool Ipv4AddressGenerator::IsAddressAllocated(const Ipv4Address addr) {
  NS_LOG_FUNCTION(addr);

  return SimulationSingleton<Ipv4AddressGeneratorImpl>::Get()
      ->IsAddressAllocated(addr);
}

bool Ipv4AddressGenerator::IsNetworkAllocated(const Ipv4Address addr,
                                              const Ipv4Mask mask) {
  NS_LOG_FUNCTION(addr << mask);

  return SimulationSingleton<Ipv4AddressGeneratorImpl>::Get()
      ->IsNetworkAllocated(addr, mask);
}

void Ipv4AddressGenerator::TestMode() {
  NS_LOG_FUNCTION_NOARGS();

  SimulationSingleton<Ipv4AddressGeneratorImpl>::Get()->TestMode();
}

} // namespace ns3
