
#include "ns3/address-utils.h"
#include "ns3/header-serialization-test.h"
#include "ns3/log.h"
#include "ns3/mgt-headers.h"
#include "ns3/multi-link-element.h"
#include "ns3/reduced-neighbor-report.h"
#include "ns3/simulator.h"
#include "ns3/tid-to-link-mapping-element.h"
#include "ns3/wifi-phy-operating-channel.h"
#include "ns3/wifi-utils.h"

#include <optional>
#include <set>
#include <sstream>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WifiEhtInfoElemsTest");

class BasicMultiLinkElementTest : public HeaderSerializationTestCase {
public:
  BasicMultiLinkElementTest();
  ~BasicMultiLinkElementTest() override;

  MultiLinkElement GetMultiLinkElement(
      const CommonInfoBasicMle &commonInfo,
      std::vector<MultiLinkElement::PerStaProfileSubelement> subelements);

private:
  void DoRun() override;

  MgtAssocRequestHeader m_outerAssoc;
};

BasicMultiLinkElementTest::BasicMultiLinkElementTest()
    : HeaderSerializationTestCase("Check serialization and deserialization of "
                                  "Basic variant Multi-Link elements") {}

BasicMultiLinkElementTest::~BasicMultiLinkElementTest() {}

MultiLinkElement BasicMultiLinkElementTest::GetMultiLinkElement(
    const CommonInfoBasicMle &commonInfo,
    std::vector<MultiLinkElement::PerStaProfileSubelement> subelements) {
  MultiLinkElement mle(MultiLinkElement::BASIC_VARIANT);
  mle.SetMldMacAddress(commonInfo.m_mldMacAddress);
  if (commonInfo.m_linkIdInfo.has_value()) {
    mle.SetLinkIdInfo(*commonInfo.m_linkIdInfo);
  }
  if (commonInfo.m_bssParamsChangeCount.has_value()) {
    mle.SetBssParamsChangeCount(*commonInfo.m_bssParamsChangeCount);
  }
  if (commonInfo.m_mediumSyncDelayInfo.has_value()) {
    mle.SetMediumSyncDelayTimer(MicroSeconds(
        32 * commonInfo.m_mediumSyncDelayInfo->mediumSyncDuration));
    mle.SetMediumSyncOfdmEdThreshold(
        commonInfo.m_mediumSyncDelayInfo->mediumSyncOfdmEdThreshold - 72);
    mle.SetMediumSyncMaxNTxops(
        commonInfo.m_mediumSyncDelayInfo->mediumSyncMaxNTxops + 1);
  }
  if (commonInfo.m_emlCapabilities.has_value()) {
    auto padding = commonInfo.m_emlCapabilities->emlsrPaddingDelay;
    mle.SetEmlsrPaddingDelay(
        MicroSeconds(padding == 0 ? 0 : (1 << (4 + padding))));
    auto transitionD = commonInfo.m_emlCapabilities->emlsrTransitionDelay;
    mle.SetEmlsrTransitionDelay(
        MicroSeconds(transitionD == 0 ? 0 : (1 << (3 + transitionD))));
    auto transitionT = commonInfo.m_emlCapabilities->transitionTimeout;
    mle.SetTransitionTimeout(
        MicroSeconds(transitionT == 0 ? 0 : (1 << (6 + transitionT))));
  }

  for (std::size_t i = 0; i < subelements.size(); ++i) {
    mle.AddPerStaProfileSubelement();
    mle.GetPerStaProfile(i) = std::move(subelements[i]);
  }

  return mle;
}

