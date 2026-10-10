
#ifndef WIMAX_HELPER_H
#define WIMAX_HELPER_H

#include "ns3/bs-net-device.h"
#include "ns3/bs-scheduler-rtps.h"
#include "ns3/bs-scheduler-simple.h"
#include "ns3/bs-scheduler.h"
#include "ns3/bs-uplink-scheduler-mbqos.h"
#include "ns3/bs-uplink-scheduler-rtps.h"
#include "ns3/bs-uplink-scheduler-simple.h"
#include "ns3/bs-uplink-scheduler.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/service-flow.h"
#include "ns3/simple-ofdm-wimax-channel.h"
#include "ns3/ss-net-device.h"
#include "ns3/trace-helper.h"

#include <string>

namespace ns3 {

class WimaxChannel;
class WimaxPhy;
class UplinkScheduler;

class WimaxHelper : public PcapHelperForDevice,
                    public AsciiTraceHelperForDevice {
public:
  enum NetDeviceType {
    DEVICE_TYPE_SUBSCRIBER_STATION,
    DEVICE_TYPE_BASE_STATION
  };

  enum PhyType { SIMPLE_PHY_TYPE_OFDM };

  enum SchedulerType { SCHED_TYPE_SIMPLE, SCHED_TYPE_RTPS, SCHED_TYPE_MBQOS };

  WimaxHelper();
  ~WimaxHelper() override;
  static void EnableAsciiForConnection(Ptr<OutputStreamWrapper> oss,
                                       uint32_t nodeid, uint32_t deviceid,
                                       char *netdevice, char *connection);

  Ptr<WimaxPhy> CreatePhy(PhyType phyType);

  Ptr<UplinkScheduler> CreateUplinkScheduler(SchedulerType schedulerType);

  Ptr<BSScheduler> CreateBSScheduler(SchedulerType schedulerType);

  NetDeviceContainer Install(NodeContainer c, NetDeviceType type,
                             PhyType phyType, SchedulerType schedulerType);

  NetDeviceContainer Install(NodeContainer c, NetDeviceType deviceType,
                             PhyType phyType, Ptr<WimaxChannel> channel,
                             SchedulerType schedulerType);
  NetDeviceContainer Install(NodeContainer c, NetDeviceType deviceType,
                             PhyType phyType, SchedulerType schedulerType,
                             double frameDuration);

  void
  SetPropagationLossModel(SimpleOfdmWimaxChannel::PropModel propagationModel);

  Ptr<WimaxPhy> CreatePhyWithoutChannel(PhyType phyType);

  Ptr<WimaxPhy> CreatePhyWithoutChannel(PhyType phyType, char *SNRTraceFilePath,
                                        bool activateLoss);

  Ptr<WimaxPhy> CreatePhy(PhyType phyType, char *SNRTraceFilePath,
                          bool activateLoss);
  Ptr<WimaxNetDevice> Install(Ptr<Node> node, NetDeviceType deviceType,
                              PhyType phyType, Ptr<WimaxChannel> channel,
                              SchedulerType schedulerType);

  ServiceFlow CreateServiceFlow(ServiceFlow::Direction direction,
                                ServiceFlow::SchedulingType schedulinType,
                                IpcsClassifierRecord classifier);

  static void EnableLogComponents();

  int64_t AssignStreams(int64_t stream);

  int64_t AssignStreams(NetDeviceContainer c, int64_t stream);

private:
  static void AsciiRxEvent(Ptr<OutputStreamWrapper> stream, std::string path,
                           Ptr<const Packet> packet,
                           const Mac48Address &source);
  static void AsciiTxEvent(Ptr<OutputStreamWrapper> stream, std::string path,
                           Ptr<const Packet> packet, const Mac48Address &dest);
  void EnablePcapInternal(std::string prefix, Ptr<NetDevice> nd,
                          bool explicitFilename, bool promiscuous) override;

  void EnableAsciiInternal(Ptr<OutputStreamWrapper> stream, std::string prefix,
                           Ptr<NetDevice> nd, bool explicitFilename) override;

  Ptr<WimaxChannel> m_channel;
};

} // namespace ns3

#endif
