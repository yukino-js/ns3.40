
#include "ns3/ap-wifi-mac.h"
#include "ns3/config.h"
#include "ns3/eht-configuration.h"
#include "ns3/frame-exchange-manager.h"
#include "ns3/log.h"
#include "ns3/mgt-headers.h"
#include "ns3/mobility-helper.h"
#include "ns3/multi-link-element.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/node-list.h"
#include "ns3/packet-socket-client.h"
#include "ns3/packet-socket-helper.h"
#include "ns3/packet-socket-server.h"
#include "ns3/packet.h"
#include "ns3/pointer.h"
#include "ns3/qos-utils.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/rr-multi-user-scheduler.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/spectrum-wifi-phy.h"
#include "ns3/sta-wifi-mac.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/wifi-acknowledgment.h"
#include "ns3/wifi-assoc-manager.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/wifi-mac-queue.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-protection.h"
#include "ns3/wifi-psdu.h"

#include <algorithm>
#include <array>
#include <iomanip>
#include <optional>
#include <sstream>
#include <tuple>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WifiMloTest");

class GetRnrLinkInfoTest : public TestCase {
public:
  GetRnrLinkInfoTest();
  ~GetRnrLinkInfoTest() override = default;

private:
  void DoRun() override;
};

GetRnrLinkInfoTest::GetRnrLinkInfoTest()
    : TestCase("Check the implementation of "
               "WifiAssocManager::GetNextAffiliatedAp()") {}

void GetRnrLinkInfoTest::DoRun() {
  ReducedNeighborReport rnr;
  std::size_t nbrId;
  std::size_t tbttId;

  rnr.AddNbrApInfoField();
  nbrId = rnr.GetNNbrApInfoFields() - 1;

  rnr.AddTbttInformationField(nbrId);
  rnr.AddTbttInformationField(nbrId);

  rnr.AddNbrApInfoField();
  nbrId = rnr.GetNNbrApInfoFields() - 1;

  rnr.AddTbttInformationField(nbrId);
  tbttId = rnr.GetNTbttInformationFields(nbrId) - 1;
  rnr.SetMldParameters(nbrId, tbttId, 0, 0, 0);

  rnr.AddTbttInformationField(nbrId);
  tbttId = rnr.GetNTbttInformationFields(nbrId) - 1;
  rnr.SetMldParameters(nbrId, tbttId, 5, 0, 0);

  rnr.AddNbrApInfoField();
  nbrId = rnr.GetNNbrApInfoFields() - 1;

  rnr.AddTbttInformationField(nbrId);
  tbttId = rnr.GetNTbttInformationFields(nbrId) - 1;
  rnr.SetMldParameters(nbrId, tbttId, 3, 0, 0);

  rnr.AddTbttInformationField(nbrId);
  tbttId = rnr.GetNTbttInformationFields(nbrId) - 1;
  rnr.SetMldParameters(nbrId, tbttId, 4, 0, 0);

  rnr.AddNbrApInfoField();
  nbrId = rnr.GetNNbrApInfoFields() - 1;

  rnr.AddTbttInformationField(nbrId);
  tbttId = rnr.GetNTbttInformationFields(nbrId) - 1;
  rnr.SetMldParameters(nbrId, tbttId, 6, 0, 0);

  rnr.AddTbttInformationField(nbrId);
  tbttId = rnr.GetNTbttInformationFields(nbrId) - 1;
  rnr.SetMldParameters(nbrId, tbttId, 0, 0, 0);

  auto ret = WifiAssocManager::GetNextAffiliatedAp(rnr, 0);

  NS_TEST_EXPECT_MSG_EQ(ret.has_value(), true,
                        "Expected to find a suitable reported AP");
  NS_TEST_EXPECT_MSG_EQ(ret->m_nbrApInfoId, 1,
                        "Unexpected neighbor ID of the first reported AP");
  NS_TEST_EXPECT_MSG_EQ(ret->m_tbttInfoFieldId, 0,
                        "Unexpected tbtt ID of the first reported AP");

  ret = WifiAssocManager::GetNextAffiliatedAp(rnr, ret->m_nbrApInfoId + 1);

  NS_TEST_EXPECT_MSG_EQ(ret.has_value(), true,
                        "Expected to find a second suitable reported AP");
  NS_TEST_EXPECT_MSG_EQ(ret->m_nbrApInfoId, 3,
                        "Unexpected neighbor ID of the second reported AP");
  NS_TEST_EXPECT_MSG_EQ(ret->m_tbttInfoFieldId, 1,
                        "Unexpected tbtt ID of the second reported AP");

  ret = WifiAssocManager::GetNextAffiliatedAp(rnr, ret->m_nbrApInfoId + 1);

  NS_TEST_EXPECT_MSG_EQ(ret.has_value(), false,
                        "Did not expect to find a third suitable reported AP");

  auto allAps = WifiAssocManager::GetAllAffiliatedAps(rnr);

  NS_TEST_EXPECT_MSG_EQ(allAps.size(), 2,
                        "Expected to find two suitable reported APs");

  auto apIt = allAps.begin();
  NS_TEST_EXPECT_MSG_EQ(apIt->m_nbrApInfoId, 1,
                        "Unexpected neighbor ID of the first reported AP");
  NS_TEST_EXPECT_MSG_EQ(apIt->m_tbttInfoFieldId, 0,
                        "Unexpected tbtt ID of the first reported AP");

  apIt++;
  NS_TEST_EXPECT_MSG_EQ(apIt->m_nbrApInfoId, 3,
                        "Unexpected neighbor ID of the second reported AP");
  NS_TEST_EXPECT_MSG_EQ(apIt->m_tbttInfoFieldId, 1,
                        "Unexpected tbtt ID of the second reported AP");
}

class MldSwapLinksTest : public TestCase {
  class TestWifiMac : public WifiMac {
  public:
    ~TestWifiMac() override = default;

    using WifiMac::GetLinks;
    using WifiMac::SwapLinks;

    bool CanForwardPacketsTo(Mac48Address to) const override { return true; }

    void Enqueue(Ptr<Packet> packet, Mac48Address to) override {}
  };

public:
  MldSwapLinksTest();
  ~MldSwapLinksTest() override = default;

protected:
  void DoRun() override;

private:
  void RunOne(std::string text, std::size_t nLinks,
              const std::map<uint8_t, uint8_t> &links,
              const std::map<uint8_t, uint8_t> &expected);
};

MldSwapLinksTest::MldSwapLinksTest()
    : TestCase("Test the WifiMac::SwapLinks() method") {}

void MldSwapLinksTest::RunOne(std::string text, std::size_t nLinks,
                              const std::map<uint8_t, uint8_t> &links,
                              const std::map<uint8_t, uint8_t> &expected) {
  TestWifiMac mac;

  std::vector<Ptr<WifiPhy>> phys;
  for (std::size_t i = 0; i < nLinks; i++) {
    phys.emplace_back(CreateObject<SpectrumWifiPhy>());
  }
  mac.SetWifiPhys(phys);

  mac.SwapLinks(links);

  NS_TEST_EXPECT_MSG_EQ(mac.GetNLinks(), nLinks,
                        "Number of links changed after swapping");

  for (const auto &[linkId, phyId] : expected) {
    NS_TEST_ASSERT_MSG_EQ(mac.GetLinks().count(linkId), 1,
                          "Link ID " << +linkId << " does not exist");

    NS_TEST_ASSERT_MSG_LT(+phyId, nLinks, "Invalid PHY ID");

    NS_TEST_EXPECT_MSG_EQ(mac.GetWifiPhy(linkId), phys.at(phyId),
                          text << ": Link " << +phyId
                               << " has not been moved to link " << +linkId);
  }
}

void MldSwapLinksTest::DoRun() {
  RunOne("No change needed", 3, {{0, 0}, {1, 1}, {2, 2}},
         {{0, 0}, {1, 1}, {2, 2}});
  RunOne("Circular swapping", 3, {{0, 2}, {1, 0}, {2, 1}},
         {{0, 1}, {1, 2}, {2, 0}});
  RunOne("Swapping two links, one unchanged", 3, {{0, 2}, {2, 0}},
         {{0, 2}, {1, 1}, {2, 0}});
  RunOne("Non-circular swapping, autodetect how to close the loop", 3,
         {{0, 2}, {2, 1}}, {{0, 1}, {1, 2}, {2, 0}});
  RunOne("One move only, autodetect how to complete the swapping", 3, {{2, 0}},
         {{0, 2}, {1, 1}, {2, 0}});
  RunOne("Create a new link ID (2), remove the unused one (0)", 2,
         {{0, 1}, {1, 2}}, {{1, 0}, {2, 1}});
  RunOne("One move only that creates a new link ID (2)", 2, {{0, 2}},
         {{1, 1}, {2, 0}});
  RunOne("Move all links to a new set of IDs", 2, {{0, 2}, {1, 3}},
         {{2, 0}, {3, 1}});
}

class MultiLinkOperationsTestBase : public TestCase {
public:
  struct BaseParams {
    std::vector<std::string> staChannels;
    std::vector<std::string> apChannels;
    std::vector<uint8_t> fixedPhyBands;
  };

  MultiLinkOperationsTestBase(const std::string &name, uint8_t nStations,
                              const BaseParams &baseParams);
  ~MultiLinkOperationsTestBase() override = default;

protected:
  virtual void Transmit(Ptr<WifiMac> mac, uint8_t phyId,
                        WifiConstPsduMap psduMap, WifiTxVector txVector,
                        double txPowerW);

  virtual void L7Receive(uint8_t nodeId, Ptr<const Packet> p,
                         const Address &addr);

  Ptr<PacketSocketClient> GetApplication(const PacketSocketAddress &sockAddr,
                                         std::size_t count, std::size_t pktSize,
                                         Time delay = Seconds(0),
                                         uint8_t priority = 0) const;

  void DoSetup() override;

  using ChannelMap = std::map<FrequencyRange, Ptr<MultiModelSpectrumChannel>>;

  enum Direction { DL = 0, UL };

  void CheckAddresses(Ptr<const WifiPsdu> psdu,
                      std::optional<Direction> direction = std::nullopt);

  struct FrameInfo {
    Time startTx;
    WifiConstPsduMap psduMap;
    WifiTxVector txVector;
    uint8_t linkId;
    uint8_t phyId;
  };

  std::vector<FrameInfo> m_txPsdus;
  const std::vector<std::string> m_staChannels;
  const std::vector<std::string> m_apChannels;
  const std::vector<uint8_t> m_fixedPhyBands;
  Ptr<ApWifiMac> m_apMac;
  std::vector<Ptr<StaWifiMac>> m_staMacs;
  uint8_t m_nStations;
  uint16_t m_lastAid;
  Time m_duration{Seconds(1)};
  std::vector<std::size_t> m_rxPkts;

private:
  void SetChannels(SpectrumWifiPhyHelper &helper,
                   const std::vector<std::string> &channels,
                   const ChannelMap &channelMap);

  void SetSsid(uint16_t aid, Mac48Address);

  virtual void StartTraffic() {}
};

MultiLinkOperationsTestBase::MultiLinkOperationsTestBase(
    const std::string &name, uint8_t nStations, const BaseParams &baseParams)
    : TestCase(name), m_staChannels(baseParams.staChannels),
      m_apChannels(baseParams.apChannels),
      m_fixedPhyBands(baseParams.fixedPhyBands), m_staMacs(nStations),
      m_nStations(nStations), m_lastAid(0), m_rxPkts(nStations + 1) {}

void MultiLinkOperationsTestBase::CheckAddresses(
    Ptr<const WifiPsdu> psdu, std::optional<Direction> direction) {
  std::optional<Mac48Address> apAddr;
  std::optional<Mac48Address> staAddr;

  if (psdu->GetHeader(0).IsQosData()) {
    direction = (!psdu->GetHeader(0).IsToDs() && psdu->GetHeader(0).IsFromDs())
                    ? DL
                    : UL;
  }
  NS_ASSERT(direction);

  if (direction == DL) {
    if (!psdu->GetAddr1().IsGroup()) {
      staAddr = psdu->GetAddr1();
    }
    apAddr = psdu->GetAddr2();
  } else {
    if (!psdu->GetAddr1().IsGroup()) {
      apAddr = psdu->GetAddr1();
    }
    staAddr = psdu->GetAddr2();
  }

  if (apAddr) {
    bool found = false;
    for (uint8_t linkId = 0; linkId < m_apMac->GetNLinks(); linkId++) {
      if (m_apMac->GetFrameExchangeManager(linkId)->GetAddress() == *apAddr) {
        found = true;
        break;
      }
    }
    NS_TEST_EXPECT_MSG_EQ(found, true,
                          "Address " << *apAddr
                                     << " is not an AP device address. "
                                     << "PSDU: " << *psdu);
  }

  if (staAddr) {
    bool found = false;
    for (uint8_t i = 0; i < m_nStations; i++) {
      for (const auto &linkId : m_staMacs[i]->GetLinkIds()) {
        if (m_staMacs[i]->GetFrameExchangeManager(linkId)->GetAddress() ==
            *staAddr) {
          found = true;
          break;
        }
      }
      if (found) {
        break;
      }
    }
    NS_TEST_EXPECT_MSG_EQ(found, true,
                          "Address " << *staAddr
                                     << " is not a STA device address. "
                                     << "PSDU: " << *psdu);
  }
}

