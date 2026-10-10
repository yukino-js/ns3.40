
#ifndef WIFI_HELPER_H
#define WIFI_HELPER_H

#include "wifi-mac-helper.h"

#include "ns3/qos-utils.h"
#include "ns3/trace-helper.h"
#include "ns3/wifi-phy.h"

#include <functional>
#include <vector>

namespace ns3 {

class WifiNetDevice;
class Node;
class RadiotapHeader;
class QueueItem;

class WifiPhyHelper : public PcapHelperForDevice,
                      public AsciiTraceHelperForDevice {
public:
  WifiPhyHelper(uint8_t nLinks = 1);
  ~WifiPhyHelper() override;

  virtual std::vector<Ptr<WifiPhy>> Create(Ptr<Node> node,
                                           Ptr<WifiNetDevice> device) const = 0;

  void Set(std::string name, const AttributeValue &v);

  void Set(uint8_t linkId, std::string name, const AttributeValue &v);

  template <typename... Args>
  void SetInterferenceHelper(std::string type, Args &&...args);

  template <typename... Args>
  void SetErrorRateModel(std::string type, Args &&...args);

  template <typename... Args>
  void SetErrorRateModel(uint8_t linkId, std::string type, Args &&...args);

  template <typename... Args>
  void SetFrameCaptureModel(std::string type, Args &&...args);

  template <typename... Args>
  void SetFrameCaptureModel(uint8_t linkId, std::string type, Args &&...args);

  template <typename... Args>
  void SetPreambleDetectionModel(std::string type, Args &&...args);

  template <typename... Args>
  void SetPreambleDetectionModel(uint8_t linkId, std::string type,
                                 Args &&...args);

  void DisablePreambleDetectionModel();

  enum SupportedPcapDataLinkTypes {
    DLT_IEEE802_11 = PcapHelper::DLT_IEEE802_11,
    DLT_PRISM_HEADER = PcapHelper::DLT_PRISM_HEADER,
    DLT_IEEE802_11_RADIO = PcapHelper::DLT_IEEE802_11_RADIO
  };

  void SetPcapDataLinkType(SupportedPcapDataLinkTypes dlt);

  PcapHelper::DataLinkType GetPcapDataLinkType() const;

protected:
  static void PcapSniffTxEvent(Ptr<PcapFileWrapper> file,
                               Ptr<const Packet> packet,
                               uint16_t channelFreqMhz, WifiTxVector txVector,
                               MpduInfo aMpdu, uint16_t staId = SU_STA_ID);
  static void PcapSniffRxEvent(Ptr<PcapFileWrapper> file,
                               Ptr<const Packet> packet,
                               uint16_t channelFreqMhz, WifiTxVector txVector,
                               MpduInfo aMpdu, SignalNoiseDbm signalNoise,
                               uint16_t staId = SU_STA_ID);

  std::vector<ObjectFactory> m_phys;
  ObjectFactory m_interferenceHelper;
  std::vector<ObjectFactory> m_errorRateModel;
  std::vector<ObjectFactory> m_frameCaptureModel;
  std::vector<ObjectFactory> m_preambleDetectionModel;

private:
  static void GetRadiotapHeader(RadiotapHeader &header, Ptr<Packet> packet,
                                uint16_t channelFreqMhz, WifiTxVector txVector,
                                MpduInfo aMpdu, uint16_t staId);

  static void GetRadiotapHeader(RadiotapHeader &header, Ptr<Packet> packet,
                                uint16_t channelFreqMhz, WifiTxVector txVector,
                                MpduInfo aMpdu, uint16_t staId,
                                SignalNoiseDbm signalNoise);

  void EnablePcapInternal(std::string prefix, Ptr<NetDevice> nd,
                          bool promiscuous, bool explicitFilename) override;

  void EnableAsciiInternal(Ptr<OutputStreamWrapper> stream, std::string prefix,
                           Ptr<NetDevice> nd, bool explicitFilename) override;

  PcapHelper::DataLinkType m_pcapDlt;
};

class WifiHelper {
public:
  virtual ~WifiHelper();

  WifiHelper();

  template <typename... Args>
  void SetRemoteStationManager(std::string type, Args &&...args);

  template <typename... Args>
  void SetRemoteStationManager(uint8_t linkId, std::string type,
                               Args &&...args);

  template <typename... Args>
  void SetObssPdAlgorithm(std::string type, Args &&...args);

  typedef std::function<std::size_t(Ptr<QueueItem>)> SelectQueueCallback;

  void SetSelectQueueCallback(SelectQueueCallback f);

  void DisableFlowControl();

