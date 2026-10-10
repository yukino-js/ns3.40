
#ifndef GENERIC_PHY_H
#define GENERIC_PHY_H

#include <ns3/callback.h>

namespace ns3 {

class Packet;

typedef Callback<bool, Ptr<Packet>> GenericPhyTxStartCallback;

typedef Callback<void, Ptr<const Packet>> GenericPhyTxEndCallback;

typedef Callback<void> GenericPhyRxStartCallback;

typedef Callback<void> GenericPhyRxEndErrorCallback;

typedef Callback<void, Ptr<Packet>> GenericPhyRxEndOkCallback;

} // namespace ns3

#endif
