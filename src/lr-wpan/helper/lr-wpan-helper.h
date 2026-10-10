#ifndef LR_WPAN_HELPER_H
#define LR_WPAN_HELPER_H

#include <ns3/lr-wpan-mac.h>
#include <ns3/lr-wpan-phy.h>
#include <ns3/node-container.h>
#include <ns3/trace-helper.h>

namespace ns3 {

class SpectrumChannel;
class MobilityModel;

class LrWpanHelper : public PcapHelperForDevice,
                     public AsciiTraceHelperForDevice {
public:
  LrWpanHelper();

  LrWpanHelper(bool useMultiModelSpectrumChannel);

  ~LrWpanHelper() override;

  LrWpanHelper(const LrWpanHelper &) = delete;
  LrWpanHelper &operator=(const LrWpanHelper &) = delete;

  Ptr<SpectrumChannel> GetChannel();

  void SetChannel(Ptr<SpectrumChannel> channel);

  void SetChannel(std::string channelName);

  void AddMobility(Ptr<LrWpanPhy> phy, Ptr<MobilityModel> m);

  NetDeviceContainer Install(NodeContainer c);

  void CreateAssociatedPan(NetDeviceContainer c, uint16_t panId);

  void SetExtendedAddresses(NetDeviceContainer c);

  void EnableLogComponents();

  static std::string LrWpanPhyEnumerationPrinter(LrWpanPhyEnumeration e);

  static std::string LrWpanMacStatePrinter(LrWpanMacState e);

  int64_t AssignStreams(NetDeviceContainer c, int64_t stream);

private:
  void EnablePcapInternal(std::string prefix, Ptr<NetDevice> nd,
                          bool promiscuous, bool explicitFilename) override;

  void EnableAsciiInternal(Ptr<OutputStreamWrapper> stream, std::string prefix,
                           Ptr<NetDevice> nd, bool explicitFilename) override;

private:
  Ptr<SpectrumChannel> m_channel;
};

} // namespace ns3

#endif
