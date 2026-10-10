#ifndef ADDRESS_UTILS_H
#define ADDRESS_UTILS_H

#include "ipv4-address.h"
#include "ipv6-address.h"
#include "mac16-address.h"
#include "mac48-address.h"
#include "mac64-address.h"

#include "ns3/address.h"
#include "ns3/buffer.h"

namespace ns3 {

void WriteTo(Buffer::Iterator &i, Ipv4Address ad);

void WriteTo(Buffer::Iterator &i, Ipv6Address ad);

void WriteTo(Buffer::Iterator &i, const Address &ad);

void WriteTo(Buffer::Iterator &i, Mac64Address ad);

void WriteTo(Buffer::Iterator &i, Mac48Address ad);

void WriteTo(Buffer::Iterator &i, Mac16Address ad);

void ReadFrom(Buffer::Iterator &i, Ipv4Address &ad);

void ReadFrom(Buffer::Iterator &i, Ipv6Address &ad);

void ReadFrom(Buffer::Iterator &i, Address &ad, uint32_t len);

void ReadFrom(Buffer::Iterator &i, Mac64Address &ad);

void ReadFrom(Buffer::Iterator &i, Mac48Address &ad);

void ReadFrom(Buffer::Iterator &i, Mac16Address &ad);

namespace addressUtils {

bool IsMulticast(const Address &ad);
};

}; // namespace ns3

#endif