void BasicMultiLinkElementTest::DoRun() {
  CommonInfoBasicMle commonInfo = {
      .m_mldMacAddress = Mac48Address("01:23:45:67:89:ab"),
  };

  TestHeaderSerialization(GetMultiLinkElement(commonInfo, {}));

  commonInfo.m_linkIdInfo = 3;

  TestHeaderSerialization(GetMultiLinkElement(commonInfo, {}));

  commonInfo.m_bssParamsChangeCount = 1;

  TestHeaderSerialization(GetMultiLinkElement(commonInfo, {}));

  commonInfo.m_mediumSyncDelayInfo =
      CommonInfoBasicMle::MediumSyncDelayInfo{.mediumSyncDuration = 1,
                                              .mediumSyncOfdmEdThreshold = 4,
                                              .mediumSyncMaxNTxops = 5};

  TestHeaderSerialization(GetMultiLinkElement(commonInfo, {}));

  commonInfo.m_emlCapabilities =
      CommonInfoBasicMle::EmlCapabilities{.emlsrSupport = 1,
                                          .emlsrPaddingDelay = 4,
                                          .emlsrTransitionDelay = 5,
                                          .transitionTimeout = 10};

  TestHeaderSerialization(GetMultiLinkElement(commonInfo, {}));

  CapabilityInformation capabilities;
  capabilities.SetShortPreamble(true);
  capabilities.SetShortSlotTime(true);
  capabilities.SetEss();

  m_outerAssoc.SetListenInterval(0);
  m_outerAssoc.Capabilities() = capabilities;
  m_outerAssoc.Get<Ssid>() = Ssid("MySsid");

  AllSupportedRates rates;
  rates.AddSupportedRate(6e6);
  rates.AddSupportedRate(9e6);
  rates.AddSupportedRate(12e6);
  rates.AddSupportedRate(18e6);
  rates.AddSupportedRate(24e6);
  rates.AddSupportedRate(36e6);
  rates.AddSupportedRate(48e6);
  rates.AddSupportedRate(54e6);
  rates.AddSupportedRate(1e6);
  rates.AddSupportedRate(2e6);

  m_outerAssoc.Get<SupportedRates>() = rates.rates;
  m_outerAssoc.Get<ExtendedSupportedRatesIE>() = rates.extendedRates;

  EhtCapabilities ehtCapabilities;
  for (auto maxMcs : {7, 9, 11, 13}) {
    ehtCapabilities.SetSupportedRxEhtMcsAndNss(
        EhtMcsAndNssSet::EHT_MCS_MAP_TYPE_20_MHZ_ONLY, maxMcs, 1);
    ehtCapabilities.SetSupportedTxEhtMcsAndNss(
        EhtMcsAndNssSet::EHT_MCS_MAP_TYPE_20_MHZ_ONLY, maxMcs, 1);
  }

  m_outerAssoc.Get<HeCapabilities>().emplace();
  m_outerAssoc.Get<EhtCapabilities>() = ehtCapabilities;

  MultiLinkElement::PerStaProfileSubelement perStaProfile1(
      MultiLinkElement::BASIC_VARIANT);
  perStaProfile1.SetLinkId(3);
  perStaProfile1.SetCompleteProfile();
  perStaProfile1.SetAssocRequest(m_outerAssoc);

  MgtAssocRequestHeader assoc;
  assoc.Capabilities() = capabilities;
  assoc.Get<Ssid>() = Ssid("OtherSsid");
  assoc.Get<TidToLinkMapping>().emplace_back();
  rates.AddSupportedRate(5.5e6);
  rates.AddSupportedRate(11e6);
  assoc.Get<SupportedRates>() = rates.rates;
  assoc.Get<ExtendedSupportedRatesIE>() = rates.extendedRates;
  assoc.Get<VhtCapabilities>().emplace();
  assoc.Get<HeCapabilities>().emplace();

  MultiLinkElement::PerStaProfileSubelement perStaProfile2(
      MultiLinkElement::BASIC_VARIANT);
  perStaProfile2.SetLinkId(0);
  perStaProfile2.SetCompleteProfile();
  perStaProfile2.SetStaMacAddress(Mac48Address("ba:98:76:54:32:10"));
  perStaProfile2.SetAssocRequest(assoc);

  assoc.Get<Ssid>().reset();
  assoc.Get<EhtCapabilities>() = ehtCapabilities;

  auto perStaProfile3 = perStaProfile2;
  perStaProfile3.SetAssocRequest(assoc);

  m_outerAssoc.Get<MultiLinkElement>() = GetMultiLinkElement(
      commonInfo, {perStaProfile1, perStaProfile2, perStaProfile3});

  TestHeaderSerialization(m_outerAssoc);

  Buffer buffer;
  buffer.AddAtStart(m_outerAssoc.GetSerializedSize());
  m_outerAssoc.Serialize(buffer.Begin());

  auto i = buffer.Begin();
  i = CapabilityInformation().Deserialize(i);
  i.ReadLsbtohU16();

  auto tmp = i;
  i = Ssid().DeserializeIfPresent(tmp);
  NS_TEST_EXPECT_MSG_GT(i.GetDistanceFrom(tmp), 0, "Ssid element not present");

  i = SupportedRates().DeserializeIfPresent(tmp = i);
  NS_TEST_EXPECT_MSG_GT(i.GetDistanceFrom(tmp), 0,
                        "SupportedRates element not present");

  i = ExtendedSupportedRatesIE().DeserializeIfPresent(tmp = i);
  NS_TEST_EXPECT_MSG_GT(i.GetDistanceFrom(tmp), 0,
                        "ExtendedSupportedRatesIE element not present");

  i = HeCapabilities().DeserializeIfPresent(tmp = i);
  NS_TEST_EXPECT_MSG_GT(i.GetDistanceFrom(tmp), 0,
                        "HeCapabilities element not present");

  NS_TEST_EXPECT_MSG_EQ(i.ReadU8(), IE_EXTENSION,
                        "IE_EXTENSION expected at the begin of MLE");
  i.ReadU8();
  NS_TEST_EXPECT_MSG_EQ(i.ReadU8(), IE_EXT_MULTI_LINK_ELEMENT,
                        "IE_EXT_MULTI_LINK_ELEMENT expected");

  uint16_t mlControl = i.ReadLsbtohU16();
  auto nBytes = CommonInfoBasicMle().Deserialize(i, mlControl >> 4);
  i.Next(nBytes);

  NS_TEST_EXPECT_MSG_EQ(i.ReadU8(),
                        MultiLinkElement::PER_STA_PROFILE_SUBELEMENT_ID,
                        "PER_STA_PROFILE_SUBELEMENT_ID expected");
  i.ReadU8();
  i.ReadLsbtohU16();
  i.ReadU8();
  i = CapabilityInformation().Deserialize(i);

  NS_TEST_EXPECT_MSG_EQ(i.ReadU8(),
                        MultiLinkElement::PER_STA_PROFILE_SUBELEMENT_ID,
                        "PER_STA_PROFILE_SUBELEMENT_ID expected");
  i.ReadU8();
  i.ReadLsbtohU16();
  i.ReadU8();
  Mac48Address address;
  ReadFrom(i, address);
  i = CapabilityInformation().Deserialize(i);

  i = ExtendedSupportedRatesIE().DeserializeIfPresent(tmp = i);
  NS_TEST_EXPECT_MSG_GT(i.GetDistanceFrom(tmp), 0,
                        "ExtendedSupportedRatesIE element not present");

  i = VhtCapabilities().DeserializeIfPresent(tmp = i);
  NS_TEST_EXPECT_MSG_GT(i.GetDistanceFrom(tmp), 0,
                        "VhtCapabilities element not present");

  NonInheritance nonInheritance;
  i = nonInheritance.DeserializeIfPresent(tmp = i);
  NS_TEST_EXPECT_MSG_GT(i.GetDistanceFrom(tmp), 0,
                        "Non-Inheritance element not present");
  NS_TEST_EXPECT_MSG_EQ(
      nonInheritance.IsPresent(IE_EXTENSION, IE_EXT_EHT_CAPABILITIES), true,
      "Non-Inheritance does not indicate EhtCapabilities");
  NS_TEST_EXPECT_MSG_EQ(
      nonInheritance.m_elemIdList.size(), 0,
      "Unexpected size for Elem ID list of Non-Inheritance element");
  NS_TEST_EXPECT_MSG_EQ(
      nonInheritance.m_elemIdExtList.size(), 1,
      "Unexpected size for Elem ID list of Non-Inheritance element");

  NS_TEST_EXPECT_MSG_EQ(i.ReadU8(),
                        MultiLinkElement::PER_STA_PROFILE_SUBELEMENT_ID,
                        "PER_STA_PROFILE_SUBELEMENT_ID expected");
  i.ReadU8();
  i.ReadLsbtohU16();
  i.ReadU8();
  ReadFrom(i, address);
  i = CapabilityInformation().Deserialize(i);

  i = ExtendedSupportedRatesIE().DeserializeIfPresent(tmp = i);
  NS_TEST_EXPECT_MSG_GT(i.GetDistanceFrom(tmp), 0,
                        "ExtendedSupportedRatesIE element not present");

  i = VhtCapabilities().DeserializeIfPresent(tmp = i);
  NS_TEST_EXPECT_MSG_GT(i.GetDistanceFrom(tmp), 0,
                        "VhtCapabilities element not present");

  ehtCapabilities =
      EhtCapabilities(true, m_outerAssoc.Get<HeCapabilities>().value());
  i = ehtCapabilities.DeserializeIfPresent(tmp = i);
  NS_TEST_EXPECT_MSG_GT(i.GetDistanceFrom(tmp), 0,
                        "EhtCapabilities element not present");

  MgtAssocRequestHeader frame;
  auto count = frame.Deserialize(buffer.Begin());

  NS_TEST_EXPECT_MSG_EQ(count, buffer.GetSize(),
                        "Unexpected number of deserialized bytes");

  NS_TEST_EXPECT_MSG_EQ(frame.Get<Ssid>().has_value(), true,
                        "Containing frame should have SSID IE");
  NS_TEST_EXPECT_MSG_EQ(frame.Get<SupportedRates>().has_value(), true,
                        "Containing frame should have Supported Rates IE");
  NS_TEST_EXPECT_MSG_EQ(
      frame.Get<ExtendedSupportedRatesIE>().has_value(), true,
      "Containing frame should have Extended Supported Rates IE");
  NS_TEST_EXPECT_MSG_EQ(frame.Get<HtCapabilities>().has_value(), false,
                        "Containing frame should not have HT Capabilities IE");
  NS_TEST_EXPECT_MSG_EQ(
      frame.Get<ExtendedCapabilities>().has_value(), false,
      "Containing frame should not have Extended Capabilities IE");
  NS_TEST_EXPECT_MSG_EQ(frame.Get<VhtCapabilities>().has_value(), false,
                        "Containing frame should not have VHT Capabilities IE");
  NS_TEST_EXPECT_MSG_EQ(frame.Get<HeCapabilities>().has_value(), true,
                        "Containing frame should have HE Capabilities IE");
  NS_TEST_EXPECT_MSG_EQ(frame.Get<MultiLinkElement>().has_value(), true,
                        "Containing frame should have Multi-Link Element IE");
  NS_TEST_EXPECT_MSG_EQ(frame.Get<EhtCapabilities>().has_value(), true,
                        "Containing frame should have EHT Capabilities IE");
  NS_TEST_EXPECT_MSG_EQ(
      frame.Get<TidToLinkMapping>().empty(), true,
      "Containing frame should not have TID-to-Link Mapping IE");

  auto &mle = frame.Get<MultiLinkElement>().value();

  NS_TEST_EXPECT_MSG_EQ(mle.GetNPerStaProfileSubelements(), 3,
                        "Unexpected number of Per-STA Profile subelements");

  auto &perSta1 = mle.GetPerStaProfile(0);
  NS_TEST_EXPECT_MSG_EQ(
      perSta1.HasAssocRequest(), true,
      "First Per-STA Profile should contain an Association Request frame");
  auto &perSta1Frame = std::get<std::reference_wrapper<MgtAssocRequestHeader>>(
                           perSta1.GetAssocRequest())
                           .get();

  NS_TEST_EXPECT_MSG_EQ(
      perSta1Frame.Get<Ssid>().has_value(), false,
      "Frame in first Per-STA Profile should not have SSID IE");
  NS_TEST_EXPECT_MSG_EQ(
      perSta1Frame.Get<SupportedRates>().has_value(), true,
      "Frame in first Per-STA Profile should have Supported Rates IE");
  NS_TEST_EXPECT_MSG_EQ(
      (perSta1Frame.Get<SupportedRates>() == frame.Get<SupportedRates>()), true,
      "Supported Rates IE not correctly inherited by frame in first Per-STA "
      "Profile");
  NS_TEST_EXPECT_MSG_EQ(
      perSta1Frame.Get<ExtendedSupportedRatesIE>().has_value(), true,
      "Frame in first Per-STA Profile should have Extended Supported Rates IE");
  NS_TEST_EXPECT_MSG_EQ((perSta1Frame.Get<ExtendedSupportedRatesIE>() ==
                         frame.Get<ExtendedSupportedRatesIE>()),
                        true,
                        "Extended Supported Rates IE not correctly inherited "
                        "by frame in first Per-STA Profile");
  NS_TEST_EXPECT_MSG_EQ(
      perSta1Frame.Get<HtCapabilities>().has_value(), false,
      "Frame in first Per-STA Profile should not have HT Capabilities IE");
  NS_TEST_EXPECT_MSG_EQ(perSta1Frame.Get<ExtendedCapabilities>().has_value(),
                        false,
                        "Frame in first Per-STA Profile should not have "
                        "Extended Capabilities IE");
  NS_TEST_EXPECT_MSG_EQ(
      perSta1Frame.Get<VhtCapabilities>().has_value(), false,
      "Frame in first Per-STA Profile should not have VHT Capabilities IE");
  NS_TEST_EXPECT_MSG_EQ(
      perSta1Frame.Get<HeCapabilities>().has_value(), true,
      "Frame in first Per-STA Profile should have HE Capabilities IE");
  NS_TEST_EXPECT_MSG_EQ(
      (perSta1Frame.Get<HeCapabilities>() == frame.Get<HeCapabilities>()), true,
      "HE Capabilities IE not correctly inherited by frame in first Per-STA "
      "Profile");
  NS_TEST_EXPECT_MSG_EQ(
      perSta1Frame.Get<MultiLinkElement>().has_value(), false,
      "Frame in first Per-STA Profile should not have Multi-Link Element IE");
  NS_TEST_EXPECT_MSG_EQ(
      perSta1Frame.Get<EhtCapabilities>().has_value(), true,
      "Frame in first Per-STA Profile should have EHT Capabilities IE");
  NS_TEST_EXPECT_MSG_EQ(
      (perSta1Frame.Get<EhtCapabilities>() == frame.Get<EhtCapabilities>()),
      true,
      "EHT Capabilities IE not correctly inherited by frame in first Per-STA "
      "Profile");
  NS_TEST_EXPECT_MSG_EQ(
      perSta1Frame.Get<TidToLinkMapping>().empty(), true,
      "Frame in first Per-STA Profile should not have TID-to-Link Mapping IE");

  auto &perSta2 = mle.GetPerStaProfile(1);
  NS_TEST_EXPECT_MSG_EQ(
      perSta2.HasAssocRequest(), true,
      "Second Per-STA Profile should contain an Association Request frame");
  auto &perSta2Frame = std::get<std::reference_wrapper<MgtAssocRequestHeader>>(
                           perSta2.GetAssocRequest())
                           .get();

  NS_TEST_EXPECT_MSG_EQ(
      perSta2Frame.Get<Ssid>().has_value(), false,
      "Frame in second Per-STA Profile should not have SSID IE");
  NS_TEST_EXPECT_MSG_EQ(
      perSta2Frame.Get<SupportedRates>().has_value(), true,
      "Frame in second Per-STA Profile should have Supported Rates IE");
  NS_TEST_EXPECT_MSG_EQ(
      (perSta2Frame.Get<SupportedRates>() == frame.Get<SupportedRates>()), true,
      "Supported Rates IE not correctly inherited by frame in second Per-STA "
      "Profile");
  NS_TEST_EXPECT_MSG_EQ(
      perSta2Frame.Get<ExtendedSupportedRatesIE>().has_value(), true,
      "Frame in second Per-STA Profile should have Extended Supported Rates "
      "IE");
  NS_TEST_EXPECT_MSG_EQ((perSta2Frame.Get<ExtendedSupportedRatesIE>() ==
                         frame.Get<ExtendedSupportedRatesIE>()),
                        false,
                        "Extended Supported Rates IE should have not been "
                        "inherited by frame in second Per-STA "
                        "Profile");
  NS_TEST_EXPECT_MSG_EQ(
      perSta2Frame.Get<HtCapabilities>().has_value(), false,
      "Frame in second Per-STA Profile should not have HT Capabilities IE");
  NS_TEST_EXPECT_MSG_EQ(perSta2Frame.Get<ExtendedCapabilities>().has_value(),
                        false,
                        "Frame in second Per-STA Profile should not have "
                        "Extended Capabilities IE");
  NS_TEST_EXPECT_MSG_EQ(
      perSta2Frame.Get<VhtCapabilities>().has_value(), true,
      "Frame in second Per-STA Profile should have VHT Capabilities IE");
  NS_TEST_EXPECT_MSG_EQ(
      perSta2Frame.Get<HeCapabilities>().has_value(), true,
      "Frame in second Per-STA Profile should have HE Capabilities IE");
  NS_TEST_EXPECT_MSG_EQ(
      (perSta2Frame.Get<HeCapabilities>() == frame.Get<HeCapabilities>()), true,
      "HE Capabilities IE not correctly inherited by frame in second Per-STA "
      "Profile");
  NS_TEST_EXPECT_MSG_EQ(
      perSta2Frame.Get<MultiLinkElement>().has_value(), false,
      "Frame in second Per-STA Profile should not have Multi-Link Element IE");
  NS_TEST_EXPECT_MSG_EQ(perSta2Frame.Get<EhtCapabilities>().has_value(), false,
                        "Frame in second Per-STA Profile should have not "
                        "inherited EHT Capabilities IE");
  NS_TEST_EXPECT_MSG_EQ(
      perSta2Frame.Get<TidToLinkMapping>().empty(), true,
      "Frame in second Per-STA Profile should not have TID-to-Link Mapping IE");

  auto &perSta3 = mle.GetPerStaProfile(2);
  NS_TEST_EXPECT_MSG_EQ(
      perSta3.HasAssocRequest(), true,
      "Third Per-STA Profile should contain an Association Request frame");
  auto &perSta3Frame = std::get<std::reference_wrapper<MgtAssocRequestHeader>>(
                           perSta3.GetAssocRequest())
                           .get();

  NS_TEST_EXPECT_MSG_EQ(
      perSta3Frame.Get<Ssid>().has_value(), false,
      "Frame in third Per-STA Profile should not have SSID IE");
  NS_TEST_EXPECT_MSG_EQ(
      perSta3Frame.Get<SupportedRates>().has_value(), true,
      "Frame in third Per-STA Profile should have Supported Rates IE");
  NS_TEST_EXPECT_MSG_EQ(
      (perSta3Frame.Get<SupportedRates>() == frame.Get<SupportedRates>()), true,
      "Supported Rates IE not correctly inherited by frame in third Per-STA "
      "Profile");
  NS_TEST_EXPECT_MSG_EQ(
      perSta3Frame.Get<ExtendedSupportedRatesIE>().has_value(), true,
      "Frame in third Per-STA Profile should have Extended Supported Rates IE");
  NS_TEST_EXPECT_MSG_EQ((perSta3Frame.Get<ExtendedSupportedRatesIE>() ==
                         frame.Get<ExtendedSupportedRatesIE>()),
                        false,
                        "Extended Supported Rates IE should have not been "
                        "inherited by frame in third Per-STA "
                        "Profile");
  NS_TEST_EXPECT_MSG_EQ(
      perSta3Frame.Get<HtCapabilities>().has_value(), false,
      "Frame in third Per-STA Profile should not have HT Capabilities IE");
  NS_TEST_EXPECT_MSG_EQ(perSta3Frame.Get<ExtendedCapabilities>().has_value(),
                        false,
                        "Frame in third Per-STA Profile should not have "
                        "Extended Capabilities IE");
  NS_TEST_EXPECT_MSG_EQ(
      perSta3Frame.Get<VhtCapabilities>().has_value(), true,
      "Frame in third Per-STA Profile should have VHT Capabilities IE");
  NS_TEST_EXPECT_MSG_EQ(
      perSta3Frame.Get<HeCapabilities>().has_value(), true,
      "Frame in third Per-STA Profile should have HE Capabilities IE");
  NS_TEST_EXPECT_MSG_EQ(
      (perSta3Frame.Get<HeCapabilities>() == frame.Get<HeCapabilities>()), true,
      "HE Capabilities IE not correctly inherited by frame in third Per-STA "
      "Profile");
  NS_TEST_EXPECT_MSG_EQ(
      perSta3Frame.Get<MultiLinkElement>().has_value(), false,
      "Frame in third Per-STA Profile should not have Multi-Link Element IE");
  NS_TEST_EXPECT_MSG_EQ(perSta3Frame.Get<EhtCapabilities>().has_value(), true,
                        "Frame in third Per-STA Profile should have inherited "
                        "EHT Capabilities IE");
  NS_TEST_EXPECT_MSG_EQ(
      (perSta3Frame.Get<EhtCapabilities>() == frame.Get<EhtCapabilities>()),
      true,
      "EHT Capabilities IE not correctly inherited by frame in third Per-STA "
      "Profile");
  NS_TEST_EXPECT_MSG_EQ(
      perSta3Frame.Get<TidToLinkMapping>().empty(), true,
      "Frame in third Per-STA Profile should not have TID-to-Link Mapping IE");
}