void MultiLinkOperationsTestBase::Transmit(Ptr<WifiMac> mac, uint8_t phyId,
                                           WifiConstPsduMap psduMap,
                                           WifiTxVector txVector,
                                           double txPowerW) {
  auto linkId = mac->GetLinkForPhy(phyId);
  NS_TEST_ASSERT_MSG_EQ(linkId.has_value(), true,
                        "No link found for PHY ID " << +phyId);
  m_txPsdus.push_back({Simulator::Now(), psduMap, txVector, *linkId, phyId});

  for (const auto &[aid, psdu] : psduMap) {
    std::stringstream ss;
    ss << std::setprecision(10) << "PSDU #" << m_txPsdus.size() << " Link ID "
       << +linkId.value() << " Phy ID " << +phyId << " "
       << psdu->GetHeader(0).GetTypeString() << " #MPDUs " << psdu->GetNMpdus()
       << " duration/ID " << psdu->GetHeader(0).GetDuration()
       << " RA = " << psdu->GetAddr1() << " TA = " << psdu->GetAddr2()
       << " ADDR3 = " << psdu->GetHeader(0).GetAddr3()
       << " ToDS = " << psdu->GetHeader(0).IsToDs()
       << " FromDS = " << psdu->GetHeader(0).IsFromDs();
    if (psdu->GetHeader(0).IsQosData()) {
      ss << " seqNo = {";
      for (auto &mpdu : *PeekPointer(psdu)) {
        ss << mpdu->GetHeader().GetSequenceNumber() << ",";
      }
      ss << "} TID = " << +psdu->GetHeader(0).GetQosTid();
    }
    NS_LOG_INFO(ss.str());
  }
  NS_LOG_INFO("TXVECTOR = " << txVector << "\n");
}

void MultiLinkOperationsTestBase::L7Receive(uint8_t nodeId, Ptr<const Packet> p,
                                            const Address &addr) {
  NS_LOG_INFO("Packet received by NODE " << +nodeId << "\n");
  m_rxPkts[nodeId]++;
}

void MultiLinkOperationsTestBase::SetChannels(
    SpectrumWifiPhyHelper &helper, const std::vector<std::string> &channels,
    const ChannelMap &channelMap) {
  helper = SpectrumWifiPhyHelper(channels.size());
  helper.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);

  uint8_t linkId = 0;
  for (const auto &str : channels) {
    helper.Set(linkId++, "ChannelSettings", StringValue(str));
  }

  for (const auto &[band, channel] : channelMap) {
    helper.AddChannel(channel, band);
  }
}

void MultiLinkOperationsTestBase::DoSetup() {
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(2);
  int64_t streamNumber = 30;

  NodeContainer wifiApNode;
  wifiApNode.Create(1);

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create(m_nStations);

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211be);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue("EhtMcs0"), "ControlMode",
                               StringValue("HtMcs0"));

  ChannelMap channelMap{
      {WIFI_SPECTRUM_2_4_GHZ, CreateObject<MultiModelSpectrumChannel>()},
      {WIFI_SPECTRUM_5_GHZ, CreateObject<MultiModelSpectrumChannel>()},
      {WIFI_SPECTRUM_6_GHZ, CreateObject<MultiModelSpectrumChannel>()}};

  SpectrumWifiPhyHelper staPhyHelper;
  SpectrumWifiPhyHelper apPhyHelper;
  SetChannels(staPhyHelper, m_staChannels, channelMap);
  SetChannels(apPhyHelper, m_apChannels, channelMap);

  for (const auto &linkId : m_fixedPhyBands) {
    staPhyHelper.Set(linkId, "FixedPhyBand", BooleanValue(true));
  }

  WifiMacHelper mac;
  mac.SetType("ns3::StaWifiMac", "ActiveProbing", BooleanValue(false));

  NetDeviceContainer staDevices = wifi.Install(staPhyHelper, mac, wifiStaNodes);

  mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(Ssid("ns-3-ssid")),
              "BeaconGeneration", BooleanValue(true));

  NetDeviceContainer apDevices = wifi.Install(apPhyHelper, mac, wifiApNode);

  streamNumber += wifi.AssignStreams(apDevices, streamNumber);
  streamNumber += wifi.AssignStreams(staDevices, streamNumber);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc =
      CreateObject<ListPositionAllocator>();

  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  positionAlloc->Add(Vector(1.0, 0.0, 0.0));
  mobility.SetPositionAllocator(positionAlloc);

  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(wifiApNode);
  mobility.Install(wifiStaNodes);

  m_apMac = DynamicCast<ApWifiMac>(
      DynamicCast<WifiNetDevice>(apDevices.Get(0))->GetMac());
  for (uint8_t i = 0; i < m_nStations; i++) {
    m_staMacs[i] = DynamicCast<StaWifiMac>(
        DynamicCast<WifiNetDevice>(staDevices.Get(i))->GetMac());
  }

  for (uint8_t phyId = 0; phyId < m_apMac->GetDevice()->GetNPhys(); phyId++) {
    Config::ConnectWithoutContext(
        "/NodeList/0/DeviceList/*/$ns3::WifiNetDevice/Phys/" +
            std::to_string(phyId) + "/PhyTxPsduBegin",
        MakeCallback(&MultiLinkOperationsTestBase::Transmit, this)
            .Bind(m_apMac, phyId));
  }
  for (uint8_t i = 0; i < m_nStations; i++) {
    for (uint8_t phyId = 0; phyId < m_staMacs[i]->GetDevice()->GetNPhys();
         phyId++) {
      Config::ConnectWithoutContext(
          "/NodeList/" + std::to_string(i + 1) +
              "/DeviceList/*/$ns3::WifiNetDevice/Phys/" +
              std::to_string(phyId) + "/PhyTxPsduBegin",
          MakeCallback(&MultiLinkOperationsTestBase::Transmit, this)
              .Bind(m_staMacs[i], phyId));
    }
  }

  PacketSocketHelper packetSocket;
  packetSocket.Install(wifiApNode);
  packetSocket.Install(wifiStaNodes);

  for (auto nodeIt = NodeList::Begin(); nodeIt != NodeList::End(); ++nodeIt) {
    PacketSocketAddress srvAddr;
    auto device = DynamicCast<WifiNetDevice>((*nodeIt)->GetDevice(0));
    NS_TEST_ASSERT_MSG_NE(device, nullptr, "Expected a WifiNetDevice");
    srvAddr.SetSingleDevice(device->GetIfIndex());
    srvAddr.SetProtocol(1);

    auto server = CreateObject<PacketSocketServer>();
    server->SetLocal(srvAddr);
    (*nodeIt)->AddApplication(server);
    server->SetStartTime(Seconds(0));
    server->SetStopTime(m_duration);
  }

  for (std::size_t nodeId = 0; nodeId < NodeList::GetNNodes(); nodeId++) {
    Config::ConnectWithoutContext(
        "/NodeList/" + std::to_string(nodeId) +
            "/ApplicationList/*/$ns3::PacketSocketServer/Rx",
        MakeCallback(&MultiLinkOperationsTestBase::L7Receive, this)
            .Bind(nodeId));
  }

  m_apMac->TraceConnectWithoutContext(
      "AssociatedSta",
      MakeCallback(&MultiLinkOperationsTestBase::SetSsid, this));
  m_staMacs[0]->SetSsid(Ssid("ns-3-ssid"));
}

Ptr<PacketSocketClient> MultiLinkOperationsTestBase::GetApplication(
    const PacketSocketAddress &sockAddr, std::size_t count, std::size_t pktSize,
    Time delay, uint8_t priority) const {
  auto client = CreateObject<PacketSocketClient>();
  client->SetAttribute("PacketSize", UintegerValue(pktSize));
  client->SetAttribute("MaxPackets", UintegerValue(count));
  client->SetAttribute("Interval", TimeValue(MicroSeconds(0)));
  client->SetAttribute("Priority", UintegerValue(priority));
  client->SetRemote(sockAddr);
  client->SetStartTime(delay);
  client->SetStopTime(m_duration - Simulator::Now());

  return client;
}

void MultiLinkOperationsTestBase::SetSsid(uint16_t aid, Mac48Address) {
  if (m_lastAid == aid) {
    return;
  }
  m_lastAid = aid;

  if (aid < m_nStations) {
    m_staMacs[aid]->SetSsid(Ssid("ns-3-ssid"));
    return;
  }
  Simulator::Schedule(MilliSeconds(5),
                      &MultiLinkOperationsTestBase::StartTraffic, this);
}

class MultiLinkSetupTest : public MultiLinkOperationsTestBase {
public:
  MultiLinkSetupTest(const BaseParams &baseParams, WifiScanType scanType,
                     const std::vector<uint8_t> &setupLinks,
                     uint8_t apNegSupport,
                     const std::string &dlTidToLinkMapping,
                     const std::string &ulTidToLinkMapping);
  ~MultiLinkSetupTest() override = default;

protected:
  void DoSetup() override;
  void DoRun() override;

private:
  void StartTraffic() override;

  void CheckMlSetup();

  void CheckDisabledLinks();

  void CheckBeacon(Ptr<WifiMpdu> mpdu, uint8_t linkId);

  void CheckProbeResponse(Ptr<WifiMpdu> mpdu, uint8_t linkId);

  void CheckAssocRequest(Ptr<WifiMpdu> mpdu, uint8_t linkId);

  void CheckAssocResponse(Ptr<WifiMpdu> mpdu, uint8_t linkId);

  void CheckQosData(Ptr<WifiMpdu> mpdu, uint8_t linkId, std::size_t index);

  const std::vector<uint8_t> m_setupLinks;
  WifiScanType m_scanType;
  std::size_t m_nProbeResp;
  uint8_t m_apNegSupport;
  std::string m_dlTidLinkMappingStr;
  std::string m_ulTidLinkMappingStr;
  WifiTidLinkMapping m_dlTidLinkMapping;
  WifiTidLinkMapping m_ulTidLinkMapping;
  uint8_t m_dlTid1;
  uint8_t m_ulTid1;
  std::optional<uint8_t> m_dlTid2;
  std::optional<uint8_t> m_ulTid2;
  std::vector<std::size_t> m_qosFrames1;
  std::vector<std::size_t> m_qosFrames2;
};

MultiLinkSetupTest::MultiLinkSetupTest(const BaseParams &baseParams,
                                       WifiScanType scanType,
                                       const std::vector<uint8_t> &setupLinks,
                                       uint8_t apNegSupport,
                                       const std::string &dlTidToLinkMapping,
                                       const std::string &ulTidToLinkMapping)
    : MultiLinkOperationsTestBase("Check correctness of Multi-Link Setup", 1,
                                  baseParams),
      m_setupLinks(setupLinks), m_scanType(scanType), m_nProbeResp(0),
      m_apNegSupport(apNegSupport), m_dlTidLinkMappingStr(dlTidToLinkMapping),
      m_ulTidLinkMappingStr(ulTidToLinkMapping) {}

void MultiLinkSetupTest::DoSetup() {
  MultiLinkOperationsTestBase::DoSetup();

  m_staMacs[0]->SetAttribute("ActiveProbing",
                             BooleanValue(m_scanType == WifiScanType::ACTIVE));
  m_apMac->GetEhtConfiguration()->SetAttribute("TidToLinkMappingNegSupport",
                                               EnumValue(m_apNegSupport));
  auto staEhtConfig = m_staMacs[0]->GetEhtConfiguration();
  staEhtConfig->SetAttribute("TidToLinkMappingNegSupport", EnumValue(3));
  staEhtConfig->SetAttribute("TidToLinkMappingDl",
                             StringValue(m_dlTidLinkMappingStr));
  staEhtConfig->SetAttribute("TidToLinkMappingUl",
                             StringValue(m_ulTidLinkMappingStr));

  m_dlTidLinkMapping = staEhtConfig->GetTidLinkMapping(WifiDirection::DOWNLINK);
  m_ulTidLinkMapping = staEhtConfig->GetTidLinkMapping(WifiDirection::UPLINK);

  if (m_apNegSupport == 0 ||
      (m_apNegSupport == 1 && !TidToLinkMappingValidForNegType1(
                                  m_dlTidLinkMapping, m_ulTidLinkMapping))) {
    m_dlTidLinkMapping.clear();
    m_ulTidLinkMapping.clear();
  }

  using TupleRefs =
      std::tuple<std::reference_wrapper<const WifiTidLinkMapping>,
                 std::reference_wrapper<uint8_t>,
                 std::reference_wrapper<std::optional<uint8_t>>, Ptr<WifiMac>>;
  for (auto &[mappingRef, tid1Ref, tid2Ref, mac] :
       {TupleRefs{m_dlTidLinkMapping, m_dlTid1, m_dlTid2, m_apMac},
        TupleRefs{m_ulTidLinkMapping, m_ulTid1, m_ulTid2, m_staMacs[0]}}) {
    tid1Ref.get() = 0;
    for (uint8_t tid1 = 0; tid1 < 8; tid1++) {
      if (auto it1 = mappingRef.get().find(tid1);
          it1 != mappingRef.get().cend() &&
          it1->second.size() != m_setupLinks.size()) {
        for (uint8_t tid2 = tid1 + 1; tid2 < 8; tid2++) {
          if (auto it2 = mappingRef.get().find(tid2);
              it2 != mappingRef.get().cend() &&
              it2->second.size() != m_setupLinks.size()) {
            std::list<uint8_t> intersection;
            std::set_intersection(it1->second.cbegin(), it1->second.cend(),
                                  it2->second.cbegin(), it2->second.cend(),
                                  std::back_inserter(intersection));
            if (intersection.empty()) {
              tid2Ref.get() = tid2;
              break;
            }
          }
        }
        tid1Ref.get() = tid1;
        break;
      }
    }

    std::list<uint8_t> tids = {tid1Ref.get()};
    if (tid2Ref.get()) {
      tids.emplace_back(*tid2Ref.get());
    }

    for (auto tid : tids) {
      std::string attrName;
      switch (QosUtilsMapTidToAc(tid)) {
      case AC_VI:
        attrName = "VI_MaxAmpduSize";
        break;
      case AC_VO:
        attrName = "VO_MaxAmpduSize";
        break;
      case AC_BE:
        attrName = "BE_MaxAmpduSize";
        break;
      case AC_BK:
        attrName = "BK_MaxAmpduSize";
        break;
      default:
        NS_FATAL_ERROR("Invalid TID " << +tid);
      }

      mac->SetAttribute(attrName, UintegerValue(100));
    }
  }
}

