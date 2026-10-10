
#ifndef CHANNEL_LIST_H
#define CHANNEL_LIST_H

#include "ns3/ptr.h"

#include <vector>

namespace ns3 {

class Channel;
class CallbackBase;

class ChannelList {
public:
  typedef std::vector<Ptr<Channel>>::const_iterator Iterator;

  static uint32_t Add(Ptr<Channel> channel);
  static Iterator Begin();
  static Iterator End();
  static Ptr<Channel> GetChannel(uint32_t n);
  static uint32_t GetNChannels();
};

} // namespace ns3

#endif