class ReducedNeighborReportTest : public HeaderSerializationTestCase {
public:
  ReducedNeighborReportTest();
  ~ReducedNeighborReportTest() override;

  using PhyOpChannelIt = WifiPhyOperatingChannel::ConstIterator;

  ReducedNeighborReport GetReducedNeighborReport(PhyOpChannelIt channel2_4It,
                                                 PhyOpChannelIt channel5It,
                                                 PhyOpChannelIt channel6It);

private:
  void DoRun() override;
};

ReducedNeighborReportTest::ReducedNeighborReportTest()
    : HeaderSerializationTestCase("Check serialization and deserialization of "
                                  "Reduced Neighbor Report elements") {}

ReducedNeighborReportTest::~ReducedNeighborReportTest() {}

ReducedNeighborReport
ReducedNeighborReportTest::GetReducedNeighborReport(PhyOpChannelIt channel2_4It,
                                                    PhyOpChannelIt channel5It,
                                                    PhyOpChannelIt channel6It) {
  ReducedNeighborReport rnr;

  std::stringstream info;

  if (channel2_4It != WifiPhyOperatingChannel::m_frequencyChannels.cend()) {
    WifiPhyOperatingChannel channel(channel2_4It);

    info << "{Ch=" << +channel.GetNumber() << ", Bw=" << channel.GetWidth()
         << ", 2.4 GHz} ";
    rnr.AddNbrApInfoField();
    std::size_t nbrId = rnr.GetNNbrApInfoFields() - 1;
    rnr.SetOperatingChannel(nbrId, channel);
    rnr.AddTbttInformationField(nbrId);
    rnr.SetBssid(nbrId, 0, Mac48Address("00:00:00:00:00:24"));
    rnr.SetShortSsid(nbrId, 0, 0);
    rnr.SetBssParameters(nbrId, 0, 10);
    rnr.SetPsd20MHz(nbrId, 0, 50);
    rnr.SetMldParameters(nbrId, 0, 0, 2, 3);
  }

  if (channel5It != WifiPhyOperatingChannel::m_frequencyChannels.cend()) {
    WifiPhyOperatingChannel channel(channel5It);

    info << "{Ch=" << +channel.GetNumber() << ", Bw=" << channel.GetWidth()
         << ", 5 GHz} ";
    rnr.AddNbrApInfoField();
    std::size_t nbrId = rnr.GetNNbrApInfoFields() - 1;
    rnr.SetOperatingChannel(nbrId, channel);
    rnr.AddTbttInformationField(nbrId);
    rnr.SetBssid(nbrId, 0, Mac48Address("00:00:00:00:00:05"));
    rnr.SetShortSsid(nbrId, 0, 0);
    rnr.SetBssParameters(nbrId, 0, 20);
    rnr.SetPsd20MHz(nbrId, 0, 60);
    rnr.SetMldParameters(nbrId, 0, 0, 3, 4);
    rnr.AddTbttInformationField(nbrId);
    rnr.SetBssid(nbrId, 1, Mac48Address("00:00:00:00:01:05"));
    rnr.SetShortSsid(nbrId, 1, 0);
    rnr.SetBssParameters(nbrId, 1, 30);
    rnr.SetPsd20MHz(nbrId, 1, 70);
    rnr.SetMldParameters(nbrId, 1, 0, 4, 5);
  }

  if (channel6It != WifiPhyOperatingChannel::m_frequencyChannels.cend()) {
    WifiPhyOperatingChannel channel(channel6It);

    info << "{Ch=" << +channel.GetNumber() << ", Bw=" << channel.GetWidth()
         << ", 6 GHz} ";
    rnr.AddNbrApInfoField();
    std::size_t nbrId = rnr.GetNNbrApInfoFields() - 1;
    rnr.SetOperatingChannel(nbrId, channel);
    rnr.AddTbttInformationField(nbrId);
    rnr.SetBssid(nbrId, 0, Mac48Address("00:00:00:00:00:06"));
    rnr.SetShortSsid(nbrId, 0, 0);
    rnr.SetBssParameters(nbrId, 0, 40);
    rnr.SetPsd20MHz(nbrId, 0, 80);
    rnr.SetMldParameters(nbrId, 0, 0, 5, 6);
  }

  NS_LOG_DEBUG(info.str());
  return rnr;
}

