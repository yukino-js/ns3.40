
#ifndef YANS_WIFI_HELPER_H
#define YANS_WIFI_HELPER_H

#include "wifi-helper.h"

#include "ns3/yans-wifi-channel.h"

namespace ns3 {

class YansWifiChannelHelper {
public:
  YansWifiChannelHelper();

  static YansWifiChannelHelper Default();

  template <typename... Ts>
  void AddPropagationLoss(std::string name, Ts &&...args);
  template <typename... Ts>
  void SetPropagationDelay(std::string name, Ts &&...args);

  Ptr<YansWifiChannel> Create() const;

  int64_t AssignStreams(Ptr<YansWifiChannel> c, int64_t stream);

private:
  std::vector<ObjectFactory> m_propagationLoss;
  ObjectFactory m_propagationDelay;
};

class YansWifiPhyHelper : public WifiPhyHelper {
public:
  YansWifiPhyHelper();

  void SetChannel(Ptr<YansWifiChannel> channel);
  void SetChannel(std::string channelName);

private:
  std::vector<Ptr<WifiPhy>> Create(Ptr<Node> node,
                                   Ptr<WifiNetDevice> device) const override;

  Ptr<YansWifiChannel> m_channel;
};

template <typename... Ts>
void YansWifiChannelHelper::AddPropagationLoss(std::string name, Ts &&...args) {
  m_propagationLoss.push_back(ObjectFactory(name, std::forward<Ts>(args)...));
}

template <typename... Ts>
void YansWifiChannelHelper::SetPropagationDelay(std::string name,
                                                Ts &&...args) {
  m_propagationDelay = ObjectFactory(name, std::forward<Ts>(args)...);
}

} // namespace ns3

#endif
