

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SimpleErrorModelExample");

int main(int argc, char *argv[]) {
#if 0
  LogComponentEnable ("SimplePointToPointExample", LOG_LEVEL_INFO);
#endif

  Config::SetDefault("ns3::RateErrorModel::ErrorRate", DoubleValue(0.001));
  Config::SetDefault("ns3::RateErrorModel::ErrorUnit",
                     StringValue("ERROR_UNIT_PACKET"));

  Config::SetDefault("ns3::BurstErrorModel::ErrorRate", DoubleValue(0.01));
  Config::SetDefault("ns3::BurstErrorModel::BurstSize",
                     StringValue("ns3::UniformRandomVariable[Min=1|Max=3]"));

  Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(210));
  Config::SetDefault("ns3::OnOffApplication::DataRate",
                     DataRateValue(DataRate("448kb/s")));

  std::string errorModelType = "ns3::RateErrorModel";

  CommandLine cmd(__FILE__);
  cmd.AddValue("errorModelType", "TypeId of the error model to use",
               errorModelType);
  cmd.Parse(argc, argv);

  NS_LOG_INFO("Create nodes.");
  NodeContainer c;
  c.Create(4);
  NodeContainer n0n2 = NodeContainer(c.Get(0), c.Get(2));
  NodeContainer n1n2 = NodeContainer(c.Get(1), c.Get(2));
  NodeContainer n3n2 = NodeContainer(c.Get(3), c.Get(2));

  InternetStackHelper internet;
  internet.Install(c);

  NS_LOG_INFO("Create channels.");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", DataRateValue(DataRate(5000000)));
  p2p.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));
  NetDeviceContainer d0d2 = p2p.Install(n0n2);

  NetDeviceContainer d1d2 = p2p.Install(n1n2);

  p2p.SetDeviceAttribute("DataRate", DataRateValue(DataRate(1500000)));
  p2p.SetChannelAttribute("Delay", TimeValue(MilliSeconds(10)));
  NetDeviceContainer d3d2 = p2p.Install(n3n2);

  NS_LOG_INFO("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  ipv4.Assign(d0d2);

  ipv4.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i1i2 = ipv4.Assign(d1d2);

  ipv4.SetBase("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i3i2 = ipv4.Assign(d3d2);

  NS_LOG_INFO("Use global routing.");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  NS_LOG_INFO("Create Applications.");
  uint16_t port = 9;

  OnOffHelper onoff("ns3::UdpSocketFactory",
                    Address(InetSocketAddress(i3i2.GetAddress(1), port)));
  onoff.SetConstantRate(DataRate("448kb/s"));
  ApplicationContainer apps = onoff.Install(c.Get(0));
  apps.Start(Seconds(1.0));
  apps.Stop(Seconds(10.0));

  PacketSinkHelper sink(
      "ns3::UdpSocketFactory",
      Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
  apps = sink.Install(c.Get(2));
  apps.Start(Seconds(1.0));
  apps.Stop(Seconds(10.0));

  onoff.SetAttribute("Remote",
                     AddressValue(InetSocketAddress(i1i2.GetAddress(0), port)));
  apps = onoff.Install(c.Get(3));
  apps.Start(Seconds(1.1));
  apps.Stop(Seconds(10.0));

  sink.SetAttribute(
      "Local", AddressValue(InetSocketAddress(Ipv4Address::GetAny(), port)));
  apps = sink.Install(c.Get(1));
  apps.Start(Seconds(1.1));
  apps.Stop(Seconds(10.0));

  ObjectFactory factory;
  factory.SetTypeId(errorModelType);
  Ptr<ErrorModel> em = factory.Create<ErrorModel>();
  d3d2.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(em));

  std::list<uint64_t> sampleList;
  sampleList.push_back(11);
  sampleList.push_back(17);
  Ptr<ListErrorModel> pem = CreateObject<ListErrorModel>();
  pem->SetList(sampleList);
  d0d2.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(pem));

  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll(ascii.CreateFileStream("simple-error-model.tr"));
  p2p.EnablePcapAll("simple-error-model");

  NS_LOG_INFO("Run Simulation.");
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");

  return 0;
}