void ReducedNeighborReportTest::DoRun() {
  PhyOpChannelIt channel2_4It;
  PhyOpChannelIt channel5It;
  PhyOpChannelIt channel6It;
  channel2_4It = channel5It = channel6It =
      WifiPhyOperatingChannel::m_frequencyChannels.cbegin();

  while (channel2_4It != WifiPhyOperatingChannel::m_frequencyChannels.cend() ||
         channel5It != WifiPhyOperatingChannel::m_frequencyChannels.cend() ||
         channel6It != WifiPhyOperatingChannel::m_frequencyChannels.cend()) {
    if (channel2_4It != WifiPhyOperatingChannel::m_frequencyChannels.cend()) {
      channel2_4It = WifiPhyOperatingChannel::FindFirst(
          0, 0, 0, WIFI_STANDARD_80211be, WIFI_PHY_BAND_2_4GHZ, channel2_4It);
    }
    if (channel5It != WifiPhyOperatingChannel::m_frequencyChannels.cend()) {
      channel5It = WifiPhyOperatingChannel::FindFirst(
          0, 0, 0, WIFI_STANDARD_80211be, WIFI_PHY_BAND_5GHZ, channel5It);
    }
    if (channel6It != WifiPhyOperatingChannel::m_frequencyChannels.cend()) {
      channel6It = WifiPhyOperatingChannel::FindFirst(
          0, 0, 0, WIFI_STANDARD_80211be, WIFI_PHY_BAND_6GHZ, channel6It);
    }

    TestHeaderSerialization(
        GetReducedNeighborReport(channel2_4It, channel5It, channel6It));

    if (channel2_4It != WifiPhyOperatingChannel::m_frequencyChannels.cend()) {
      channel2_4It++;
    }
    if (channel5It != WifiPhyOperatingChannel::m_frequencyChannels.cend()) {
      channel5It++;
    }
    if (channel6It != WifiPhyOperatingChannel::m_frequencyChannels.cend()) {
      channel6It++;
    }
  }
}