void MultiLinkSetupTest::StartTraffic() {
  {
    PacketSocketAddress sockAddr;
    sockAddr.SetSingleDevice(m_apMac->GetDevice()->GetIfIndex());
    sockAddr.SetPhysicalAddress(m_staMacs[0]->GetDevice()->GetAddress());
    sockAddr.SetProtocol(1);

    m_apMac->GetDevice()->GetNode()->AddApplication(GetApplication(
        sockAddr, m_setupLinks.size(), 500, Seconds(0), m_dlTid1));
    if (m_dlTid2) {
      m_apMac->GetDevice()->GetNode()->AddApplication(GetApplication(
          sockAddr, m_setupLinks.size(), 500, Seconds(0), *m_dlTid2));
    }
  }

  {
    PacketSocketAddress sockAddr;
    sockAddr.SetSingleDevice(m_staMacs[0]->GetDevice()->GetIfIndex());
    sockAddr.SetPhysicalAddress(m_apMac->GetDevice()->GetAddress());
    sockAddr.SetProtocol(1);

    m_staMacs[0]->GetDevice()->GetNode()->AddApplication(GetApplication(
        sockAddr, m_setupLinks.size(), 500, MilliSeconds(500), m_ulTid1));
    if (m_ulTid2) {
      m_staMacs[0]->GetDevice()->GetNode()->AddApplication(GetApplication(
          sockAddr, m_setupLinks.size(), 500, MilliSeconds(500), *m_ulTid2));
    }
  }
}

void MultiLinkSetupTest::DoRun() {
  Simulator::Schedule(MilliSeconds(500), &MultiLinkSetupTest::CheckMlSetup,
                      this);

  Simulator::Stop(m_duration);
  Simulator::Run();

  std::size_t index = 0;

  for (const auto &frameInfo : m_txPsdus) {
    const auto &mpdu = *frameInfo.psduMap.begin()->second->begin();
    const auto &linkId = frameInfo.linkId;

    switch (mpdu->GetHeader().GetType()) {
    case WIFI_MAC_MGT_BEACON:
      CheckBeacon(mpdu, linkId);
      break;

    case WIFI_MAC_MGT_PROBE_RESPONSE:
      CheckProbeResponse(mpdu, linkId);
      m_nProbeResp++;
      break;

    case WIFI_MAC_MGT_ASSOCIATION_REQUEST:
      CheckAssocRequest(mpdu, linkId);
      break;

    case WIFI_MAC_MGT_ASSOCIATION_RESPONSE:
      CheckAssocResponse(mpdu, linkId);
      break;

    case WIFI_MAC_QOSDATA:
      CheckQosData(mpdu, linkId, index);
      break;

    default:
      break;
    }

    index++;
  }

  CheckDisabledLinks();

  std::size_t expectedProbeResp = 0;
  if (m_scanType == WifiScanType::ACTIVE) {
    for (const auto &staChannel : m_staChannels) {
      for (const auto &apChannel : m_apChannels) {
        if (staChannel == apChannel) {
          expectedProbeResp++;
          break;
        }
      }
    }
  }

  NS_TEST_EXPECT_MSG_EQ(m_nProbeResp, expectedProbeResp,
                        "Unexpected number of Probe Responses");

  std::size_t expectedRxDlPkts = m_setupLinks.size();
  if (m_dlTid2) {
    expectedRxDlPkts *= 2;
  }
  NS_TEST_EXPECT_MSG_EQ(m_rxPkts[m_staMacs[0]->GetDevice()->GetNode()->GetId()],
                        expectedRxDlPkts,
                        "Unexpected number of DL packets received");

  std::size_t expectedRxUlPkts = m_setupLinks.size();
  if (m_ulTid2) {
    expectedRxUlPkts *= 2;
  }
  NS_TEST_EXPECT_MSG_EQ(m_rxPkts[m_apMac->GetDevice()->GetNode()->GetId()],
                        expectedRxUlPkts,
                        "Unexpected number of UL packets received");

  Simulator::Destroy();
}

void MultiLinkSetupTest::CheckBeacon(Ptr<WifiMpdu> mpdu, uint8_t linkId) {
  NS_ABORT_IF(mpdu->GetHeader().GetType() != WIFI_MAC_MGT_BEACON);

  CheckAddresses(Create<WifiPsdu>(mpdu, false),
                 MultiLinkOperationsTestBase::DL);

  NS_TEST_EXPECT_MSG_EQ(
      m_apMac->GetFrameExchangeManager(linkId)->GetAddress(),
      mpdu->GetHeader().GetAddr2(),
      "TA of Beacon frame is not the address of the link it is transmitted on");
  MgtBeaconHeader beacon;
  mpdu->GetPacket()->PeekHeader(beacon);
  const auto &rnr = beacon.Get<ReducedNeighborReport>();
  const auto &mle = beacon.Get<MultiLinkElement>();

  if (m_apMac->GetNLinks() == 1) {
    NS_TEST_EXPECT_MSG_EQ(rnr.has_value(), false,
                          "RNR Element in Beacon frame from single link AP");
    NS_TEST_EXPECT_MSG_EQ(
        mle.has_value(), false,
        "Multi-Link Element in Beacon frame from single link AP");
    return;
  }

  NS_TEST_EXPECT_MSG_EQ(rnr.has_value(), true,
                        "No RNR Element in Beacon frame");
  NS_TEST_EXPECT_MSG_EQ(rnr->GetNNbrApInfoFields(),
                        static_cast<std::size_t>(m_apMac->GetNLinks() - 1),
                        "Unexpected number of Neighbor AP Info fields in RNR");
  for (std::size_t nbrApInfoId = 0; nbrApInfoId < rnr->GetNNbrApInfoFields();
       nbrApInfoId++) {
    NS_TEST_EXPECT_MSG_EQ(rnr->HasMldParameters(nbrApInfoId), true,
                          "MLD Parameters not present");
    NS_TEST_EXPECT_MSG_EQ(
        rnr->GetNTbttInformationFields(nbrApInfoId), 1,
        "Expected only one TBTT Info subfield per Neighbor AP Info");
    uint8_t nbrLinkId = rnr->GetLinkId(nbrApInfoId, 0);
    NS_TEST_EXPECT_MSG_EQ(
        rnr->GetBssid(nbrApInfoId, 0),
        m_apMac->GetFrameExchangeManager(nbrLinkId)->GetAddress(),
        "BSSID advertised in Neighbor AP Info field "
            << nbrApInfoId
            << " does not match the address configured on the link "
               "advertised in the same field");
  }

  NS_TEST_EXPECT_MSG_EQ(mle.has_value(), true,
                        "No Multi-Link Element in Beacon frame");
  NS_TEST_EXPECT_MSG_EQ(
      mle->GetMldMacAddress(), m_apMac->GetAddress(),
      "Incorrect MLD address advertised in Multi-Link Element");
  NS_TEST_EXPECT_MSG_EQ(mle->GetLinkIdInfo(), +linkId,
                        "Incorrect Link ID advertised in Multi-Link Element");
}

void MultiLinkSetupTest::CheckProbeResponse(Ptr<WifiMpdu> mpdu,
                                            uint8_t linkId) {
  NS_ABORT_IF(mpdu->GetHeader().GetType() != WIFI_MAC_MGT_PROBE_RESPONSE);

  CheckAddresses(Create<WifiPsdu>(mpdu, false),
                 MultiLinkOperationsTestBase::DL);

  NS_TEST_EXPECT_MSG_EQ(m_apMac->GetFrameExchangeManager(linkId)->GetAddress(),
                        mpdu->GetHeader().GetAddr2(),
                        "TA of Probe Response is not the address of the link "
                        "it is transmitted on");
  MgtProbeResponseHeader probeResp;
  mpdu->GetPacket()->PeekHeader(probeResp);
  const auto &rnr = probeResp.Get<ReducedNeighborReport>();
  const auto &mle = probeResp.Get<MultiLinkElement>();

  if (m_apMac->GetNLinks() == 1) {
    NS_TEST_EXPECT_MSG_EQ(
        rnr.has_value(), false,
        "RNR Element in Probe Response frame from single link AP");
    NS_TEST_EXPECT_MSG_EQ(
        mle.has_value(), false,
        "Multi-Link Element in Probe Response frame from single link AP");
    return;
  }

  NS_TEST_EXPECT_MSG_EQ(rnr.has_value(), true,
                        "No RNR Element in Probe Response frame");
  NS_TEST_EXPECT_MSG_EQ(rnr->GetNNbrApInfoFields(),
                        static_cast<std::size_t>(m_apMac->GetNLinks() - 1),
                        "Unexpected number of Neighbor AP Info fields in RNR");
  for (std::size_t nbrApInfoId = 0; nbrApInfoId < rnr->GetNNbrApInfoFields();
       nbrApInfoId++) {
    NS_TEST_EXPECT_MSG_EQ(rnr->HasMldParameters(nbrApInfoId), true,
                          "MLD Parameters not present");
    NS_TEST_EXPECT_MSG_EQ(
        rnr->GetNTbttInformationFields(nbrApInfoId), 1,
        "Expected only one TBTT Info subfield per Neighbor AP Info");
    uint8_t nbrLinkId = rnr->GetLinkId(nbrApInfoId, 0);
    NS_TEST_EXPECT_MSG_EQ(
        rnr->GetBssid(nbrApInfoId, 0),
        m_apMac->GetFrameExchangeManager(nbrLinkId)->GetAddress(),
        "BSSID advertised in Neighbor AP Info field "
            << nbrApInfoId
            << " does not match the address configured on the link "
               "advertised in the same field");
  }

  NS_TEST_EXPECT_MSG_EQ(mle.has_value(), true,
                        "No Multi-Link Element in Probe Response frame");
  NS_TEST_EXPECT_MSG_EQ(
      mle->GetMldMacAddress(), m_apMac->GetAddress(),
      "Incorrect MLD address advertised in Multi-Link Element");
  NS_TEST_EXPECT_MSG_EQ(mle->GetLinkIdInfo(), +linkId,
                        "Incorrect Link ID advertised in Multi-Link Element");
}

void MultiLinkSetupTest::CheckAssocRequest(Ptr<WifiMpdu> mpdu, uint8_t linkId) {
  NS_ABORT_IF(mpdu->GetHeader().GetType() != WIFI_MAC_MGT_ASSOCIATION_REQUEST);

  CheckAddresses(Create<WifiPsdu>(mpdu, false),
                 MultiLinkOperationsTestBase::UL);

  NS_TEST_EXPECT_MSG_EQ(
      m_staMacs[0]->GetFrameExchangeManager(linkId)->GetAddress(),
      mpdu->GetHeader().GetAddr2(),
      "TA of Assoc Request frame is not the address of the link it is "
      "transmitted on");
  MgtAssocRequestHeader assoc;
  mpdu->GetPacket()->PeekHeader(assoc);
  const auto &mle = assoc.Get<MultiLinkElement>();

  if (m_apMac->GetNLinks() == 1 || m_staMacs[0]->GetNLinks() == 1) {
    NS_TEST_EXPECT_MSG_EQ(
        mle.has_value(), false,
        "Multi-Link Element in Assoc Request frame from single link STA");
  } else {
    NS_TEST_EXPECT_MSG_EQ(mle.has_value(), true,
                          "No Multi-Link Element in Assoc Request frame");
    NS_TEST_EXPECT_MSG_EQ(
        mle->GetMldMacAddress(), m_staMacs[0]->GetAddress(),
        "Incorrect MLD Address advertised in Multi-Link Element");
    NS_TEST_EXPECT_MSG_EQ(mle->GetNPerStaProfileSubelements(),
                          m_setupLinks.size() - 1,
                          "Incorrect number of Per-STA Profile subelements in "
                          "Multi-Link Element");
    for (std::size_t i = 0; i < mle->GetNPerStaProfileSubelements(); i++) {
      auto &perStaProfile = mle->GetPerStaProfile(i);
      NS_TEST_EXPECT_MSG_EQ(perStaProfile.HasStaMacAddress(), true,
                            "Per-STA Profile must contain STA MAC address");
      auto staLinkId =
          m_staMacs[0]->GetLinkIdByAddress(perStaProfile.GetStaMacAddress());
      NS_TEST_EXPECT_MSG_EQ(staLinkId.has_value(), true,
                            "No link found with the STA MAC address advertised "
                            "in Per-STA Profile");
      NS_TEST_EXPECT_MSG_NE(+staLinkId.value(), +linkId,
                            "The STA that sent the Assoc Request should not be "
                            "included in a Per-STA Profile");
      auto it = std::find(m_setupLinks.begin(), m_setupLinks.end(),
                          staLinkId.value());
      NS_TEST_EXPECT_MSG_EQ((it != m_setupLinks.end()), true,
                            "Not expecting to setup STA link ID "
                                << +staLinkId.value());
      NS_TEST_EXPECT_MSG_EQ(+staLinkId.value(), +perStaProfile.GetLinkId(),
                            "Not expecting to request association to AP Link "
                            "ID in Per-STA Profile");
      NS_TEST_EXPECT_MSG_EQ(perStaProfile.HasAssocRequest(), true,
                            "Missing Association Request in Per-STA Profile");
    }
  }

  const auto &tlm = assoc.Get<TidToLinkMapping>();

  if (m_apMac->GetNLinks() == 1 || m_staMacs[0]->GetNLinks() == 1 ||
      m_apNegSupport == 0) {
    NS_TEST_EXPECT_MSG_EQ(
        tlm.empty(), true,
        "Didn't expect a TID-to-Link Mapping IE in Assoc Request frame");
  } else {
    std::size_t expectedNTlm =
        (m_dlTidLinkMapping == m_ulTidLinkMapping ? 1 : 2);

    NS_TEST_ASSERT_MSG_EQ(
        tlm.size(), expectedNTlm,
        "Unexpected number of TID-to-Link Mapping IE in Assoc Request");

    auto checkTlm = [&](std::size_t tlmId, WifiDirection dir) {
      NS_TEST_EXPECT_MSG_EQ(
          +static_cast<uint8_t>(tlm[tlmId].m_control.direction),
          +static_cast<uint8_t>(dir),
          "Unexpected direction in TID-to-Link Mapping IE " << tlmId);
      auto &expectedMapping =
          (dir == WifiDirection::UPLINK ? m_ulTidLinkMapping
                                        : m_dlTidLinkMapping);

      NS_TEST_EXPECT_MSG_EQ(tlm[tlmId].m_control.defaultMapping,
                            expectedMapping.empty(),
                            "Default Link Mapping bit not set correctly");
      NS_TEST_EXPECT_MSG_EQ(
          tlm[tlmId].m_linkMapping.size(), expectedMapping.size(),
          "Unexpected number of Link Mapping Of TID n fields");
      for (uint8_t tid = 0; tid < 8; tid++) {
        if (auto it = expectedMapping.find(tid); it != expectedMapping.cend()) {
          NS_TEST_EXPECT_MSG_EQ(
              (tlm[tlmId].GetLinkMappingOfTid(tid) == it->second), true,
              "Unexpected link mapping for TID " << +tid << " direction "
                                                 << dir);
        } else {
          NS_TEST_EXPECT_MSG_EQ(
              tlm[tlmId].GetLinkMappingOfTid(tid).empty(), true,
              "Expecting no Link Mapping Of TID n field for TID "
                  << +tid << " direction " << dir);
        }
      }
    };

    if (tlm.size() == 1) {
      checkTlm(0, WifiDirection::BOTH_DIRECTIONS);
    } else {
      std::size_t dlId =
          (tlm[0].m_control.direction == WifiDirection::DOWNLINK ? 0 : 1);
      std::size_t ulId = (dlId == 0 ? 1 : 0);

      checkTlm(dlId, WifiDirection::DOWNLINK);
      checkTlm(ulId, WifiDirection::UPLINK);
    }
  }
}

