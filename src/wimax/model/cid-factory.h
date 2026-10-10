
#ifndef CONNECTION_IDENTIFIER_FACTORY_H
#define CONNECTION_IDENTIFIER_FACTORY_H

#include "cid.h"

#include <stdint.h>

namespace ns3 {

class CidFactory {
public:
  CidFactory();
  Cid AllocateBasic();
  Cid AllocatePrimary();
  Cid AllocateTransportOrSecondary();
  Cid AllocateMulticast();
  Cid Allocate(Cid::Type type);

  bool IsTransport(Cid cid) const;
  bool IsPrimary(Cid cid) const;
  bool IsBasic(Cid cid) const;

  void FreeCid(Cid cid);

private:
  uint16_t m_m;

  uint16_t m_basicIdentifier;
  uint16_t m_primaryIdentifier;
  uint16_t m_transportOrSecondaryIdentifier;
  uint16_t m_multicastPollingIdentifier;
};

} // namespace ns3

#endif