class WifiEhtCapabilitiesIeTest : public HeaderSerializationTestCase {
public:
  WifiEhtCapabilitiesIeTest(bool is2_4Ghz, uint16_t channelWidth);
  ~WifiEhtCapabilitiesIeTest() override = default;

  HeCapabilities GetHeCapabilities() const;

  EhtCapabilities GetEhtCapabilities(uint16_t maxMpduLength,
                                     uint32_t maxAmpduSize,
                                     uint8_t maxSupportedMcs) const;

  Buffer SerializeIntoBuffer(const EhtCapabilities &ehtCapabilities);

  void CheckSerializedByte(const Buffer &buffer, uint32_t position,
                           uint8_t value);

  void CheckEhtMacCapabilitiesInformation(const Buffer &buffer,
                                          uint8_t expectedValueFirstByte);

  void CheckEhtPhyCapabilitiesInformation(const Buffer &buffer,
                                          uint8_t expectedValueSixthByte);

  void CheckSupportedEhtMcsAndNssSet(const Buffer &buffer,
                                     uint8_t maxSupportedMcs);

private:
  void DoRun() override;

  bool m_is2_4Ghz;
  uint16_t m_channelWidth;
};

WifiEhtCapabilitiesIeTest ::WifiEhtCapabilitiesIeTest(bool is2_4Ghz,
                                                      uint16_t channelWidth)
    : HeaderSerializationTestCase{"Check serialization and deserialization of "
                                  "EHT capabilities IE"},
      m_is2_4Ghz{is2_4Ghz}, m_channelWidth{channelWidth} {}

HeCapabilities WifiEhtCapabilitiesIeTest::GetHeCapabilities() const {
  HeCapabilities capabilities;
  uint8_t channelWidthSet = 0;
  if ((m_channelWidth >= 40) && m_is2_4Ghz) {
    channelWidthSet |= 0x01;
  }
  if ((m_channelWidth >= 80) && !m_is2_4Ghz) {
    channelWidthSet |= 0x02;
  }
  if ((m_channelWidth >= 160) && !m_is2_4Ghz) {
    channelWidthSet |= 0x04;
  }
  capabilities.SetChannelWidthSet(channelWidthSet);
  return capabilities;
}

