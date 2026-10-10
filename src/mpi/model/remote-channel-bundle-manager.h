

#ifndef NS3_REMOTE_CHANNEL_BUNDLE_MANAGER
#define NS3_REMOTE_CHANNEL_BUNDLE_MANAGER

#include <ns3/nstime.h>
#include <ns3/ptr.h>

#include <unordered_map>

namespace ns3 {

class RemoteChannelBundle;

class RemoteChannelBundleManager {
public:
  static Ptr<RemoteChannelBundle> Find(uint32_t systemId);

  static Ptr<RemoteChannelBundle> Add(uint32_t systemId);

  static std::size_t Size();

  static void InitializeNullMessageEvents();

  static Time GetSafeTime();

  static void Destroy();

private:
  RemoteChannelBundleManager() {}

  ~RemoteChannelBundleManager() {}

  typedef std::unordered_map<uint32_t, Ptr<RemoteChannelBundle>>
      RemoteChannelMap;
  static RemoteChannelMap g_remoteChannelBundles;

  static bool g_initialized;
};

} // namespace ns3

#endif
