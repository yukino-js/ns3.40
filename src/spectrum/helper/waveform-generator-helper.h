
#ifndef WAVEFORM_GENERATOR_HELPER_H
#define WAVEFORM_GENERATOR_HELPER_H

#include <ns3/attribute.h>
#include <ns3/net-device-container.h>
#include <ns3/node-container.h>
#include <ns3/object-factory.h>
#include <ns3/queue.h>

#include <string>

namespace ns3 {

class SpectrumValue;
class SpectrumChannel;

class WaveformGeneratorHelper {
public:
  WaveformGeneratorHelper();
  ~WaveformGeneratorHelper();

  void SetChannel(Ptr<SpectrumChannel> channel);

  void SetChannel(std::string channelName);

  void SetTxPowerSpectralDensity(Ptr<SpectrumValue> txPsd);

  void SetPhyAttribute(std::string name, const AttributeValue &v);

  void SetDeviceAttribute(std::string n1, const AttributeValue &v1);

  template <typename... Ts> void SetAntenna(std::string type, Ts &&...args);

  NetDeviceContainer Install(NodeContainer c) const;
  NetDeviceContainer Install(Ptr<Node> node) const;
  NetDeviceContainer Install(std::string nodeName) const;

protected:
  ObjectFactory m_phy;
  ObjectFactory m_device;
  ObjectFactory m_antenna;
  Ptr<SpectrumChannel> m_channel;
  Ptr<SpectrumValue> m_txPsd;
};

template <typename... Ts>
void WaveformGeneratorHelper::SetAntenna(std::string type, Ts &&...args) {
  m_antenna = ObjectFactory(type, std::forward<Ts>(args)...);
}

} // namespace ns3

#endif