EhtCapabilities
WifiEhtCapabilitiesIeTest::GetEhtCapabilities(uint16_t maxMpduLength,
                                              uint32_t maxAmpduSize,
                                              uint8_t maxSupportedMcs) const {
  EhtCapabilities capabilities;

  if (m_is2_4Ghz) {
    capabilities.SetMaxMpduLength(maxMpduLength);
  }
  maxAmpduSize =
      (1UL << static_cast<uint32_t>(std::ceil(std::log2(maxAmpduSize + 1)))) -
      1;
  capabilities.SetMaxAmpduLength(
      std::min(std::max(maxAmpduSize, 8388607U), 16777215U));

  capabilities.m_phyCapabilities
      .supportTx1024And4096QamForRuSmallerThan242Tones =
      (maxSupportedMcs >= 12) ? 1 : 0;
  capabilities.m_phyCapabilities
      .supportRx1024And4096QamForRuSmallerThan242Tones =
      (maxSupportedMcs >= 12) ? 1 : 0;
  if (m_channelWidth == 20) {
    for (auto maxMcs : {7, 9, 11, 13}) {
      capabilities.SetSupportedRxEhtMcsAndNss(
          EhtMcsAndNssSet::EHT_MCS_MAP_TYPE_20_MHZ_ONLY, maxMcs,
          maxMcs <= maxSupportedMcs ? 1 : 0);
      capabilities.SetSupportedTxEhtMcsAndNss(
          EhtMcsAndNssSet::EHT_MCS_MAP_TYPE_20_MHZ_ONLY, maxMcs,
          maxMcs <= maxSupportedMcs ? 2 : 0);
    }
  } else {
    for (auto maxMcs : {9, 11, 13}) {
      capabilities.SetSupportedRxEhtMcsAndNss(
          EhtMcsAndNssSet::EHT_MCS_MAP_TYPE_NOT_LARGER_THAN_80_MHZ, maxMcs,
          maxMcs <= maxSupportedMcs ? 3 : 0);
      capabilities.SetSupportedTxEhtMcsAndNss(
          EhtMcsAndNssSet::EHT_MCS_MAP_TYPE_NOT_LARGER_THAN_80_MHZ, maxMcs,
          maxMcs <= maxSupportedMcs ? 4 : 0);
    }
  }
  if (m_channelWidth >= 160) {
    for (auto maxMcs : {9, 11, 13}) {
      capabilities.SetSupportedRxEhtMcsAndNss(
          EhtMcsAndNssSet::EHT_MCS_MAP_TYPE_160_MHZ, maxMcs,
          maxMcs <= maxSupportedMcs ? 2 : 0);
      capabilities.SetSupportedTxEhtMcsAndNss(
          EhtMcsAndNssSet::EHT_MCS_MAP_TYPE_160_MHZ, maxMcs,
          maxMcs <= maxSupportedMcs ? 1 : 0);
    }
  }
  if (m_channelWidth == 320) {
    capabilities.m_phyCapabilities.support320MhzIn6Ghz = 1;
    for (auto maxMcs : {9, 11, 13}) {
      capabilities.SetSupportedRxEhtMcsAndNss(
          EhtMcsAndNssSet::EHT_MCS_MAP_TYPE_320_MHZ, maxMcs,
          maxMcs <= maxSupportedMcs ? 4 : 0);
      capabilities.SetSupportedTxEhtMcsAndNss(
          EhtMcsAndNssSet::EHT_MCS_MAP_TYPE_320_MHZ, maxMcs,
          maxMcs <= maxSupportedMcs ? 3 : 0);
    }
  } else {
    capabilities.m_phyCapabilities.support320MhzIn6Ghz = 0;
  }

  return capabilities;
}

Buffer WifiEhtCapabilitiesIeTest::SerializeIntoBuffer(
    const EhtCapabilities &ehtCapabilities) {
  Buffer buffer;
  buffer.AddAtStart(ehtCapabilities.GetSerializedSize());
  ehtCapabilities.Serialize(buffer.Begin());
  return buffer;
}

void WifiEhtCapabilitiesIeTest::CheckSerializedByte(const Buffer &buffer,
                                                    uint32_t position,
                                                    uint8_t value) {
  Buffer::Iterator it = buffer.Begin();
  it.Next(position);
  uint8_t byte = it.ReadU8();
  NS_TEST_EXPECT_MSG_EQ(+byte, +value, "Unexpected byte at pos=" << position);
}

void WifiEhtCapabilitiesIeTest::CheckEhtMacCapabilitiesInformation(
    const Buffer &buffer, uint8_t expectedValueFirstByte) {
  CheckSerializedByte(buffer, 3, expectedValueFirstByte);
  CheckSerializedByte(buffer, 4, 0x00);
}

void WifiEhtCapabilitiesIeTest::CheckEhtPhyCapabilitiesInformation(
    const Buffer &buffer, uint8_t expectedValueSixthByte) {
  CheckSerializedByte(buffer, 5, (m_channelWidth == 320) ? 0x02 : 0x00);
  CheckSerializedByte(buffer, 6, 0x00);
  CheckSerializedByte(buffer, 7, 0x00);
  CheckSerializedByte(buffer, 8, 0x00);
  CheckSerializedByte(buffer, 9, 0x00);
  CheckSerializedByte(buffer, 10, expectedValueSixthByte);
  CheckSerializedByte(buffer, 11, 0x00);
  CheckSerializedByte(buffer, 12, 0x00);
  CheckSerializedByte(buffer, 13, 0x00);
}

void WifiEhtCapabilitiesIeTest::CheckSupportedEhtMcsAndNssSet(
    const Buffer &buffer, uint8_t maxSupportedMcs) {
  if (m_channelWidth == 20) {
    CheckSerializedByte(buffer, 14, 0x21);
    CheckSerializedByte(buffer, 15, maxSupportedMcs >= 8 ? 0x21 : 0x00);
    CheckSerializedByte(buffer, 16, maxSupportedMcs >= 10 ? 0x21 : 0x00);
    CheckSerializedByte(buffer, 17, maxSupportedMcs >= 12 ? 0x21 : 0x00);
  } else {
    CheckSerializedByte(buffer, 14, 0x43);
    CheckSerializedByte(buffer, 15, maxSupportedMcs >= 10 ? 0x43 : 0x00);
    CheckSerializedByte(buffer, 16, maxSupportedMcs >= 12 ? 0x43 : 0x00);
  }
  if (m_channelWidth >= 160) {
    CheckSerializedByte(buffer, 17, 0x12);
    CheckSerializedByte(buffer, 18, maxSupportedMcs >= 10 ? 0x12 : 0x00);
    CheckSerializedByte(buffer, 19, maxSupportedMcs >= 12 ? 0x12 : 0x00);
  }
  if (m_channelWidth == 320) {
    CheckSerializedByte(buffer, 20, 0x34);
    CheckSerializedByte(buffer, 21, maxSupportedMcs >= 10 ? 0x34 : 0x00);
    CheckSerializedByte(buffer, 22, maxSupportedMcs >= 12 ? 0x34 : 0x00);
  }
}