void MultiLinkSetupTest::CheckAssocResponse(Ptr<WifiMpdu> mpdu,
                                            uint8_t linkId) {
  NS_ABORT_IF(mpdu->GetHeader().GetType() != WIFI_MAC_MGT_ASSOCIATION_RESPONSE);

  CheckAddresses(Create<WifiPsdu>(mpdu, false),
                 MultiLinkOperationsTestBase::DL);

  NS_TEST_EXPECT_MSG_EQ(m_apMac->GetFrameExchangeManager(linkId)->GetAddress(),
                        mpdu->GetHeader().GetAddr2(),
                        "TA of Assoc Response frame is not the address of the "
                        "link it is transmitted on");
  MgtAssocResponseHeader assoc;
  mpdu->GetPacket()->PeekHeader(assoc);
  const auto &mle = assoc.Get<MultiLinkElement>();

  if (m_apMac->GetNLinks() == 1 || m_staMacs[0]->GetNLinks() == 1) {
    NS_TEST_EXPECT_MSG_EQ(mle.has_value(), false,
                          "Multi-Link Element in Assoc Response frame with "
                          "single link AP or single link STA");
    return;
  }

  NS_TEST_EXPECT_MSG_EQ(mle.has_value(), true,
                        "No Multi-Link Element in Assoc Request frame");
  NS_TEST_EXPECT_MSG_EQ(
      mle->GetMldMacAddress(), m_apMac->GetAddress(),
      "Incorrect MLD Address advertised in Multi-Link Element");
  NS_TEST_EXPECT_MSG_EQ(
      mle->GetNPerStaProfileSubelements(), m_setupLinks.size() - 1,
      "Incorrect number of Per-STA Profile subelements in Multi-Link Element");
  for (std::size_t i = 0; i < mle->GetNPerStaProfileSubelements(); i++) {
    auto &perStaProfile = mle->GetPerStaProfile(i);
    NS_TEST_EXPECT_MSG_EQ(perStaProfile.HasStaMacAddress(), true,
                          "Per-STA Profile must contain STA MAC address");
    auto apLinkId =
        m_apMac->GetLinkIdByAddress(perStaProfile.GetStaMacAddress());
    NS_TEST_EXPECT_MSG_EQ(
        apLinkId.has_value(), true,
        "No link found with the STA MAC address advertised in Per-STA Profile");
    NS_TEST_EXPECT_MSG_EQ(
        +apLinkId.value(), +perStaProfile.GetLinkId(),
        "Link ID and MAC address advertised in Per-STA Profile do not match");
    NS_TEST_EXPECT_MSG_NE(+apLinkId.value(), +linkId,
                          "The AP that sent the Assoc Response should not be "
                          "included in a Per-STA Profile");
    auto it =
        std::find(m_setupLinks.begin(), m_setupLinks.end(), apLinkId.value());
    NS_TEST_EXPECT_MSG_EQ((it != m_setupLinks.end()), true,
                          "Not expecting to setup AP link ID "
                              << +apLinkId.value());
    NS_TEST_EXPECT_MSG_EQ(perStaProfile.HasAssocResponse(), true,
                          "Missing Association Response in Per-STA Profile");
  }

  NS_TEST_EXPECT_MSG_EQ(
      assoc.Get<TidToLinkMapping>().empty(), true,
      "Didn't expect to find a TID-to-Link Mapping IE in Association Response");
}

void MultiLinkSetupTest::CheckMlSetup() {
  NS_TEST_EXPECT_MSG_EQ(m_staMacs[0]->IsAssociated(), true,
                        "Expected the STA to be associated");

  for (const auto linkId : m_setupLinks) {
    auto staLinkId =
        (m_staMacs[0]->GetNLinks() > 1 ? linkId : SINGLE_LINK_OP_ID);
    auto apLinkId = (m_apMac->GetNLinks() > 1 ? linkId : SINGLE_LINK_OP_ID);

    auto staAddr =
        m_staMacs[0]->GetFrameExchangeManager(staLinkId)->GetAddress();
    auto apAddr = m_apMac->GetFrameExchangeManager(apLinkId)->GetAddress();

    auto staRemoteMgr = m_staMacs[0]->GetWifiRemoteStationManager(staLinkId);
    auto apRemoteMgr = m_apMac->GetWifiRemoteStationManager(apLinkId);

    NS_TEST_EXPECT_MSG_EQ(
        m_staMacs[0]->GetFrameExchangeManager(staLinkId)->GetBssid(), apAddr,
        "Unexpected BSSID for STA link ID " << +staLinkId);
    if (m_apMac->GetNLinks() > 1 && m_staMacs[0]->GetNLinks() > 1) {
      NS_TEST_EXPECT_MSG_EQ(
          (staRemoteMgr->GetMldAddress(apAddr) == m_apMac->GetAddress()), true,
          "Incorrect MLD address stored by STA on link ID " << +staLinkId);
      NS_TEST_EXPECT_MSG_EQ(
          (staRemoteMgr->GetAffiliatedStaAddress(m_apMac->GetAddress()) ==
           apAddr),
          true,
          "Incorrect affiliated address stored by STA on link ID "
              << +staLinkId);
    }

    NS_TEST_EXPECT_MSG_EQ(apRemoteMgr->IsAssociated(staAddr), true,
                          "Expecting STA " << staAddr
                                           << " to be associated on link "
                                           << +apLinkId);
    if (m_apMac->GetNLinks() > 1 && m_staMacs[0]->GetNLinks() > 1) {
      NS_TEST_EXPECT_MSG_EQ(
          (apRemoteMgr->GetMldAddress(staAddr) == m_staMacs[0]->GetAddress()),
          true, "Incorrect MLD address stored by AP on link ID " << +apLinkId);
      NS_TEST_EXPECT_MSG_EQ(
          (apRemoteMgr->GetAffiliatedStaAddress(m_staMacs[0]->GetAddress()) ==
           staAddr),
          true,
          "Incorrect affiliated address stored by AP on link ID " << +apLinkId);
    }
    auto aid = m_apMac->GetAssociationId(staAddr, apLinkId);
    const auto &staList = m_apMac->GetStaList(apLinkId);
    NS_TEST_EXPECT_MSG_EQ((staList.find(aid) != staList.end()), true,
                          "STA " << staAddr
                                 << " not found in list of associated STAs");

    NS_TEST_EXPECT_MSG_EQ(
        +m_staMacs[0]->GetWifiPhy(staLinkId)->GetOperatingChannel().GetNumber(),
        +m_apMac->GetWifiPhy(apLinkId)->GetOperatingChannel().GetNumber(),
        "Incorrect operating channel number for STA on link " << +staLinkId);
    NS_TEST_EXPECT_MSG_EQ(
        m_staMacs[0]
            ->GetWifiPhy(staLinkId)
            ->GetOperatingChannel()
            .GetFrequency(),
        m_apMac->GetWifiPhy(apLinkId)->GetOperatingChannel().GetFrequency(),
        "Incorrect operating channel frequency for STA on link " << +staLinkId);
    NS_TEST_EXPECT_MSG_EQ(
        m_staMacs[0]->GetWifiPhy(staLinkId)->GetOperatingChannel().GetWidth(),
        m_apMac->GetWifiPhy(apLinkId)->GetOperatingChannel().GetWidth(),
        "Incorrect operating channel width for STA on link " << +staLinkId);
    NS_TEST_EXPECT_MSG_EQ(
        +m_staMacs[0]
             ->GetWifiPhy(staLinkId)
             ->GetOperatingChannel()
             .GetPhyBand(),
        +m_apMac->GetWifiPhy(apLinkId)->GetOperatingChannel().GetPhyBand(),
        "Incorrect operating PHY band for STA on link " << +staLinkId);
    NS_TEST_EXPECT_MSG_EQ(
        +m_staMacs[0]
             ->GetWifiPhy(staLinkId)
             ->GetOperatingChannel()
             .GetPrimaryChannelIndex(20),
        +m_apMac->GetWifiPhy(apLinkId)
             ->GetOperatingChannel()
             .GetPrimaryChannelIndex(20),
        "Incorrect operating primary channel index for STA on link "
            << +staLinkId);
  }

  auto checkStoredMapping = [this](Ptr<WifiMac> mac, Ptr<WifiMac> dest,
                                   WifiDirection dir, bool present) {
    NS_TEST_ASSERT_MSG_EQ(
        mac->GetTidToLinkMapping(dest->GetAddress(), dir).has_value(), present,
        "Link mapping stored by "
            << (mac->GetTypeOfStation() == AP ? "AP" : "non-AP") << " MLD for "
            << dir << " direction " << (present ? "expected" : "not expected"));
    if (present) {
      const auto &mapping =
          (dir == WifiDirection::DOWNLINK ? m_dlTidLinkMapping
                                          : m_ulTidLinkMapping);
      NS_TEST_EXPECT_MSG_EQ(
          (mac->GetTidToLinkMapping(dest->GetAddress(), dir)->get() == mapping),
          true,
          "Incorrect link mapping stored by "
              << (mac->GetTypeOfStation() == AP ? "AP" : "non-AP")
              << " MLD for " << dir << " direction");
    }
  };

  auto storedMapping = m_apMac->GetNLinks() > 1 &&
                       m_staMacs[0]->GetNLinks() > 1 && m_apNegSupport > 0;
  checkStoredMapping(m_apMac, m_staMacs[0], WifiDirection::DOWNLINK,
                     storedMapping);
  checkStoredMapping(m_apMac, m_staMacs[0], WifiDirection::UPLINK,
                     storedMapping);
  checkStoredMapping(m_staMacs[0], m_apMac, WifiDirection::DOWNLINK,
                     storedMapping);
  checkStoredMapping(m_staMacs[0], m_apMac, WifiDirection::UPLINK,
                     storedMapping);
}

void MultiLinkSetupTest::CheckDisabledLinks() {
  if (m_staMacs[0]->GetNLinks() == 1) {
    return;
  }

  for (const auto &linkId : m_staMacs[0]->GetLinkIds()) {
    auto it = std::find(m_setupLinks.begin(), m_setupLinks.end(), linkId);
    if (it == m_setupLinks.end()) {
      NS_TEST_EXPECT_MSG_EQ(
          m_staMacs[0]->GetWifiPhy(linkId)->GetState()->IsStateOff(), true,
          "Link " << +linkId << " has not been setup but is not disabled");
      continue;
    }

    NS_TEST_EXPECT_MSG_EQ(
        m_staMacs[0]->GetWifiPhy(linkId)->GetState()->IsStateOff(), false,
        "Expecting link " << +linkId << " to be active");
  }
}

