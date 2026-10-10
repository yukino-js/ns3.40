
#ifndef QUEUE_FWD_H
#define QUEUE_FWD_H

#include "ns3/ptr.h"

#include <list>

namespace ns3 {

template <typename Item, typename Container = std::list<Ptr<Item>>> class Queue;

}

#endif