void WifiEhtCapabilitiesIeTest::DoRun() {
  uint8_t maxMcs = 0;
  uint16_t expectedEhtMcsAndNssSetSize = 0;
  switch (m_channelWidth) {
  case 20:
    expectedEhtMcsAndNssSetSize = 4;
    break;
  case 40:
  case 80:
    expectedEhtMcsAndNssSetSize = 3;
    break;
  case 160:
    expectedEhtMcsAndNssSetSize = (2 * 3);
    break;
  case 320:
    expectedEhtMcsAndNssSetSize = (3 * 3);
    break;
  default:
    NS_ASSERT_MSG(false, "Invalid upper channel width " << m_channelWidth);
  }

  uint16_t expectedSize = 1 + 1 + 1 + 2 + 9 + expectedEhtMcsAndNssSetSize;

  auto mapType = m_channelWidth == 20
                     ? EhtMcsAndNssSet::EHT_MCS_MAP_TYPE_20_MHZ_ONLY
                     : EhtMcsAndNssSet::EHT_MCS_MAP_TYPE_NOT_LARGER_THAN_80_MHZ;

  {
    maxMcs = 11;
    HeCapabilities heCapabilities = GetHeCapabilities();
    EhtCapabilities ehtCapabilities = GetEhtCapabilities(3895, 65535, maxMcs);

    NS_ASSERT(ehtCapabilities.GetHighestSupportedRxMcs(mapType) == maxMcs);
    NS_ASSERT(ehtCapabilities.GetHighestSupportedTxMcs(mapType) == maxMcs);

    NS_TEST_EXPECT_MSG_EQ(ehtCapabilities.GetSerializedSize(), expectedSize,
                          "Unexpected header size");

    Buffer buffer = SerializeIntoBuffer(ehtCapabilities);

    CheckEhtMacCapabilitiesInformation(buffer, 0x00);

    CheckEhtPhyCapabilitiesInformation(buffer, 0x00);

    CheckSupportedEhtMcsAndNssSet(buffer, maxMcs);

    TestHeaderSerialization(ehtCapabilities, m_is2_4Ghz, heCapabilities);
  }

  {
    maxMcs = 11;
    HeCapabilities heCapabilities = GetHeCapabilities();
    EhtCapabilities ehtCapabilities = GetEhtCapabilities(11454, 65535, maxMcs);

    NS_ASSERT(ehtCapabilities.GetHighestSupportedRxMcs(mapType) == maxMcs);
    NS_ASSERT(ehtCapabilities.GetHighestSupportedTxMcs(mapType) == maxMcs);

    NS_TEST_EXPECT_MSG_EQ(ehtCapabilities.GetSerializedSize(), expectedSize,
                          "Unexpected header size");

    Buffer buffer = SerializeIntoBuffer(ehtCapabilities);

    CheckEhtMacCapabilitiesInformation(buffer, m_is2_4Ghz ? 0x80 : 0x00);

    CheckEhtPhyCapabilitiesInformation(buffer, 0x00);

    CheckSupportedEhtMcsAndNssSet(buffer, maxMcs);

    TestHeaderSerialization(ehtCapabilities, m_is2_4Ghz, heCapabilities);
  }

  {
    maxMcs = 13;
    HeCapabilities heCapabilities = GetHeCapabilities();
    EhtCapabilities ehtCapabilities = GetEhtCapabilities(3895, 65535, maxMcs);

    NS_ASSERT(ehtCapabilities.GetHighestSupportedRxMcs(mapType) == maxMcs);
    NS_ASSERT(ehtCapabilities.GetHighestSupportedTxMcs(mapType) == maxMcs);

    NS_TEST_EXPECT_MSG_EQ(ehtCapabilities.GetSerializedSize(), expectedSize,
                          "Unexpected header size");

    Buffer buffer = SerializeIntoBuffer(ehtCapabilities);

    CheckEhtMacCapabilitiesInformation(buffer, 0x00);

    CheckEhtPhyCapabilitiesInformation(buffer, 0x06);

    CheckSupportedEhtMcsAndNssSet(buffer, maxMcs);

    TestHeaderSerialization(ehtCapabilities, m_is2_4Ghz, heCapabilities);
  }

  {
    maxMcs = 11;
    HeCapabilities heCapabilities = GetHeCapabilities();
    EhtCapabilities ehtCapabilities = GetEhtCapabilities(3895, 65535, maxMcs);

    NS_ASSERT(ehtCapabilities.GetHighestSupportedRxMcs(mapType) == maxMcs);
    NS_ASSERT(ehtCapabilities.GetHighestSupportedTxMcs(mapType) == maxMcs);

    std::vector<std::pair<uint8_t, uint8_t>> ppeThresholds;
    ppeThresholds.emplace_back(1, 2);
    ppeThresholds.emplace_back(2, 3);
    ppeThresholds.emplace_back(3, 4);
    ppeThresholds.emplace_back(4, 3);
    ppeThresholds.emplace_back(3, 2);
    ppeThresholds.emplace_back(2, 1);
    ehtCapabilities.SetPpeThresholds(2, 0x03, ppeThresholds);

    expectedSize += 6;

    NS_TEST_EXPECT_MSG_EQ(ehtCapabilities.GetSerializedSize(), expectedSize,
                          "Unexpected header size");

    Buffer buffer = SerializeIntoBuffer(ehtCapabilities);

    CheckEhtMacCapabilitiesInformation(buffer, 0x00);

    CheckEhtPhyCapabilitiesInformation(buffer, 0x08);

    CheckSupportedEhtMcsAndNssSet(buffer, maxMcs);

    TestHeaderSerialization(ehtCapabilities, m_is2_4Ghz, heCapabilities);
  }
}

class TidToLinkMappingElementTest : public HeaderSerializationTestCase {
public:
  TidToLinkMappingElementTest(WifiDirection direction,
                              std::optional<Time> mappingSwitchTime,
                              std::optional<Time> expectedDuration,
                              const WifiTidLinkMapping &mappings);

  ~TidToLinkMappingElementTest() override = default;

private:
  void DoSetup() override;
  void DoRun() override;

  WifiDirection m_direction;
  std::optional<Time> m_mappingSwitchTime;
  std::optional<Time> m_expectedDuration;
  WifiTidLinkMapping m_mappings;
  TidToLinkMapping m_tidToLinkMapping;
};

TidToLinkMappingElementTest::TidToLinkMappingElementTest(
    WifiDirection direction, std::optional<Time> mappingSwitchTime,
    std::optional<Time> expectedDuration, const WifiTidLinkMapping &mappings)
    : HeaderSerializationTestCase("Check serialization and deserialization of "
                                  "TID-To-Link Mapping elements"),
      m_direction(direction), m_mappingSwitchTime(mappingSwitchTime),
      m_expectedDuration(expectedDuration), m_mappings(mappings) {}