void MultiLinkSetupTest::CheckQosData(Ptr<WifiMpdu> mpdu, uint8_t linkId,
                                      std::size_t index) {
  WifiDirection dir;
  const auto &hdr = mpdu->GetHeader();

  NS_TEST_ASSERT_MSG_EQ(hdr.IsQosData(), true, "Expected a QoS data frame");

  if (!hdr.IsToDs() && hdr.IsFromDs()) {
    dir = WifiDirection::DOWNLINK;
  } else if (hdr.IsToDs() && !hdr.IsFromDs()) {
    dir = WifiDirection::UPLINK;
  } else {
    NS_ABORT_MSG("Invalid combination for QoS data frame: ToDS("
                 << hdr.IsToDs() << ") FromDS(" << hdr.IsFromDs() << ")");
  }

  const auto &tid1 = (dir == WifiDirection::DOWNLINK) ? m_dlTid1 : m_ulTid1;
  const auto &tid2 = (dir == WifiDirection::DOWNLINK) ? m_dlTid2 : m_ulTid2;
  uint8_t tid = hdr.GetQosTid();

  NS_TEST_ASSERT_MSG_NE((tid == tid1), (tid2.has_value() && tid == *tid2),
                        "QoS frame with unexpected TID " << +tid);

  auto findLinkSet = [this, dir](uint8_t tid) -> std::set<uint8_t> {
    std::set<uint8_t> linkSet(m_setupLinks.cbegin(), m_setupLinks.cend());
    if (auto mappingOptRef =
            m_apMac->GetTidToLinkMapping(m_staMacs[0]->GetAddress(), dir)) {
      if (auto it = mappingOptRef->get().find(tid);
          it != mappingOptRef->get().cend()) {
        linkSet = it->second;
        NS_ASSERT_MSG(!linkSet.empty(), "TID " << +tid << " mapped to no link");
      }
    }
    return linkSet;
  };

  auto linkSet = findLinkSet(tid);
  auto &qosFrames = (tid == tid1) ? m_qosFrames1 : m_qosFrames2;

  std::size_t nConcurFrames = std::min(qosFrames.size(), linkSet.size());

  for (std::size_t i = 0; i < nConcurFrames; i++) {
    auto prev = qosFrames[i];

    auto band = m_apMac->GetWifiPhy(m_txPsdus[prev].linkId)->GetPhyBand();
    Time txDuration = WifiPhy::CalculateTxDuration(
        m_txPsdus[prev].psduMap, m_txPsdus[prev].txVector, band);

    if (qosFrames.size() < linkSet.size()) {
      NS_TEST_EXPECT_MSG_LT(
          m_txPsdus[index].startTx, m_txPsdus[prev].startTx + txDuration,
          "The " << dir << " QoS frame number " << qosFrames.size()
                 << " was not sent concurrently with others on link " << +linkId
                 << " which TID " << +tid << " is mapped to");
    } else if (m_txPsdus[prev].linkId == linkId) {
      NS_TEST_EXPECT_MSG_GT(
          m_txPsdus[index].startTx, m_txPsdus[prev].startTx + txDuration,
          "The " << dir << " QoS frame number " << qosFrames.size()
                 << " was sent concurrently with others on a link " << +linkId
                 << " which TID " << +tid << " is mapped to");
    }
  }

  if (m_apMac->GetNLinks() > 1 && m_staMacs[0]->GetNLinks() > 1) {
    NS_TEST_EXPECT_MSG_EQ(
        std::count(linkSet.cbegin(), linkSet.cend(), linkId), 1,
        "QoS frame sent on Link ID "
            << +linkId << " that does not belong to the link set of TID "
            << +tid);
  }

  if (tid2) {
    auto otherTid = (tid == tid1) ? *tid2 : tid1;
    const auto &otherQosFrames = (tid == tid1) ? m_qosFrames2 : m_qosFrames1;
    auto otherLinkSet = findLinkSet(otherTid);

    std::size_t nOtherConcurFrames =
        std::min(otherQosFrames.size(), otherLinkSet.size());

    for (std::size_t i = 0; i < nOtherConcurFrames; i++) {
      auto prev = otherQosFrames[i];

      auto band = m_apMac->GetWifiPhy(m_txPsdus[prev].linkId)->GetPhyBand();
      Time txDuration = WifiPhy::CalculateTxDuration(
          m_txPsdus[prev].psduMap, m_txPsdus[prev].txVector, band);

      if (qosFrames.size() < linkSet.size()) {
        NS_TEST_EXPECT_MSG_LT(
            m_txPsdus[index].startTx, m_txPsdus[prev].startTx + txDuration,
            "The " << dir << " QoS frame number " << qosFrames.size()
                   << " was not sent concurrently with others with TID "
                   << +otherTid);
      }
    }
  }

  qosFrames.emplace_back(index);

  if (qosFrames.size() == m_setupLinks.size()) {
    qosFrames.clear();
  }
}

enum class WifiTrafficPattern : uint8_t {
  STA_TO_STA = 0,
  STA_TO_AP,
  AP_TO_STA,
  AP_TO_BCAST,
  STA_TO_BCAST
};

enum class WifiBaEnabled : uint8_t { NO = 0, YES };

enum class WifiUseBarAfterMissedBa : uint8_t { NO = 0, YES };

class MultiLinkTxTest : public MultiLinkOperationsTestBase {
public:
  MultiLinkTxTest(const BaseParams &baseParams,
                  WifiTrafficPattern trafficPattern, WifiBaEnabled baEnabled,
                  WifiUseBarAfterMissedBa useBarAfterMissedBa,
                  uint8_t nMaxInflight);
  ~MultiLinkTxTest() override = default;

protected:
  void CheckBlockAck(Ptr<const WifiPsdu> psdu, const WifiTxVector &txVector,
                     uint8_t linkId);

  void Transmit(Ptr<WifiMac> mac, uint8_t phyId, WifiConstPsduMap psduMap,
                WifiTxVector txVector, double txPowerW) override;
  void DoSetup() override;
  void DoRun() override;

private:
  void StartTraffic() override;

  using RxErrorModelMap =
      std::unordered_map<Mac48Address, Ptr<ListErrorModel>, WifiAddressHash>;

  RxErrorModelMap m_errorModels;
  std::list<uint64_t> m_uidList;
  bool m_dataCorrupted{false};
  WifiTrafficPattern m_trafficPattern;
  bool m_baEnabled;
  bool m_useBarAfterMissedBa;
  std::size_t m_nMaxInflight;
  std::size_t m_nPackets;
  std::size_t m_blockAckCount{0};
  std::size_t m_blockAckReqCount{0};
  std::map<uint16_t, std::size_t> m_inflightCount;
  Ptr<WifiMac> m_sourceMac;
};

MultiLinkTxTest::MultiLinkTxTest(const BaseParams &baseParams,
                                 WifiTrafficPattern trafficPattern,
                                 WifiBaEnabled baEnabled,
                                 WifiUseBarAfterMissedBa useBarAfterMissedBa,
                                 uint8_t nMaxInflight)
    : MultiLinkOperationsTestBase(
          std::string("Check data transmission between MLDs ") +
              (baEnabled == WifiBaEnabled::YES
                   ? (useBarAfterMissedBa == WifiUseBarAfterMissedBa::YES
                          ? "with BA agreement, send BAR after BlockAck timeout"
                          : "with BA agreement, send Data frames after "
                            "BlockAck timeout")
                   : "without BA agreement") +
              " (Traffic pattern: " +
              std::to_string(static_cast<uint8_t>(trafficPattern)) +
              (baEnabled == WifiBaEnabled::YES
                   ? ", nMaxInflight=" + std::to_string(nMaxInflight)
                   : "") +
              ")",
          2, baseParams),
      m_trafficPattern(trafficPattern),
      m_baEnabled(baEnabled == WifiBaEnabled::YES),
      m_useBarAfterMissedBa(useBarAfterMissedBa ==
                            WifiUseBarAfterMissedBa::YES),
      m_nMaxInflight(nMaxInflight),
      m_nPackets(trafficPattern == WifiTrafficPattern::STA_TO_BCAST ||
                         trafficPattern == WifiTrafficPattern::STA_TO_STA
                     ? 4
                     : 8) {}

void MultiLinkTxTest::Transmit(Ptr<WifiMac> mac, uint8_t phyId,
                               WifiConstPsduMap psduMap, WifiTxVector txVector,
                               double txPowerW) {
  MultiLinkOperationsTestBase::Transmit(mac, phyId, psduMap, txVector,
                                        txPowerW);
  auto linkId = m_txPsdus.back().linkId;

  auto psdu = psduMap.begin()->second;

  switch (psdu->GetHeader(0).GetType()) {
  case WIFI_MAC_MGT_ACTION:
    CheckAddresses(psdu, psdu->GetHeader(0).GetAddr2() ==
                                 psdu->GetHeader(0).GetAddr3()
                             ? DL
                             : UL);
    if (!m_baEnabled) {
      m_uidList.push_front(psdu->GetPacket()->GetUid());
      m_errorModels.at(psdu->GetAddr1())->SetList(m_uidList);
      NS_LOG_INFO("CORRUPTED");
    }
    break;
  case WIFI_MAC_QOSDATA:
    CheckAddresses(psdu);

    for (const auto &mpdu : *psdu) {
      if (m_baEnabled && m_sourceMac->GetLinkIds().count(linkId) == 1 &&
          m_sourceMac->GetFrameExchangeManager(linkId)->GetAddress() ==
              mpdu->GetHeader().GetAddr2() &&
          !mpdu->GetHeader().GetAddr1().IsGroup()) {
        auto seqNo = mpdu->GetHeader().GetSequenceNumber();
        auto [it, success] =
            m_inflightCount.insert({seqNo, mpdu->GetInFlightLinkIds().size()});
        if (!success) {
          it->second = std::max(it->second, mpdu->GetInFlightLinkIds().size());
        }
      }
    }
    for (std::size_t i = 0; i < psdu->GetNMpdus(); i++) {
      if (psdu->GetHeader(i).GetSequenceNumber() != 1 ||
          m_trafficPattern == WifiTrafficPattern::AP_TO_BCAST ||
          m_trafficPattern == WifiTrafficPattern::STA_TO_BCAST ||
          m_trafficPattern == WifiTrafficPattern::STA_TO_STA) {
        continue;
      }
      auto uid = psdu->GetPayload(i)->GetUid();
      if (!m_dataCorrupted) {
        m_uidList.push_front(uid);
        m_dataCorrupted = true;
        NS_LOG_INFO("CORRUPTED");
        m_errorModels.at(psdu->GetAddr1())->SetList(m_uidList);
      } else {
        if (auto it = std::find(m_uidList.cbegin(), m_uidList.cend(), uid);
            it != m_uidList.cend()) {
          m_uidList.erase(it);
        }
        m_errorModels.at(psdu->GetAddr1())->SetList(m_uidList);
      }
      break;
    }
    break;
  case WIFI_MAC_CTL_BACKRESP: {
    if (!m_sourceMac->GetLinkIdByAddress(psdu->GetHeader(0).GetAddr1())) {
      break;
    }
    if (m_nMaxInflight > 1) {
      break;
    }
    CheckBlockAck(psdu, txVector, linkId);
    m_blockAckCount++;
    if (m_blockAckCount == 2) {
      m_uidList.push_front(psdu->GetPacket()->GetUid());
      NS_LOG_INFO("CORRUPTED");
      m_errorModels.at(psdu->GetAddr1())->SetList(m_uidList);
    }
    break;
  case WIFI_MAC_CTL_BACKREQ:
    if (m_sourceMac->GetLinkIdByAddress(psdu->GetHeader(0).GetAddr2())) {
      m_blockAckReqCount++;
    }
    break;
  }
  default:;
  }
}

void MultiLinkTxTest::CheckBlockAck(Ptr<const WifiPsdu> psdu,
                                    const WifiTxVector &txVector,
                                    uint8_t linkId) {
  NS_TEST_ASSERT_MSG_EQ(m_baEnabled, true,
                        "No BlockAck expected without BA agreement");
  NS_TEST_ASSERT_MSG_EQ(
      (m_trafficPattern != WifiTrafficPattern::AP_TO_BCAST), true,
      "No BlockAck expected in AP to broadcast traffic pattern");

  auto mpdu = *psdu->begin();
  CtrlBAckResponseHeader blockAck;
  mpdu->GetPacket()->PeekHeader(blockAck);
  bool isMpdu1corrupted = (m_trafficPattern == WifiTrafficPattern::STA_TO_AP ||
                           m_trafficPattern == WifiTrafficPattern::AP_TO_STA);

  switch (m_blockAckCount) {
  case 0:
    NS_TEST_EXPECT_MSG_EQ(blockAck.IsPacketReceived(0), true,
                          "MPDU 0 expected to be successfully received");
    NS_TEST_EXPECT_MSG_EQ(blockAck.IsPacketReceived(1), !isMpdu1corrupted,
                          "MPDU 1 expected to be received only in "
                          "STA_TO_STA/STA_TO_BCAST scenarios");
    if (m_staMacs[0]->GetSetupLinkIds().size() > 1) {
      auto queue = m_sourceMac->GetTxopQueue(AC_BE);
      auto rcvMac = m_sourceMac == m_staMacs[0]
                        ? StaticCast<WifiMac>(m_apMac)
                        : StaticCast<WifiMac>(m_staMacs[1]);
      auto item = queue->PeekByTidAndAddress(0, rcvMac->GetAddress());
      std::size_t nQueuedPkt = 0;
      auto delay =
          WifiPhy::CalculateTxDuration(
              psdu, txVector, rcvMac->GetWifiPhy(linkId)->GetPhyBand()) +
          MicroSeconds(1);

      while (item) {
        auto seqNo = item->GetHeader().GetSequenceNumber();
        NS_TEST_EXPECT_MSG_EQ(item->IsInFlight(), true,
                              "MPDU with seqNo=" << seqNo
                                                 << " is not in flight");
        auto linkIds = item->GetInFlightLinkIds();
        NS_TEST_EXPECT_MSG_EQ(
            linkIds.size(), 1,
            "MPDU with seqNo=" << seqNo << " is in flight on multiple links");
        auto srcLinkId =
            m_sourceMac->GetLinkIdByAddress(mpdu->GetHeader().GetAddr1());
        NS_TEST_ASSERT_MSG_EQ(
            srcLinkId.has_value(), true,
            "Addr1 of BlockAck is not an originator's link address");
        NS_TEST_EXPECT_MSG_EQ(
            (*linkIds.begin() == *srcLinkId), (seqNo <= 1),
            "MPDU with seqNo=" << seqNo << " in flight on unexpected link");

        bool isQueued = (seqNo > (isMpdu1corrupted ? 0 : 1));
        bool isRetry = isQueued && seqNo <= 1;

        Simulator::Schedule(delay, [this, item, isQueued, isRetry]() {
          NS_TEST_EXPECT_MSG_EQ(item->IsQueued(), isQueued,
                                "MPDU with seqNo="
                                    << item->GetHeader().GetSequenceNumber()
                                    << " should " << (isQueued ? "" : "not")
                                    << " be queued");
          NS_TEST_EXPECT_MSG_EQ(
              item->GetHeader().IsRetry(), isRetry,
              "Unexpected value for the Retry subfield of the MPDU with seqNo="
                  << item->GetHeader().GetSequenceNumber());
        });

        nQueuedPkt++;
        item = queue->PeekByTidAndAddress(0, rcvMac->GetAddress(), item);
      }
      NS_TEST_EXPECT_MSG_EQ(nQueuedPkt, m_nPackets / 2,
                            "Unexpected number of queued MPDUs");
    }
    break;
  case 1:
  case 2:
    NS_TEST_EXPECT_MSG_EQ((m_trafficPattern == WifiTrafficPattern::AP_TO_STA ||
                           m_trafficPattern == WifiTrafficPattern::STA_TO_AP),
                          true, "Did not expect to receive a second BlockAck");
    std::pair<uint16_t, uint16_t> seqNos;
    if (m_staMacs[0]->GetSetupLinkIds().size() > 1) {
      seqNos = {2, 3};
    } else {
      seqNos = {1, 2};
    }
    NS_TEST_EXPECT_MSG_EQ(blockAck.IsPacketReceived(seqNos.first), true,
                          "MPDU " << seqNos.first
                                  << " expected to be successfully received");
    NS_TEST_EXPECT_MSG_EQ(blockAck.IsPacketReceived(seqNos.second), true,
                          "MPDU " << seqNos.second
                                  << " expected to be successfully received");
    break;
  }
}

