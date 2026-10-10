
#ifndef SPECTRUM_HELPER_H
#define SPECTRUM_HELPER_H

#include <ns3/attribute.h>
#include <ns3/net-device-container.h>
#include <ns3/node-container.h>
#include <ns3/object-factory.h>
#include <ns3/propagation-loss-model.h>
#include <ns3/spectrum-propagation-loss-model.h>

#include <string>

namespace ns3 {

class SpectrumPhy;
class SpectrumChannel;
class Node;
class NetDevice;

class SpectrumChannelHelper {
public:
  static SpectrumChannelHelper Default();

  template <typename... Ts> void SetChannel(std::string type, Ts &&...args);
  template <typename... Ts>
  void AddPropagationLoss(std::string name, Ts &&...args);

  void AddPropagationLoss(Ptr<PropagationLossModel> m);

  template <typename... Ts>
  void AddSpectrumPropagationLoss(std::string name, Ts &&...args);

  void AddSpectrumPropagationLoss(Ptr<SpectrumPropagationLossModel> m);

  template <typename... Ts>
  void SetPropagationDelay(std::string name, Ts &&...args);

  Ptr<SpectrumChannel> Create() const;

private:
  Ptr<SpectrumPropagationLossModel> m_spectrumPropagationLossModel;
  Ptr<PropagationLossModel> m_propagationLossModel;
  ObjectFactory m_propagationDelay;
  ObjectFactory m_channel;
};

class SpectrumPhyHelper {
public:
  template <typename... Ts> void SetPhy(std::string name, Ts &&...args);

  void SetChannel(Ptr<SpectrumChannel> channel);

  void SetChannel(std::string channelName);

  void SetPhyAttribute(std::string name, const AttributeValue &v);

  Ptr<SpectrumPhy> Create(Ptr<Node> node, Ptr<NetDevice> device) const;

private:
  ObjectFactory m_phy;
  Ptr<SpectrumChannel> m_channel;
};

template <typename... Ts>
void SpectrumChannelHelper::SetChannel(std::string type, Ts &&...args) {
  m_channel.SetTypeId(type);
  m_channel.Set(std::forward<Ts>(args)...);
}

template <typename... Ts>
void SpectrumChannelHelper::AddPropagationLoss(std::string name, Ts &&...args) {
  ObjectFactory factory(name, std::forward<Ts>(args)...);
  Ptr<PropagationLossModel> m = factory.Create<PropagationLossModel>();
  AddPropagationLoss(m);
}

template <typename... Ts>
void SpectrumChannelHelper::AddSpectrumPropagationLoss(std::string name,
                                                       Ts &&...args) {
  ObjectFactory factory(name, std::forward<Ts>(args)...);
  Ptr<SpectrumPropagationLossModel> m =
      factory.Create<SpectrumPropagationLossModel>();
  AddSpectrumPropagationLoss(m);
}

template <typename... Ts>
void SpectrumChannelHelper::SetPropagationDelay(std::string name,
                                                Ts &&...args) {
  m_propagationDelay = ObjectFactory(name, std::forward<Ts>(args)...);
}

template <typename... Ts>
void SpectrumPhyHelper::SetPhy(std::string name, Ts &&...args) {
  m_phy.SetTypeId(name);
  m_phy.Set(std::forward<Ts>(args)...);
}

} // namespace ns3

#endif
