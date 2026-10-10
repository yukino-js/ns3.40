
#include <ns3/adhoc-aloha-noack-ideal-phy-helper.h>
#include <ns3/applications-module.h>
#include <ns3/core-module.h>
#include <ns3/friis-spectrum-propagation-loss.h>
#include <ns3/ism-spectrum-value-helper.h>
#include <ns3/log.h>
#include <ns3/mobility-module.h>
#include <ns3/network-module.h>
#include <ns3/propagation-delay-model.h>
#include <ns3/single-model-spectrum-channel.h>
#include <ns3/spectrum-analyzer.h>
#include <ns3/spectrum-helper.h>
#include <ns3/spectrum-model-300kHz-300GHz-log.h>
#include <ns3/spectrum-model-ism2400MHz-res1MHz.h>
#include <ns3/waveform-generator.h>

#include <iostream>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TestAdhocOfdmAloha");

static bool g_verbose = false;

void PhyTxStartTrace(std::string context, Ptr<const Packet> p) {
  if (g_verbose) {
    std::cout << context << " PHY TX START p: " << p << std::endl;
  }
}

void PhyTxEndTrace(std::string context, Ptr<const Packet> p) {
  if (g_verbose) {
    std::cout << context << " PHY TX END p: " << p << std::endl;
  }
}

void PhyRxStartTrace(std::string context, Ptr<const Packet> p) {
  if (g_verbose) {
    std::cout << context << " PHY RX START p:" << p << std::endl;
  }
}

void PhyRxEndOkTrace(std::string context, Ptr<const Packet> p) {
  if (g_verbose) {
    std::cout << context << " PHY RX END OK p:" << p << std::endl;
  }
}

void PhyRxEndErrorTrace(std::string context, Ptr<const Packet> p) {
  if (g_verbose) {
    std::cout << context << " PHY RX END ERROR p:" << p << std::endl;
  }
}

void ReceivePacket(Ptr<Socket> socket) {
  Ptr<Packet> packet;
  uint64_t bytes = 0;
  while ((packet = socket->Recv())) {
    bytes += packet->GetSize();
  }
  if (g_verbose) {
    std::cout << "SOCKET received " << bytes << " bytes" << std::endl;
  }
}

Ptr<Socket> SetupPacketReceive(Ptr<Node> node) {
  TypeId tid = TypeId::LookupByName("ns3::PacketSocketFactory");
  Ptr<Socket> sink = Socket::CreateSocket(node, tid);
  sink->Bind();
  sink->SetRecvCallback(MakeCallback(&ReceivePacket));
  return sink;
}

int main(int argc, char **argv) {
  CommandLine cmd(__FILE__);
  cmd.AddValue("verbose", "Print trace information if true", g_verbose);
  cmd.Parse(argc, argv);

  NodeContainer c;
  c.Create(2);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();
  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  positionAlloc->Add(Vector(5.0, 0.0, 0.0));
  mobility.SetPositionAllocator(positionAlloc);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

  mobility.Install(c);

  SpectrumChannelHelper channelHelper = SpectrumChannelHelper::Default();
  Ptr<SpectrumChannel> channel = channelHelper.Create();

  SpectrumValue5MhzFactory sf;

  double txPower = 0.1;
  uint32_t channelNumber = 1;
  Ptr<SpectrumValue> txPsd =
      sf.CreateTxPowerSpectralDensity(txPower, channelNumber);

  const double k = 1.381e-23;
  const double T = 290;
  double noisePsdValue = k * T;
  Ptr<SpectrumValue> noisePsd = sf.CreateConstant(noisePsdValue);

  AdhocAlohaNoackIdealPhyHelper deviceHelper;
  deviceHelper.SetChannel(channel);
  deviceHelper.SetTxPowerSpectralDensity(txPsd);
  deviceHelper.SetNoisePowerSpectralDensity(noisePsd);
  deviceHelper.SetPhyAttribute("Rate", DataRateValue(DataRate("1Mbps")));
  NetDeviceContainer devices = deviceHelper.Install(c);

  PacketSocketHelper packetSocket;
  packetSocket.Install(c);

  PacketSocketAddress socket;
  socket.SetSingleDevice(devices.Get(0)->GetIfIndex());
  socket.SetPhysicalAddress(devices.Get(1)->GetAddress());
  socket.SetProtocol(1);

  OnOffHelper onoff("ns3::PacketSocketFactory", Address(socket));
  onoff.SetConstantRate(DataRate("0.5Mbps"));
  onoff.SetAttribute("PacketSize", UintegerValue(125));

  ApplicationContainer apps = onoff.Install(c.Get(0));
  apps.Start(Seconds(0.1));
  apps.Stop(Seconds(0.104));

  Ptr<Socket> recvSink = SetupPacketReceive(c.Get(1));

  Simulator::Stop(Seconds(10.0));

  Config::Connect("/NodeList/*/DeviceList/*/Phy/TxStart",
                  MakeCallback(&PhyTxStartTrace));
  Config::Connect("/NodeList/*/DeviceList/*/Phy/TxEnd",
                  MakeCallback(&PhyTxEndTrace));
  Config::Connect("/NodeList/*/DeviceList/*/Phy/RxStart",
                  MakeCallback(&PhyRxStartTrace));
  Config::Connect("/NodeList/*/DeviceList/*/Phy/RxEndOk",
                  MakeCallback(&PhyRxEndOkTrace));
  Config::Connect("/NodeList/*/DeviceList/*/Phy/RxEndError",
                  MakeCallback(&PhyRxEndErrorTrace));

  Simulator::Run();

  Simulator::Destroy();

  return 0;
}
