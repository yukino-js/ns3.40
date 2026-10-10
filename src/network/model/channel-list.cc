
#include "channel-list.h"

#include "channel.h"

#include "ns3/assert.h"
#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/object-vector.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("ChannelList");

class ChannelListPriv : public Object {
public:
  static TypeId GetTypeId();
  ChannelListPriv();
  ~ChannelListPriv() override;

  uint32_t Add(Ptr<Channel> channel);

  ChannelList::Iterator Begin() const;
  ChannelList::Iterator End() const;

  Ptr<Channel> GetChannel(uint32_t n);

  uint32_t GetNChannels();

  static Ptr<ChannelListPriv> Get();

private:
  static Ptr<ChannelListPriv> *DoGet();

  static void Delete();

  void DoDispose() override;

  std::vector<Ptr<Channel>> m_channels;
};

NS_OBJECT_ENSURE_REGISTERED(ChannelListPriv);

TypeId ChannelListPriv::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::ChannelListPriv")
          .SetParent<Object>()
          .SetGroupName("Network")
          .AddAttribute(
              "ChannelList",
              "The list of all channels created during the simulation.",
              ObjectVectorValue(),
              MakeObjectVectorAccessor(&ChannelListPriv::m_channels),
              MakeObjectVectorChecker<Channel>());
  return tid;
}

Ptr<ChannelListPriv> ChannelListPriv::Get() {
  NS_LOG_FUNCTION_NOARGS();
  return *DoGet();
}

Ptr<ChannelListPriv> *ChannelListPriv::DoGet() {
  NS_LOG_FUNCTION_NOARGS();
  static Ptr<ChannelListPriv> ptr = nullptr;
  if (!ptr) {
    ptr = CreateObject<ChannelListPriv>();
    Config::RegisterRootNamespaceObject(ptr);
    Simulator::ScheduleDestroy(&ChannelListPriv::Delete);
  }
  return &ptr;
}

void ChannelListPriv::Delete() {
  NS_LOG_FUNCTION_NOARGS();
  Config::UnregisterRootNamespaceObject(Get());
  (*DoGet()) = nullptr;
}

ChannelListPriv::ChannelListPriv() { NS_LOG_FUNCTION(this); }

ChannelListPriv::~ChannelListPriv() { NS_LOG_FUNCTION(this); }

void ChannelListPriv::DoDispose() {
  NS_LOG_FUNCTION(this);
  for (auto i = m_channels.begin(); i != m_channels.end(); i++) {
    Ptr<Channel> channel = *i;
    channel->Dispose();
    *i = nullptr;
  }
  m_channels.erase(m_channels.begin(), m_channels.end());
  Object::DoDispose();
}

uint32_t ChannelListPriv::Add(Ptr<Channel> channel) {
  NS_LOG_FUNCTION(this << channel);
  uint32_t index = m_channels.size();
  m_channels.push_back(channel);
  return index;
}

ChannelList::Iterator ChannelListPriv::Begin() const {
  NS_LOG_FUNCTION(this);
  return m_channels.begin();
}

ChannelList::Iterator ChannelListPriv::End() const {
  NS_LOG_FUNCTION(this);
  return m_channels.end();
}

uint32_t ChannelListPriv::GetNChannels() {
  NS_LOG_FUNCTION(this);
  return m_channels.size();
}

Ptr<Channel> ChannelListPriv::GetChannel(uint32_t n) {
  NS_LOG_FUNCTION(this << n);
  NS_ASSERT_MSG(n < m_channels.size(),
                "Channel index " << n << " is out of range (only have "
                                 << m_channels.size() << " channels).");
  return m_channels[n];
}

uint32_t ChannelList::Add(Ptr<Channel> channel) {
  NS_LOG_FUNCTION_NOARGS();
  return ChannelListPriv::Get()->Add(channel);
}

ChannelList::Iterator ChannelList::Begin() {
  NS_LOG_FUNCTION_NOARGS();
  return ChannelListPriv::Get()->Begin();
}

ChannelList::Iterator ChannelList::End() {
  NS_LOG_FUNCTION_NOARGS();
  return ChannelListPriv::Get()->End();
}

Ptr<Channel> ChannelList::GetChannel(uint32_t n) {
  NS_LOG_FUNCTION(n);
  return ChannelListPriv::Get()->GetChannel(n);
}

uint32_t ChannelList::GetNChannels() {
  NS_LOG_FUNCTION_NOARGS();
  return ChannelListPriv::Get()->GetNChannels();
}

} // namespace ns3
