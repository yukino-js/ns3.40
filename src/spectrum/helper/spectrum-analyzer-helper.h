
#ifndef SPECTRUM_ANALYZER_HELPER_H
#define SPECTRUM_ANALYZER_HELPER_H

#include <ns3/attribute.h>
#include <ns3/net-device-container.h>
#include <ns3/node-container.h>
#include <ns3/object-factory.h>
#include <ns3/queue.h>

#include <string>

namespace ns3 {

class SpectrumValue;
class SpectrumChannel;
class SpectrumModel;

class SpectrumAnalyzerHelper {
public:
  SpectrumAnalyzerHelper();
  ~SpectrumAnalyzerHelper();

  void SetChannel(Ptr<SpectrumChannel> channel);

  void SetChannel(std::string channelName);

  void SetPhyAttribute(std::string name, const AttributeValue &v);

  void SetDeviceAttribute(std::string n1, const AttributeValue &v1);

  template <typename... Ts> void SetAntenna(std::string type, Ts &&...args);

  void SetRxSpectrumModel(Ptr<SpectrumModel> m);

  void EnableAsciiAll(std::string prefix);

  NetDeviceContainer Install(NodeContainer c) const;
  NetDeviceContainer Install(Ptr<Node> node) const;
  NetDeviceContainer Install(std::string nodeName) const;

private:
  ObjectFactory m_phy;
  ObjectFactory m_device;
  ObjectFactory m_antenna;

  Ptr<SpectrumChannel> m_channel;
  Ptr<SpectrumModel> m_rxSpectrumModel;
  std::string m_prefix;
};

template <typename... Ts>
void SpectrumAnalyzerHelper::SetAntenna(std::string type, Ts &&...args) {
  m_antenna = ObjectFactory(type, std::forward<Ts>(args)...);
}

} // namespace ns3

#endif