void MultiLinkTxTest::DoSetup() {
  MultiLinkOperationsTestBase::DoSetup();

  if (m_baEnabled) {
    for (auto mac : std::initializer_list<Ptr<WifiMac>>{m_apMac, m_staMacs[0],
                                                        m_staMacs[1]}) {
      mac->SetAttribute("BE_MaxAmsduSize", UintegerValue(2100));
      mac->GetQosTxop(AC_BE)->SetAttribute("UseExplicitBarAfterMissedBlockAck",
                                           BooleanValue(m_useBarAfterMissedBa));
      mac->GetQosTxop(AC_BE)->SetAttribute("NMaxInflights",
                                           UintegerValue(m_nMaxInflight));
    }
  }

  for (std::size_t linkId = 0; linkId < m_apMac->GetNLinks(); linkId++) {
    auto errorModel = CreateObject<ListErrorModel>();
    m_errorModels[m_apMac->GetFrameExchangeManager(linkId)->GetAddress()] =
        errorModel;
    m_apMac->GetWifiPhy(linkId)->SetPostReceptionErrorModel(errorModel);
  }
  for (std::size_t i : {0, 1}) {
    for (const auto linkId : m_staMacs[i]->GetLinkIds()) {
      auto errorModel = CreateObject<ListErrorModel>();
      m_errorModels
          [m_staMacs[i]->GetFrameExchangeManager(linkId)->GetAddress()] =
              errorModel;
      m_staMacs[i]->GetWifiPhy(linkId)->SetPostReceptionErrorModel(errorModel);
    }
  }
}

void MultiLinkTxTest::StartTraffic() {
  Address destAddr;

  switch (m_trafficPattern) {
  case WifiTrafficPattern::STA_TO_STA:
    m_sourceMac = m_staMacs[0];
    destAddr = m_staMacs[1]->GetDevice()->GetAddress();
    break;
  case WifiTrafficPattern::STA_TO_AP:
    m_sourceMac = m_staMacs[0];
    destAddr = m_apMac->GetDevice()->GetAddress();
    break;
  case WifiTrafficPattern::AP_TO_STA:
    m_sourceMac = m_apMac;
    destAddr = m_staMacs[1]->GetDevice()->GetAddress();
    break;
  case WifiTrafficPattern::AP_TO_BCAST:
    m_sourceMac = m_apMac;
    destAddr = Mac48Address::GetBroadcast();
    break;
  case WifiTrafficPattern::STA_TO_BCAST:
    m_sourceMac = m_staMacs[0];
    destAddr = Mac48Address::GetBroadcast();
    break;
  }

  PacketSocketAddress sockAddr;
  sockAddr.SetSingleDevice(m_sourceMac->GetDevice()->GetIfIndex());
  sockAddr.SetPhysicalAddress(destAddr);
  sockAddr.SetProtocol(1);

  m_sourceMac->GetDevice()->GetNode()->AddApplication(
      GetApplication(sockAddr, std::min<std::size_t>(m_nPackets, 4), 1000));

  if (m_nPackets > 4) {
    m_sourceMac->GetDevice()->GetNode()->AddApplication(
        GetApplication(sockAddr, m_nPackets - 4, 1000, MilliSeconds(4)));
  }

  Simulator::Stop(m_duration);
}

void MultiLinkTxTest::DoRun() {
  Simulator::Run();

  std::array<std::size_t, 3> expectedRxPkts{};

  switch (m_trafficPattern) {
  case WifiTrafficPattern::STA_TO_STA:
  case WifiTrafficPattern::AP_TO_STA:
    expectedRxPkts[2] = m_nPackets;
    break;
  case WifiTrafficPattern::STA_TO_AP:
    expectedRxPkts[0] = m_nPackets;
    break;
  case WifiTrafficPattern::AP_TO_BCAST:
    expectedRxPkts[1] = m_nPackets * m_staMacs[0]->GetSetupLinkIds().size();
    expectedRxPkts[2] = m_nPackets * m_staMacs[1]->GetSetupLinkIds().size();
    break;
  case WifiTrafficPattern::STA_TO_BCAST:
    expectedRxPkts[0] = m_nPackets;
    expectedRxPkts[2] = m_nPackets * m_staMacs[1]->GetSetupLinkIds().size();
    break;
  }

  NS_TEST_EXPECT_MSG_EQ(+m_rxPkts[0], +expectedRxPkts[0],
                        "Unexpected number of packets received by the AP");
  NS_TEST_EXPECT_MSG_EQ(+m_rxPkts[1], +expectedRxPkts[1],
                        "Unexpected number of packets received by STA 0");
  NS_TEST_EXPECT_MSG_EQ(+m_rxPkts[2], +expectedRxPkts[2],
                        "Unexpected number of packets received by STA 1");

  if (m_baEnabled && m_nMaxInflight == 1) {
    std::size_t expectedBaCount = 0;
    std::size_t expectedBarCount = 0;

    switch (m_trafficPattern) {
    case WifiTrafficPattern::STA_TO_AP:
    case WifiTrafficPattern::AP_TO_STA:
      expectedBaCount = 3;
      expectedBarCount = m_useBarAfterMissedBa ? 1 : 0;
      break;
    case WifiTrafficPattern::STA_TO_STA:
    case WifiTrafficPattern::STA_TO_BCAST:
      expectedBaCount = 1;
      break;
    default:;
    }
    NS_TEST_EXPECT_MSG_EQ(m_blockAckCount, expectedBaCount,
                          "Unexpected number of BlockAck frames");
    NS_TEST_EXPECT_MSG_EQ(m_blockAckReqCount, expectedBarCount,
                          "Unexpected number of BlockAckReq frames");
  }

  if (m_baEnabled && m_trafficPattern != WifiTrafficPattern::AP_TO_BCAST) {
    NS_TEST_EXPECT_MSG_EQ(m_inflightCount.size(), m_nPackets / 2,
                          "Did not collect number of simultaneous "
                          "transmissions for all data frames");

    auto nMaxInflight =
        std::min(m_nMaxInflight, m_staMacs[0]->GetSetupLinkIds().size());
    std::size_t maxCount = 0;
    for (const auto &[seqNo, count] : m_inflightCount) {
      NS_TEST_EXPECT_MSG_LT_OR_EQ(
          count, nMaxInflight,
          "MPDU with seqNo="
              << seqNo
              << " transmitted simultaneously more times than allowed");
      maxCount = std::max(maxCount, count);
    }

    NS_TEST_EXPECT_MSG_EQ(maxCount, nMaxInflight,
                          "Expected that at least one data frame was "
                          "transmitted simultaneously a number of "
                          "times equal to the NMaxInflights attribute");
  }

  Simulator::Destroy();
}

enum class WifiMuTrafficPattern : uint8_t {
  DL_MU_BAR_BA_SEQUENCE = 0,
  DL_MU_MU_BAR,
  DL_MU_AGGR_MU_BAR,
  UL_MU
};

class MultiLinkMuTxTest : public MultiLinkOperationsTestBase {
public:
  MultiLinkMuTxTest(const BaseParams &baseParams,
                    WifiMuTrafficPattern muTrafficPattern,
                    WifiUseBarAfterMissedBa useBarAfterMissedBa,
                    uint8_t nMaxInflight);
  ~MultiLinkMuTxTest() override = default;

protected:
  void CheckBlockAck(Ptr<const WifiPsdu> psdu, const WifiTxVector &txVector,
                     uint8_t linkId);

  void Transmit(Ptr<WifiMac> mac, uint8_t phyId, WifiConstPsduMap psduMap,
                WifiTxVector txVector, double txPowerW) override;
  void DoSetup() override;
  void DoRun() override;

private:
  void StartTraffic() override;

  using RxErrorModelMap =
      std::unordered_map<Mac48Address, Ptr<ListErrorModel>, WifiAddressHash>;

  using AddrSeqNoPair = std::pair<Mac48Address, uint16_t>;

  RxErrorModelMap m_errorModels;
  std::list<uint64_t> m_uidList;
  std::optional<Mac48Address> m_dataCorruptedSta;
  bool m_waitFirstTf{true};
  WifiMuTrafficPattern m_muTrafficPattern;
  bool m_useBarAfterMissedBa;
  std::size_t m_nMaxInflight;
  std::vector<PacketSocketAddress> m_sockets;
  std::size_t m_nPackets;
  std::size_t m_blockAckCount{0};
  std::map<AddrSeqNoPair, std::size_t> m_inflightCount;
  Ptr<WifiMac> m_sourceMac;
};

MultiLinkMuTxTest::MultiLinkMuTxTest(
    const BaseParams &baseParams, WifiMuTrafficPattern muTrafficPattern,
    WifiUseBarAfterMissedBa useBarAfterMissedBa, uint8_t nMaxInflight)
    : MultiLinkOperationsTestBase(
          std::string("Check MU data transmission between MLDs ") +
              (useBarAfterMissedBa == WifiUseBarAfterMissedBa::YES
                   ? "(send BAR after BlockAck timeout,"
                   : "(send Data frames after BlockAck timeout,") +
              " MU Traffic pattern: " +
              std::to_string(static_cast<uint8_t>(muTrafficPattern)) +
              ", nMaxInflight=" + std::to_string(nMaxInflight) + ")",
          2, baseParams),
      m_muTrafficPattern(muTrafficPattern),
      m_useBarAfterMissedBa(useBarAfterMissedBa ==
                            WifiUseBarAfterMissedBa::YES),
      m_nMaxInflight(nMaxInflight), m_sockets(m_nStations),
      m_nPackets(muTrafficPattern == WifiMuTrafficPattern::UL_MU ? 4 : 8) {}