  NetDeviceContainer virtual Install(const WifiPhyHelper &phy,
                                     const WifiMacHelper &mac,
                                     NodeContainer::Iterator first,
                                     NodeContainer::Iterator last) const;
  virtual NetDeviceContainer Install(const WifiPhyHelper &phy,
                                     const WifiMacHelper &mac,
                                     NodeContainer c) const;
  virtual NetDeviceContainer Install(const WifiPhyHelper &phy,
                                     const WifiMacHelper &mac,
                                     Ptr<Node> node) const;
  virtual NetDeviceContainer Install(const WifiPhyHelper &phy,
                                     const WifiMacHelper &mac,
                                     std::string nodeName) const;
  virtual void SetStandard(WifiStandard standard);

  template <typename... Args> void ConfigHtOptions(Args &&...args);

  template <typename... Args> void ConfigVhtOptions(Args &&...args);

  template <typename... Args> void ConfigHeOptions(Args &&...args);

  template <typename... Args> void ConfigEhtOptions(Args &&...args);

  static void EnableLogComponents();

  int64_t AssignStreams(NetDeviceContainer c, int64_t stream);

protected:
  mutable std::vector<ObjectFactory> m_stationManager;
  WifiStandard m_standard;
  ObjectFactory m_htConfig;
  ObjectFactory m_vhtConfig;
  ObjectFactory m_heConfig;
  ObjectFactory m_ehtConfig;
  SelectQueueCallback m_selectQueueCallback;
  ObjectFactory m_obssPdAlgorithm;
  bool m_enableFlowControl;
};

} // namespace ns3

namespace ns3 {

template <typename... Args>
void WifiPhyHelper::SetInterferenceHelper(std::string type, Args &&...args) {
  m_interferenceHelper.SetTypeId(type);
  m_interferenceHelper.Set(args...);
}

template <typename... Args>
void WifiPhyHelper::SetErrorRateModel(std::string type, Args &&...args) {
  for (std::size_t linkId = 0; linkId < m_phys.size(); linkId++) {
    SetErrorRateModel(linkId, type, std::forward<Args>(args)...);
  }
}

template <typename... Args>
void WifiPhyHelper::SetErrorRateModel(uint8_t linkId, std::string type,
                                      Args &&...args) {
  m_errorRateModel.at(linkId).SetTypeId(type);
  m_errorRateModel.at(linkId).Set(args...);
}

template <typename... Args>
void WifiPhyHelper::SetFrameCaptureModel(std::string type, Args &&...args) {
  for (std::size_t linkId = 0; linkId < m_phys.size(); linkId++) {
    SetFrameCaptureModel(linkId, type, std::forward<Args>(args)...);
  }
}

template <typename... Args>
void WifiPhyHelper::SetFrameCaptureModel(uint8_t linkId, std::string type,
                                         Args &&...args) {
  m_frameCaptureModel.at(linkId).SetTypeId(type);
  m_frameCaptureModel.at(linkId).Set(args...);
}

template <typename... Args>
void WifiPhyHelper::SetPreambleDetectionModel(std::string type,
                                              Args &&...args) {
  for (std::size_t linkId = 0; linkId < m_phys.size(); linkId++) {
    SetPreambleDetectionModel(linkId, type, std::forward<Args>(args)...);
  }
}

template <typename... Args>
void WifiPhyHelper::SetPreambleDetectionModel(uint8_t linkId, std::string type,
                                              Args &&...args) {
  m_preambleDetectionModel.at(linkId).SetTypeId(type);
  m_preambleDetectionModel.at(linkId).Set(args...);
}

template <typename... Args>
void WifiHelper::SetRemoteStationManager(std::string type, Args &&...args) {
  SetRemoteStationManager(0, type, std::forward<Args>(args)...);
}

template <typename... Args>
void WifiHelper::SetRemoteStationManager(uint8_t linkId, std::string type,
                                         Args &&...args) {
  if (m_stationManager.size() > linkId) {
    m_stationManager[linkId] = ObjectFactory(type, std::forward<Args>(args)...);
  } else {
    m_stationManager.resize(linkId + 1,
                            ObjectFactory(type, std::forward<Args>(args)...));
  }
}

template <typename... Args>
void WifiHelper::SetObssPdAlgorithm(std::string type, Args &&...args) {
  m_obssPdAlgorithm.SetTypeId(type);
  m_obssPdAlgorithm.Set(args...);
}

template <typename... Args> void WifiHelper::ConfigHtOptions(Args &&...args) {
  m_htConfig.Set(args...);
}

template <typename... Args> void WifiHelper::ConfigVhtOptions(Args &&...args) {
  m_vhtConfig.Set(args...);
}

template <typename... Args> void WifiHelper::ConfigHeOptions(Args &&...args) {
  m_heConfig.Set(args...);
}

template <typename... Args> void WifiHelper::ConfigEhtOptions(Args &&...args) {
  m_ehtConfig.Set(args...);
}

} // namespace ns3

#endif