void TidToLinkMappingElementTest::DoSetup() {
  m_tidToLinkMapping.m_control.direction = m_direction;
  m_tidToLinkMapping.m_control.defaultMapping = true;

  if (m_mappingSwitchTime) {
    m_tidToLinkMapping.SetMappingSwitchTime(*m_mappingSwitchTime);
    std::optional<Time> encoded = m_tidToLinkMapping.GetMappingSwitchTime();
    NS_TEST_ASSERT_MSG_EQ(encoded.has_value(), true,
                          "Mapping Switch Time should be present");
    NS_TEST_EXPECT_MSG_EQ(*m_mappingSwitchTime, *encoded,
                          "Incorrect Mapping Switch Time value");
  }
  if (m_expectedDuration) {
    m_tidToLinkMapping.SetExpectedDuration(*m_expectedDuration);
    std::optional<Time> encoded = m_tidToLinkMapping.GetExpectedDuration();
    NS_TEST_ASSERT_MSG_EQ(encoded.has_value(), true,
                          "Expected Duration should be present");
    NS_TEST_EXPECT_MSG_EQ(*m_expectedDuration, *encoded,
                          "Incorrect Expected Duration value");
  }

  for (const auto &[tid, linkSet] : m_mappings) {
    m_tidToLinkMapping.m_control.defaultMapping = false;
    m_tidToLinkMapping.SetLinkMappingOfTid(tid, linkSet);
    NS_TEST_EXPECT_MSG_EQ(
        (m_tidToLinkMapping.GetLinkMappingOfTid(tid) == linkSet), true,
        "Incorrect link set for TID " << +tid);
  }
}

void TidToLinkMappingElementTest::DoRun() {
  TestHeaderSerialization(m_tidToLinkMapping);

  Simulator::Destroy();
}

class EhtOperationElementTest : public HeaderSerializationTestCase {
public:
  EhtOperationElementTest(const EhtOperation::EhtOpParams &params,
                          uint8_t rxMaxNss0_7, uint8_t txMaxNss0_7,
                          uint8_t rxMaxNss8_9, uint8_t txMaxNss8_9,
                          uint8_t rxMaxNss10_11, uint8_t txMaxNss10_11,
                          uint8_t rxMaxNss12_13, uint8_t txMaxNss12_13,
                          std::optional<EhtOperation::EhtOpInfo> opInfo);

  ~EhtOperationElementTest() override = default;

private:
  void DoRun() override;

  EhtOperation m_ehtOperation;
};

EhtOperationElementTest::EhtOperationElementTest(
    const EhtOperation::EhtOpParams &params, uint8_t rxMaxNss0_7,
    uint8_t txMaxNss0_7, uint8_t rxMaxNss8_9, uint8_t txMaxNss8_9,
    uint8_t rxMaxNss10_11, uint8_t txMaxNss10_11, uint8_t rxMaxNss12_13,
    uint8_t txMaxNss12_13, std::optional<EhtOperation::EhtOpInfo> opInfo)
    : HeaderSerializationTestCase(
          "Check serialization and deserialization of EHT Operation elements") {
  m_ehtOperation.m_params = params;
  m_ehtOperation.SetMaxRxNss(rxMaxNss0_7, 0, 7);
  m_ehtOperation.SetMaxTxNss(txMaxNss0_7, 0, 7);
  m_ehtOperation.SetMaxRxNss(rxMaxNss8_9, 8, 9);
  m_ehtOperation.SetMaxTxNss(txMaxNss8_9, 8, 9);
  m_ehtOperation.SetMaxRxNss(rxMaxNss10_11, 10, 11);
  m_ehtOperation.SetMaxTxNss(txMaxNss10_11, 10, 11);
  m_ehtOperation.SetMaxRxNss(rxMaxNss12_13, 12, 13);
  m_ehtOperation.SetMaxTxNss(txMaxNss12_13, 12, 13);
  m_ehtOperation.m_opInfo = opInfo;
}

void EhtOperationElementTest::DoRun() {
  TestHeaderSerialization(m_ehtOperation);
}

class WifiEhtInfoElemsTestSuite : public TestSuite {
public:
  WifiEhtInfoElemsTestSuite();
};

WifiEhtInfoElemsTestSuite::WifiEhtInfoElemsTestSuite()
    : TestSuite("wifi-eht-info-elems", UNIT) {
  AddTestCase(new BasicMultiLinkElementTest(), TestCase::QUICK);
  AddTestCase(new ReducedNeighborReportTest(), TestCase::QUICK);
  AddTestCase(new WifiEhtCapabilitiesIeTest(false, 20), TestCase::QUICK);
  AddTestCase(new WifiEhtCapabilitiesIeTest(true, 20), TestCase::QUICK);
  AddTestCase(new WifiEhtCapabilitiesIeTest(false, 80), TestCase::QUICK);
  AddTestCase(new WifiEhtCapabilitiesIeTest(true, 40), TestCase::QUICK);
  AddTestCase(new WifiEhtCapabilitiesIeTest(true, 80), TestCase::QUICK);
  AddTestCase(new WifiEhtCapabilitiesIeTest(false, 160), TestCase::QUICK);
  AddTestCase(new WifiEhtCapabilitiesIeTest(false, 320), TestCase::QUICK);
  AddTestCase(new TidToLinkMappingElementTest(WifiDirection::DOWNLINK,
                                              std::nullopt, std::nullopt, {}),
              TestCase::QUICK);
  AddTestCase(new TidToLinkMappingElementTest(
                  WifiDirection::UPLINK, MicroSeconds(500 * 1024),
                  MicroSeconds(300 * 1024), {{3, std::set<uint8_t>{0, 4, 6}}}),
              TestCase::QUICK);
  AddTestCase(
      new TidToLinkMappingElementTest(WifiDirection::BOTH_DIRECTIONS,
                                      std::nullopt, MicroSeconds(100 * 1024),
                                      {{3, std::set<uint8_t>{0, 4, 6}},
                                       {6, std::set<uint8_t>{3, 7, 11, 14}}}),
      TestCase::QUICK);
  AddTestCase(
      new TidToLinkMappingElementTest(WifiDirection::DOWNLINK,
                                      MicroSeconds(100 * 1024), std::nullopt,
                                      {{0, std::set<uint8_t>{0, 1, 2}},
                                       {1, std::set<uint8_t>{3, 4, 5}},
                                       {2, std::set<uint8_t>{6, 7}},
                                       {3, std::set<uint8_t>{8, 9, 10}},
                                       {4, std::set<uint8_t>{11, 12, 13}},
                                       {5, std::set<uint8_t>{14}},
                                       {6, std::set<uint8_t>{1, 3, 6}},
                                       {7, std::set<uint8_t>{11, 14}}}),
      TestCase::QUICK);
  AddTestCase(new EhtOperationElementTest({0, 0, 0, 0, 0}, 1, 2, 3, 4, 5, 6, 7,
                                          8, std::nullopt),
              TestCase::QUICK);
  AddTestCase(new EhtOperationElementTest({1, 0, 0, 1, 0}, 1, 2, 3, 4, 5, 6, 7,
                                          8,
                                          EhtOperation::EhtOpInfo{{1}, 3, 5}),
              TestCase::QUICK);
  AddTestCase(
      new EhtOperationElementTest({1, 1, 1, 1, 2}, 1, 2, 3, 4, 5, 6, 7, 8,
                                  EhtOperation::EhtOpInfo{{2}, 4, 6, 3000}),
      TestCase::QUICK);
}

static WifiEhtInfoElemsTestSuite g_wifiEhtInfoElemsTestSuite;