void MultiLinkMuTxTest::Transmit(Ptr<WifiMac> mac, uint8_t phyId,
                                 WifiConstPsduMap psduMap,
                                 WifiTxVector txVector, double txPowerW) {
  MultiLinkOperationsTestBase::Transmit(mac, phyId, psduMap, txVector,
                                        txPowerW);
  auto linkId = m_txPsdus.back().linkId;

  CtrlTriggerHeader trigger;

  for (const auto &[staId, psdu] : psduMap) {
    switch (psdu->GetHeader(0).GetType()) {
    case WIFI_MAC_QOSDATA:
      CheckAddresses(psdu);
      if (psdu->GetHeader(0).HasData()) {
        bool isDl = psdu->GetHeader(0).IsFromDs();
        auto linkAddress = isDl ? psdu->GetHeader(0).GetAddr1()
                                : psdu->GetHeader(0).GetAddr2();
        auto address =
            m_apMac->GetMldAddress(linkAddress).value_or(linkAddress);

        for (const auto &mpdu : *psdu) {
          auto seqNo = mpdu->GetHeader().GetSequenceNumber();
          auto [it, success] = m_inflightCount.insert(
              {{address, seqNo}, mpdu->GetInFlightLinkIds().size()});
          if (!success) {
            it->second =
                std::max(it->second, mpdu->GetInFlightLinkIds().size());
          }
        }
        for (std::size_t i = 0; i < psdu->GetNMpdus(); i++) {
          if (psdu->GetHeader(i).GetSequenceNumber() == 2) {
            if (m_muTrafficPattern == WifiMuTrafficPattern::UL_MU) {
              NS_TEST_EXPECT_MSG_EQ(txVector.IsUlMu(), true,
                                    "MPDU " << **std::next(psdu->begin(), i)
                                            << " not transmitted in a TB PPDU");
            } else {
              NS_TEST_EXPECT_MSG_EQ(txVector.GetHeMuUserInfoMap().size(), 2,
                                    "MPDU "
                                        << **std::next(psdu->begin(), i)
                                        << " not transmitted in a DL MU PPDU");
            }
          }
          if (psdu->GetHeader(i).GetSequenceNumber() != 3) {
            continue;
          }
          auto uid = psdu->GetPayload(i)->GetUid();
          if (!m_dataCorruptedSta) {
            m_uidList.push_front(uid);
            m_dataCorruptedSta = isDl ? psdu->GetAddr1() : psdu->GetAddr2();
            NS_LOG_INFO("CORRUPTED");
            m_errorModels.at(psdu->GetAddr1())->SetList(m_uidList);
          } else if ((isDl && m_dataCorruptedSta == psdu->GetAddr1()) ||
                     (!isDl && m_dataCorruptedSta == psdu->GetAddr2())) {
            if (auto it = std::find(m_uidList.cbegin(), m_uidList.cend(), uid);
                it != m_uidList.cend()) {
              m_uidList.erase(it);
            }
            m_errorModels.at(psdu->GetAddr1())->SetList(m_uidList);
          }
          break;
        }
      }
      break;
    case WIFI_MAC_CTL_BACKRESP:
      if (m_nMaxInflight > 1) {
        break;
      }
      CheckBlockAck(psdu, txVector, linkId);
      m_blockAckCount++;
      if (m_blockAckCount == 5) {
        m_uidList.push_front(psdu->GetPacket()->GetUid());
        NS_LOG_INFO("CORRUPTED");
        m_errorModels.at(psdu->GetAddr1())->SetList(m_uidList);
      }
      break;
    case WIFI_MAC_CTL_TRIGGER:
      psdu->GetPayload(0)->PeekHeader(trigger);
      if (trigger.IsBasic() && m_waitFirstTf) {
        m_waitFirstTf = false;
        auto band = mac->GetWifiPhy(linkId)->GetPhyBand();
        Time txDuration = WifiPhy::CalculateTxDuration(psduMap, txVector, band);
        for (uint8_t i = 0; i < m_nStations; i++) {
          m_staMacs[i]->GetDevice()->GetNode()->AddApplication(
              GetApplication(m_sockets[i], m_nPackets, 450, txDuration, i * 4));
        }
      }
      break;
    default:;
    }
  }
}

void MultiLinkMuTxTest::CheckBlockAck(Ptr<const WifiPsdu> psdu,
                                      const WifiTxVector &txVector,
                                      uint8_t linkId) {
  auto mpdu = *psdu->begin();
  CtrlBAckResponseHeader blockAck;
  mpdu->GetPacket()->PeekHeader(blockAck);
  bool isMpdu3corrupted;

  switch (m_blockAckCount) {
  case 0:
  case 1:
    break;
  case 2:
    if (m_muTrafficPattern == WifiMuTrafficPattern::UL_MU) {
      NS_TEST_EXPECT_MSG_EQ(blockAck.IsMultiSta(), true,
                            "Expected a Multi-STA BlockAck");
      for (uint8_t i = 0; i < m_nStations; i++) {
        auto indices =
            blockAck.FindPerAidTidInfoWithAid(m_staMacs[i]->GetAssociationId());
        NS_TEST_ASSERT_MSG_EQ(indices.size(), 1,
                              "Expected one Per AID TID Info per STA");
        auto index = indices.front();
        NS_TEST_ASSERT_MSG_EQ(m_dataCorruptedSta.has_value(), true,
                              "Expected that a QoS data frame was corrupted");
        isMpdu3corrupted =
            m_staMacs[i]->GetLinkIdByAddress(*m_dataCorruptedSta).has_value();
        NS_TEST_EXPECT_MSG_EQ(blockAck.IsPacketReceived(2, index), true,
                              "MPDU 2 expected to be successfully received");
        NS_TEST_EXPECT_MSG_EQ(blockAck.IsPacketReceived(3, index),
                              !isMpdu3corrupted,
                              "Unexpected reception status for MPDU 3");
      }

      break;
    }
  case 3:
    isMpdu3corrupted = (mpdu->GetHeader().GetAddr2() == m_dataCorruptedSta);
    NS_TEST_EXPECT_MSG_EQ(blockAck.IsPacketReceived(2), true,
                          "MPDU 2 expected to be successfully received");
    NS_TEST_EXPECT_MSG_EQ(blockAck.IsPacketReceived(3), !isMpdu3corrupted,
                          "Unexpected reception status for MPDU 3");
    if (m_muTrafficPattern != WifiMuTrafficPattern::UL_MU &&
        m_staMacs[0]->GetSetupLinkIds().size() > 1) {
      auto queue = m_apMac->GetTxopQueue(AC_BE);
      Ptr<StaWifiMac> rcvMac;
      if (m_staMacs[0]->GetFrameExchangeManager(linkId)->GetAddress() ==
          mpdu->GetHeader().GetAddr2()) {
        rcvMac = m_staMacs[0];
      } else if (m_staMacs[1]->GetFrameExchangeManager(linkId)->GetAddress() ==
                 mpdu->GetHeader().GetAddr2()) {
        rcvMac = m_staMacs[1];
      } else {
        NS_ABORT_MSG("BlockAck frame not sent by a station in DL scenario");
      }
      auto item = queue->PeekByTidAndAddress(0, rcvMac->GetAddress());
      std::size_t nQueuedPkt = 0;
      auto delay =
          WifiPhy::CalculateTxDuration(
              psdu, txVector, rcvMac->GetWifiPhy(linkId)->GetPhyBand()) +
          MicroSeconds(1);

      while (item) {
        auto seqNo = item->GetHeader().GetSequenceNumber();
        NS_TEST_EXPECT_MSG_EQ(item->IsInFlight(), true,
                              "MPDU with seqNo=" << seqNo
                                                 << " is not in flight");
        auto linkIds = item->GetInFlightLinkIds();
        NS_TEST_EXPECT_MSG_EQ(
            linkIds.size(), 1,
            "MPDU with seqNo=" << seqNo << " is in flight on multiple links");
        auto srcLinkId =
            m_apMac->GetLinkIdByAddress(mpdu->GetHeader().GetAddr1());
        NS_TEST_ASSERT_MSG_EQ(
            srcLinkId.has_value(), true,
            "Addr1 of BlockAck is not an originator's link address");
        NS_TEST_EXPECT_MSG_EQ(
            (*linkIds.begin() == *srcLinkId), (seqNo <= 3),
            "MPDU with seqNo=" << seqNo << " in flight on unexpected link");

        bool isQueued = (seqNo > (isMpdu3corrupted ? 2 : 3));
        bool isRetry = isQueued && seqNo <= 3;

        Simulator::Schedule(delay, [this, item, isQueued, isRetry]() {
          NS_TEST_EXPECT_MSG_EQ(item->IsQueued(), isQueued,
                                "MPDU with seqNo="
                                    << item->GetHeader().GetSequenceNumber()
                                    << " should " << (isQueued ? "" : "not")
                                    << " be queued");
          NS_TEST_EXPECT_MSG_EQ(
              item->GetHeader().IsRetry(), isRetry,
              "Unexpected value for the Retry subfield of the MPDU with seqNo="
                  << item->GetHeader().GetSequenceNumber());
        });

        nQueuedPkt++;
        item = queue->PeekByTidAndAddress(0, rcvMac->GetAddress(), item);
      }
      NS_TEST_EXPECT_MSG_EQ(nQueuedPkt, m_nPackets / 2,
                            "Unexpected number of queued MPDUs");
    }
    break;
  }
}

void MultiLinkMuTxTest::DoSetup() {
  switch (m_muTrafficPattern) {
  case WifiMuTrafficPattern::DL_MU_BAR_BA_SEQUENCE:
    Config::SetDefault("ns3::WifiDefaultAckManager::DlMuAckSequenceType",
                       EnumValue(WifiAcknowledgment::DL_MU_BAR_BA_SEQUENCE));
    break;
  case WifiMuTrafficPattern::DL_MU_MU_BAR:
    Config::SetDefault("ns3::WifiDefaultAckManager::DlMuAckSequenceType",
                       EnumValue(WifiAcknowledgment::DL_MU_TF_MU_BAR));
    break;
  case WifiMuTrafficPattern::DL_MU_AGGR_MU_BAR:
    Config::SetDefault("ns3::WifiDefaultAckManager::DlMuAckSequenceType",
                       EnumValue(WifiAcknowledgment::DL_MU_AGGREGATE_TF));
    break;
  default:;
  }

  MultiLinkOperationsTestBase::DoSetup();

  for (auto mac : std::initializer_list<Ptr<WifiMac>>{m_apMac, m_staMacs[0],
                                                      m_staMacs[1]}) {
    mac->SetAttribute("BE_MaxAmsduSize", UintegerValue(1050));
    mac->GetQosTxop(AC_BE)->SetAttribute("UseExplicitBarAfterMissedBlockAck",
                                         BooleanValue(m_useBarAfterMissedBa));
    mac->GetQosTxop(AC_BE)->SetAttribute("NMaxInflights",
                                         UintegerValue(m_nMaxInflight));

    mac->SetAttribute("VI_MaxAmsduSize", UintegerValue(1050));
    mac->GetQosTxop(AC_VI)->SetAttribute("UseExplicitBarAfterMissedBlockAck",
                                         BooleanValue(m_useBarAfterMissedBa));
    mac->GetQosTxop(AC_VI)->SetAttribute("NMaxInflights",
                                         UintegerValue(m_nMaxInflight));
  }

  auto muScheduler = CreateObjectWithAttributes<RrMultiUserScheduler>(
      "EnableUlOfdma",
      BooleanValue(m_muTrafficPattern == WifiMuTrafficPattern::UL_MU),
      "EnableBsrp", BooleanValue(false), "UlPsduSize", UintegerValue(2000));
  m_apMac->AggregateObject(muScheduler);

  for (std::size_t linkId = 0; linkId < m_apMac->GetNLinks(); linkId++) {
    auto errorModel = CreateObject<ListErrorModel>();
    m_errorModels[m_apMac->GetFrameExchangeManager(linkId)->GetAddress()] =
        errorModel;
    m_apMac->GetWifiPhy(linkId)->SetPostReceptionErrorModel(errorModel);
  }
  for (std::size_t i : {0, 1}) {
    for (const auto linkId : m_staMacs[i]->GetLinkIds()) {
      auto errorModel = CreateObject<ListErrorModel>();
      m_errorModels
          [m_staMacs[i]->GetFrameExchangeManager(linkId)->GetAddress()] =
              errorModel;
      m_staMacs[i]->GetWifiPhy(linkId)->SetPostReceptionErrorModel(errorModel);
    }
  }
}

void MultiLinkMuTxTest::StartTraffic() {
  if (m_muTrafficPattern < WifiMuTrafficPattern::UL_MU) {
    for (uint8_t i = 0; i < m_nStations; i++) {
      PacketSocketAddress sockAddr;
      sockAddr.SetSingleDevice(m_apMac->GetDevice()->GetIfIndex());
      sockAddr.SetPhysicalAddress(m_staMacs[i]->GetDevice()->GetAddress());
      sockAddr.SetProtocol(1);

      m_apMac->GetDevice()->GetNode()->AddApplication(
          GetApplication(sockAddr, 3, 450, i * MilliSeconds(50)));

      m_apMac->GetDevice()->GetNode()->AddApplication(GetApplication(
          sockAddr, m_nPackets / 2, 450, m_nStations * MilliSeconds(50)));

      m_apMac->GetDevice()->GetNode()->AddApplication(
          GetApplication(sockAddr, m_nPackets / 2, 450,
                         m_nStations * MilliSeconds(50) + MilliSeconds(3)));
    }
  } else {
    for (uint8_t i = 0; i < m_nStations; i++) {
      m_sockets[i].SetSingleDevice(m_staMacs[i]->GetDevice()->GetIfIndex());
      m_sockets[i].SetPhysicalAddress(m_apMac->GetDevice()->GetAddress());
      m_sockets[i].SetProtocol(1);

      m_staMacs[i]->GetDevice()->GetNode()->AddApplication(
          GetApplication(m_sockets[i], 3, 450, i * MilliSeconds(50), i * 4));
    }

    Simulator::Schedule(m_nStations * MilliSeconds(50), [this]() {
      auto muScheduler = m_apMac->GetObject<MultiUserScheduler>();
      NS_TEST_ASSERT_MSG_NE(muScheduler, nullptr,
                            "Expected an aggregated MU scheduler");
      muScheduler->SetAccessReqInterval(MilliSeconds(3));
      muScheduler->SetAccessReqInterval(Seconds(0));
    });
  }

  Simulator::Stop(m_duration);
}

