

#ifndef NS3_REMOTE_CHANNEL_BUNDLE
#define NS3_REMOTE_CHANNEL_BUNDLE

#include "null-message-simulator-impl.h"

#include <ns3/channel.h>
#include <ns3/pointer.h>
#include <ns3/ptr.h>

#include <unordered_map>

namespace ns3 {

class RemoteChannelBundle : public Object {
public:
  static TypeId GetTypeId();

  RemoteChannelBundle();

  RemoteChannelBundle(const uint32_t remoteSystemId);

  ~RemoteChannelBundle() override {}

  void AddChannel(Ptr<Channel> channel, Time delay);

  uint32_t GetSystemId() const;

  Time GetGuaranteeTime() const;

  void SetGuaranteeTime(Time time);

  Time GetDelay() const;

  void SetEventId(EventId id);

  EventId GetEventId() const;

  std::size_t GetSize() const;

  void Send(Time time);

  friend std::ostream &operator<<(std::ostream &out,
                                  ns3::RemoteChannelBundle &bundle);

private:
  uint32_t m_remoteSystemId;

  typedef std::unordered_map<uint32_t, Ptr<Channel>> ChannelMap;
  ChannelMap m_channels;

  Time m_guaranteeTime;

  Time m_delay;

  EventId m_nullEventId;
};

} // namespace ns3

#endif