void MultiLinkMuTxTest::DoRun() {
  Simulator::Run();

  std::array<std::size_t, 3> expectedRxPkts{};

  switch (m_muTrafficPattern) {
  case WifiMuTrafficPattern::DL_MU_BAR_BA_SEQUENCE:
  case WifiMuTrafficPattern::DL_MU_MU_BAR:
  case WifiMuTrafficPattern::DL_MU_AGGR_MU_BAR:
    expectedRxPkts[1] = m_nPackets + 3;
    expectedRxPkts[2] = m_nPackets + 3;
    break;
  case WifiMuTrafficPattern::UL_MU:
    expectedRxPkts[0] = 2 * (m_nPackets + 3);
    break;
  }

  NS_TEST_EXPECT_MSG_EQ(+m_rxPkts[0], +expectedRxPkts[0],
                        "Unexpected number of packets received by the AP");
  NS_TEST_EXPECT_MSG_EQ(+m_rxPkts[1], +expectedRxPkts[1],
                        "Unexpected number of packets received by STA 0");
  NS_TEST_EXPECT_MSG_EQ(+m_rxPkts[2], +expectedRxPkts[2],
                        "Unexpected number of packets received by STA 1");

  NS_TEST_EXPECT_MSG_EQ(m_inflightCount.size(), 2 * (2 + m_nPackets / 2),
                        "Did not collect number of simultaneous transmissions "
                        "for all data frames");

  auto nMaxInflight =
      std::min(m_nMaxInflight, m_staMacs[0]->GetSetupLinkIds().size());
  std::size_t maxCount = 0;
  for (const auto &[txSeqNoPair, count] : m_inflightCount) {
    NS_TEST_EXPECT_MSG_LT_OR_EQ(
        count, nMaxInflight,
        "MPDU with seqNo="
            << txSeqNoPair.second
            << " transmitted simultaneously more times than allowed");
    maxCount = std::max(maxCount, count);
  }

  NS_TEST_EXPECT_MSG_EQ(maxCount, nMaxInflight,
                        "Expected that at least one data frame was transmitted "
                        "simultaneously a number of "
                        "times equal to the NMaxInflights attribute");

  Simulator::Destroy();
}

class ReleaseSeqNoAfterCtsTimeoutTest : public MultiLinkOperationsTestBase {
public:
  ReleaseSeqNoAfterCtsTimeoutTest();
  ~ReleaseSeqNoAfterCtsTimeoutTest() override = default;

protected:
  void DoSetup() override;
  void DoRun() override;
  void Transmit(Ptr<WifiMac> mac, uint8_t phyId, WifiConstPsduMap psduMap,
                WifiTxVector txVector, double txPowerW) override;

private:
  void StartTraffic() override;

  PacketSocketAddress m_sockAddr;
  std::size_t m_nQosDataFrames;
  Ptr<ListErrorModel> m_errorModel;
  bool m_rtsCorrupted;
};

ReleaseSeqNoAfterCtsTimeoutTest::ReleaseSeqNoAfterCtsTimeoutTest()
    : MultiLinkOperationsTestBase(
          "Check sequence numbers after CTS timeout", 1,
          BaseParams{{"{36, 0, BAND_5GHZ, 0}", "{2, 0, BAND_2_4GHZ, 0}",
                      "{1, 0, BAND_6GHZ, 0}"},
                     {"{36, 0, BAND_5GHZ, 0}", "{2, 0, BAND_2_4GHZ, 0}",
                      "{1, 0, BAND_6GHZ, 0}"},
                     {}}),
      m_nQosDataFrames(0), m_errorModel(CreateObject<ListErrorModel>()),
      m_rtsCorrupted(false) {}

void ReleaseSeqNoAfterCtsTimeoutTest::DoSetup() {
  Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold",
                     StringValue("1000"));

  MultiLinkOperationsTestBase::DoSetup();

  for (const auto linkId : m_staMacs[0]->GetLinkIds()) {
    m_staMacs[0]->GetWifiPhy(linkId)->SetPostReceptionErrorModel(m_errorModel);
  }
}

void ReleaseSeqNoAfterCtsTimeoutTest::StartTraffic() {
  m_sockAddr.SetSingleDevice(m_apMac->GetDevice()->GetIfIndex());
  m_sockAddr.SetPhysicalAddress(m_staMacs[0]->GetAddress());
  m_sockAddr.SetProtocol(1);

  m_apMac->GetDevice()->GetNode()->AddApplication(
      GetApplication(m_sockAddr, 4, 1000));
}

void ReleaseSeqNoAfterCtsTimeoutTest::Transmit(Ptr<WifiMac> mac, uint8_t phyId,
                                               WifiConstPsduMap psduMap,
                                               WifiTxVector txVector,
                                               double txPowerW) {
  MultiLinkOperationsTestBase::Transmit(mac, phyId, psduMap, txVector,
                                        txPowerW);

  auto psdu = psduMap.begin()->second;

  if (psdu->GetHeader(0).IsRts() && !m_rtsCorrupted) {
    m_errorModel->SetList({psdu->GetPacket()->GetUid()});
    m_rtsCorrupted = true;
    m_apMac->GetDevice()->GetNode()->AddApplication(
        GetApplication(m_sockAddr, 4, 1000));
  } else if (psdu->GetHeader(0).IsQosData()) {
    m_nQosDataFrames++;

    if (m_nQosDataFrames == 2) {
      m_apMac->GetDevice()->GetNode()->AddApplication(
          GetApplication(m_sockAddr, 4, 1000));
    }
  }
}

void ReleaseSeqNoAfterCtsTimeoutTest::DoRun() {
  Simulator::Stop(m_duration);
  Simulator::Run();

  NS_TEST_EXPECT_MSG_EQ(m_nQosDataFrames, 3,
                        "Unexpected number of transmitted QoS data frames");

  std::size_t count{};

  for (const auto &txPsdu : m_txPsdus) {
    auto psdu = txPsdu.psduMap.begin()->second;

    if (!psdu->GetHeader(0).IsQosData()) {
      continue;
    }

    NS_TEST_EXPECT_MSG_EQ(psdu->GetNMpdus(), 4,
                          "Unexpected number of MPDUs in A-MPDU");

    count++;
    uint16_t expectedSeqNo{};

    switch (count) {
    case 1:
      expectedSeqNo = 4;
      break;
    case 2:
      expectedSeqNo = 0;
      break;
    case 3:
      expectedSeqNo = 8;
      break;
    }

    for (const auto &mpdu : *PeekPointer(psdu)) {
      NS_TEST_EXPECT_MSG_EQ(mpdu->GetHeader().GetSequenceNumber(),
                            expectedSeqNo++, "Unexpected sequence number");
    }
  }

  Simulator::Destroy();
}

class WifiMultiLinkOperationsTestSuite : public TestSuite {
public:
  WifiMultiLinkOperationsTestSuite();
};

WifiMultiLinkOperationsTestSuite::WifiMultiLinkOperationsTestSuite()
    : TestSuite("wifi-mlo", UNIT) {
  using ParamsTuple =
      std::tuple<MultiLinkOperationsTestBase::BaseParams, std::vector<uint8_t>,
                 uint8_t, std::string, std::string>;

  AddTestCase(new GetRnrLinkInfoTest(), TestCase::QUICK);
  AddTestCase(new MldSwapLinksTest(), TestCase::QUICK);

  for (const auto &[baseParams, setupLinks, apNegSupport, dlTidLinkMapping,
                    ulTidLinkMapping] :
       {ParamsTuple({{"{36, 0, BAND_5GHZ, 0}", "{2, 0, BAND_2_4GHZ, 0}",
                      "{1, 0, BAND_6GHZ, 0}"},
                     {"{36, 0, BAND_5GHZ, 0}", "{2, 0, BAND_2_4GHZ, 0}",
                      "{1, 0, BAND_6GHZ, 0}"},
                     {}},
                    {0, 1, 2}, 0, "0,1,2,3  0,1,2;  4,5  0,1",
                    "0,1,2,3  1,2;    6,7  0,1"),
        ParamsTuple({{"{108, 0, BAND_5GHZ, 0}", "{36, 0, BAND_5GHZ, 0}",
                      "{1, 0, BAND_6GHZ, 0}"},
                     {"{36, 0, BAND_5GHZ, 0}", "{120, 0, BAND_5GHZ, 0}",
                      "{5, 0, BAND_6GHZ, 0}"},
                     {}},
                    {0, 1, 2}, 1, "0,1,2,3  0,1,2;  4,5  0,1", ""),
        ParamsTuple({{"{2, 0, BAND_2_4GHZ, 0}", "{1, 0, BAND_6GHZ, 0}",
                      "{36, 0, BAND_5GHZ, 0}"},
                     {"{36, 0, BAND_5GHZ, 0}", "{9, 0, BAND_6GHZ, 0}",
                      "{120, 0, BAND_5GHZ, 0}"},
                     {}},
                    {0, 1, 2}, 3, "0,1,2,3  0;  4,5,6,7  1,2",
                    "0,2,3  1,2;  1,4,5,6,7  0"),
        ParamsTuple({{"{2, 0, BAND_2_4GHZ, 0}", "{36, 0, BAND_5GHZ, 0}",
                      "{8, 20, BAND_2_4GHZ, 0}"},
                     {"{36, 0, BAND_5GHZ, 0}", "{1, 0, BAND_6GHZ, 0}",
                      "{120, 0, BAND_5GHZ, 0}"},
                     {0}},
                    {0, 1}, 1, "0,1,2,3,4,5,6,7  0", "0,1,2,3,4,5,6,7  0"),
        ParamsTuple({{"{2, 0, BAND_2_4GHZ, 0}", "{36, 0, BAND_5GHZ, 0}",
                      "{8, 20, BAND_2_4GHZ, 0}"},
                     {"{36, 0, BAND_5GHZ, 0}", "{1, 0, BAND_6GHZ, 0}",
                      "{120, 0, BAND_5GHZ, 0}"},
                     {0, 1}},
                    {0, 1}, 3, "0,1,2,3  1", "0,1,2,3  1"),
        ParamsTuple({{"{2, 0, BAND_2_4GHZ, 0}", "{36, 0, BAND_5GHZ, 0}",
                      "{60, 0, BAND_5GHZ, 0}"},
                     {"{36, 0, BAND_5GHZ, 0}", "{1, 0, BAND_6GHZ, 0}",
                      "{120, 0, BAND_5GHZ, 0}"},
                     {0, 1, 2}},
                    {0, 2}, 3, "", ""),
        ParamsTuple({{"{2, 0, BAND_2_4GHZ, 0}", "{120, 0, BAND_5GHZ, 0}"},
                     {"{36, 0, BAND_5GHZ, 0}", "{1, 0, BAND_6GHZ, 0}",
                      "{120, 0, BAND_5GHZ, 0}"},
                     {0, 1}},
                    {2}, 3, "", ""),
        ParamsTuple({{"{2, 0, BAND_2_4GHZ, 0}", "{36, 0, BAND_5GHZ, 0}"},
                     {"{36, 0, BAND_5GHZ, 0}", "{1, 0, BAND_6GHZ, 0}",
                      "{120, 0, BAND_5GHZ, 0}"},
                     {}},
                    {1, 0}, 3, "0,1,2,3  1", ""),
        ParamsTuple({{"{120, 0, BAND_5GHZ, 0}"},
                     {"{36, 0, BAND_5GHZ, 0}", "{1, 0, BAND_6GHZ, 0}",
                      "{120, 0, BAND_5GHZ, 0}"},
                     {}},
                    {2}, 3, "", ""),
        ParamsTuple({{"{36, 0, BAND_5GHZ, 0}", "{1, 0, BAND_6GHZ, 0}",
                      "{120, 0, BAND_5GHZ, 0}"},
                     {"{120, 0, BAND_5GHZ, 0}"},
                     {}},
                    {2}, 0, "0,1,2,3  0,1;  4,5,6,7  0,1", "")}) {
    AddTestCase(new MultiLinkSetupTest(baseParams, WifiScanType::PASSIVE,
                                       setupLinks, apNegSupport,
                                       dlTidLinkMapping, ulTidLinkMapping),
                TestCase::QUICK);
    AddTestCase(new MultiLinkSetupTest(baseParams, WifiScanType::ACTIVE,
                                       setupLinks, apNegSupport,
                                       dlTidLinkMapping, ulTidLinkMapping),
                TestCase::QUICK);

    for (const auto &trafficPattern :
         {WifiTrafficPattern::STA_TO_STA, WifiTrafficPattern::STA_TO_AP,
          WifiTrafficPattern::AP_TO_STA, WifiTrafficPattern::AP_TO_BCAST,
          WifiTrafficPattern::STA_TO_BCAST}) {
      AddTestCase(new MultiLinkTxTest(baseParams, trafficPattern,
                                      WifiBaEnabled::NO,
                                      WifiUseBarAfterMissedBa::NO, 1),
                  TestCase::QUICK);
      for (const auto &useBarAfterMissedBa :
           {WifiUseBarAfterMissedBa::YES, WifiUseBarAfterMissedBa::NO}) {
        AddTestCase(new MultiLinkTxTest(baseParams, trafficPattern,
                                        WifiBaEnabled::YES, useBarAfterMissedBa,
                                        1),
                    TestCase::QUICK);
        AddTestCase(new MultiLinkTxTest(baseParams, trafficPattern,
                                        WifiBaEnabled::YES, useBarAfterMissedBa,
                                        2),
                    TestCase::QUICK);
      }
    }

    for (const auto &muTrafficPattern :
         {WifiMuTrafficPattern::DL_MU_BAR_BA_SEQUENCE,
          WifiMuTrafficPattern::DL_MU_MU_BAR,
          WifiMuTrafficPattern::DL_MU_AGGR_MU_BAR,
          WifiMuTrafficPattern::UL_MU}) {
      for (const auto &useBarAfterMissedBa :
           {WifiUseBarAfterMissedBa::YES, WifiUseBarAfterMissedBa::NO}) {
        AddTestCase(new MultiLinkMuTxTest(baseParams, muTrafficPattern,
                                          useBarAfterMissedBa, 1),
                    TestCase::QUICK);
        AddTestCase(new MultiLinkMuTxTest(baseParams, muTrafficPattern,
                                          useBarAfterMissedBa, 2),
                    TestCase::QUICK);
      }
    }
  }

  AddTestCase(new ReleaseSeqNoAfterCtsTimeoutTest(), TestCase::QUICK);
}

static WifiMultiLinkOperationsTestSuite g_wifiMultiLinkOperationsTestSuite;
