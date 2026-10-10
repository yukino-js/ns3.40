
#include "ns3/core-module.h"
#include "ns3/dsr-module.h"
#include "ns3/internet-module.h"
#include "ns3/test.h"

#include "ns3/lr-wpan-mac.h"
#include "ns3/lte-module.h"
#include "ns3/mesh-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/olsr-module.h"
#include "ns3/sixlowpan-module.h"
#include "ns3/spectrum-module.h"
#include "ns3/stats-module.h"
#include "ns3/uan-module.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/wifi-phy-state-helper.h"
#include <iostream>
#include <set>
#include <sstream>
#include <string>

using namespace ns3;

class TracedCallbackTypedefTestCase : public TestCase {
public:
  TracedCallbackTypedefTestCase();

  ~TracedCallbackTypedefTestCase() override {}

  static std::size_t m_nArgs;

private:
  template <typename... Ts> class Checker;

  void DoRun() override;
};

namespace {

std::set<std::string> Duplicates() {
  std::set<std::string> dupes;

  dupes.insert("LteRlc::NotifyTxTracedCallback");
  dupes.insert("LteRlc::ReceiveTracedCallback");
  dupes.insert("LteUeRrc::ImsiCidRntiTracedCallback");
  dupes.insert("LteUeRrc::MibSibHandoverTracedCallback");
  dupes.insert("WifiPhyStateHelper::RxEndErrorTracedCallback");

  return dupes;
}

std::set<std::string> g_dupes = Duplicates();

template <typename T> inline std::string TypeName(int N) { return "unknown"; }

#define TYPENAME(T)                                                            \
  template <> inline std::string TypeName<T>(int N) {                          \
    std::stringstream ss;                                                      \
    ss << #T << "(" << N << ")";                                               \
    return ss.str();                                                           \
  }

TYPENAME(dsr::DsrOptionSRHeader::TracedCallback);
TYPENAME(EpcUeNas::StateTracedCallback);
TYPENAME(Ipv4L3Protocol::DropTracedCallback);
TYPENAME(Ipv4L3Protocol::SentTracedCallback);
TYPENAME(Ipv4L3Protocol::TxRxTracedCallback);
TYPENAME(Ipv6L3Protocol::DropTracedCallback);
TYPENAME(Ipv6L3Protocol::SentTracedCallback);
TYPENAME(Ipv6L3Protocol::TxRxTracedCallback);
TYPENAME(LrWpanMac::SentTracedCallback);
TYPENAME(LrWpanMac::StateTracedCallback);
TYPENAME(LrWpanPhy::StateTracedCallback);
TYPENAME(LteEnbMac::DlSchedulingTracedCallback);
TYPENAME(LteEnbMac::UlSchedulingTracedCallback);
TYPENAME(LteEnbPhy::ReportInterferenceTracedCallback);
TYPENAME(LteEnbPhy::ReportUeSinrTracedCallback);
TYPENAME(LteEnbRrc::ConnectionHandoverTracedCallback);
TYPENAME(LteEnbRrc::HandoverStartTracedCallback);
TYPENAME(LteEnbRrc::NewUeContextTracedCallback);
TYPENAME(LteEnbRrc::ReceiveReportTracedCallback);
TYPENAME(LtePdcp::PduRxTracedCallback);
TYPENAME(LtePdcp::PduTxTracedCallback);
TYPENAME(LteUePhy::StateTracedCallback);
TYPENAME(LteUePhy::RsrpSinrTracedCallback);
TYPENAME(LteUeRrc::CellSelectionTracedCallback);
TYPENAME(LteUeRrc::StateTracedCallback);
TYPENAME(Mac48Address::TracedCallback);
TYPENAME(MobilityModel::TracedCallback);
TYPENAME(olsr::RoutingProtocol::PacketTxRxTracedCallback);
TYPENAME(olsr::RoutingProtocol::TableChangeTracedCallback);
TYPENAME(Packet::AddressTracedCallback);
TYPENAME(Packet::Mac48AddressTracedCallback);
TYPENAME(Packet::SinrTracedCallback);
TYPENAME(Packet::SizeTracedCallback);
TYPENAME(Packet::TracedCallback);
TYPENAME(PacketBurst::TracedCallback);
TYPENAME(dot11s::PeerManagementProtocol::LinkOpenCloseTracedCallback);
TYPENAME(PhyReceptionStatParameters::TracedCallback);
TYPENAME(PhyTransmissionStatParameters::TracedCallback);
TYPENAME(SixLowPanNetDevice::DropTracedCallback);
TYPENAME(SixLowPanNetDevice::RxTxTracedCallback);
TYPENAME(SpectrumChannel::LossTracedCallback);
TYPENAME(SpectrumValue::TracedCallback);
TYPENAME(TimeSeriesAdaptor::OutputTracedCallback);
TYPENAME(UanMac::PacketModeTracedCallback);
TYPENAME(UanMacCw::QueueTracedCallback);
TYPENAME(UanMacRc::QueueTracedCallback);
TYPENAME(UanNetDevice::RxTxTracedCallback);
TYPENAME(UanPhy::TracedCallback);
TYPENAME(UeManager::StateTracedCallback);
TYPENAME(WifiMacHeader::TracedCallback);
TYPENAME(WifiPhyStateHelper::RxOkTracedCallback);
TYPENAME(WifiPhyStateHelper::StateTracedCallback);
TYPENAME(WifiPhyStateHelper::TxTracedCallback);
TYPENAME(WifiRemoteStationManager::PowerChangeTracedCallback);
TYPENAME(WifiRemoteStationManager::RateChangeTracedCallback);
#undef TYPENAME

void SinkIt(std::size_t N) {
  std::cout << "with " << N << " args." << std::endl;
  TracedCallbackTypedefTestCase::m_nArgs = N;
}

template <typename... Ts> class TracedCbSink {
public:
  static void Sink(Ts...) {
    const std::size_t n = sizeof...(Ts);
    SinkIt(n);
  }
};

} // namespace

std::size_t TracedCallbackTypedefTestCase::m_nArgs = 0;

template <typename... Ts>
class TracedCallbackTypedefTestCase::Checker : public Object {
  TracedCallback<Ts...> m_cb;

public:
  Checker() {};
  ~Checker() override {};

  std::tuple<typename TypeTraits<Ts>::BaseType...> m_items;

  const std::size_t m_nItems = sizeof...(Ts);

  template <typename U> void Invoke() {
    U sink = TracedCbSink<Ts...>::Sink;
    Callback<void, Ts...> cb = MakeCallback(sink);

    std::cout << TypeName<U>(m_nItems) << " invoked ";
    m_cb.ConnectWithoutContext(cb);
    std::apply(m_cb, m_items);
    Cleanup();
  }

  void Cleanup() {
    if (m_nArgs == 0) {
      std::cout << std::endl;
    }
    NS_ASSERT_MSG(m_nArgs && m_nArgs == m_nItems,
                  "failed, m_nArgs: " << m_nArgs << " N: " << m_nItems);
    m_nArgs = 0;
  }
};

TracedCallbackTypedefTestCase::TracedCallbackTypedefTestCase()
    : TestCase("Check basic TracedCallback operation") {}

#define DUPE(U, T1)                                                            \
  if (g_dupes.find(#U) == g_dupes.end()) {                                     \
    NS_TEST_ASSERT_MSG_NE(0, 1, "expected to find " << #U << " in dupes.");    \
  }                                                                            \
  if (TypeName<U>(0) == TypeName<T1>(0)) {                                     \
    std::cout << #U << " matches " << #T1 << std::endl;                        \
  } else {                                                                     \
    NS_TEST_ASSERT_MSG_EQ(                                                     \
        TypeName<U>(0), TypeName<T1>(0),                                       \
        "the typedef "                                                         \
            << #U << " used to match the typedef " << #T1                      \
            << " but no longer does.  Please add a new CHECK call.");          \
  }

#define CHECK(U, ...) CreateObject<Checker<__VA_ARGS__>>()->Invoke<U>()

void TracedCallbackTypedefTestCase::DoRun() {
  CHECK(dsr::DsrOptionSRHeader::TracedCallback, const dsr::DsrOptionSRHeader &);

  CHECK(EpcUeNas::StateTracedCallback, EpcUeNas::State, EpcUeNas::State);

  CHECK(Ipv4L3Protocol::DropTracedCallback, const Ipv4Header &,
        Ptr<const Packet>, Ipv4L3Protocol::DropReason, Ptr<Ipv4>, uint32_t);

  CHECK(Ipv4L3Protocol::SentTracedCallback, const Ipv4Header &,
        Ptr<const Packet>, uint32_t);

  CHECK(Ipv4L3Protocol::TxRxTracedCallback, Ptr<const Packet>, Ptr<Ipv4>,
        uint32_t);

  CHECK(Ipv6L3Protocol::DropTracedCallback, const Ipv6Header &,
        Ptr<const Packet>, Ipv6L3Protocol::DropReason, Ptr<Ipv6>, uint32_t);

  CHECK(Ipv6L3Protocol::SentTracedCallback, const Ipv6Header &,
        Ptr<const Packet>, uint32_t);

  CHECK(Ipv6L3Protocol::TxRxTracedCallback, Ptr<const Packet>, Ptr<Ipv6>,
        uint32_t);

  CHECK(LrWpanMac::SentTracedCallback, Ptr<const Packet>, uint8_t, uint8_t);

  CHECK(LrWpanMac::StateTracedCallback, LrWpanMacState, LrWpanMacState);

  CHECK(LrWpanPhy::StateTracedCallback, Time, LrWpanPhyEnumeration,
        LrWpanPhyEnumeration);

  CHECK(LteEnbMac::DlSchedulingTracedCallback, uint32_t, uint32_t, uint16_t,
        uint8_t, uint16_t, uint8_t, uint16_t, uint8_t);

  CHECK(LteEnbMac::UlSchedulingTracedCallback, uint32_t, uint32_t, uint16_t,
        uint8_t, uint16_t);

  CHECK(LteEnbPhy::ReportUeSinrTracedCallback, uint16_t, uint16_t, double,
        uint8_t);

  CHECK(LteEnbPhy::ReportInterferenceTracedCallback, uint16_t,
        Ptr<SpectrumValue>);

  CHECK(LteEnbRrc::ConnectionHandoverTracedCallback, uint64_t, uint16_t,
        uint16_t);

  CHECK(LteEnbRrc::HandoverStartTracedCallback, uint64_t, uint16_t, uint16_t,
        uint16_t);

  CHECK(LteEnbRrc::NewUeContextTracedCallback, uint16_t, uint16_t);

  CHECK(LteEnbRrc::ReceiveReportTracedCallback, uint64_t, uint16_t, uint16_t,
        LteRrcSap::MeasurementReport);

  CHECK(LtePdcp::PduRxTracedCallback, uint16_t, uint8_t, uint32_t, uint64_t);

  CHECK(LtePdcp::PduTxTracedCallback, uint16_t, uint8_t, uint32_t);

  DUPE(LteRlc::NotifyTxTracedCallback, LtePdcp::PduTxTracedCallback);

  DUPE(LteRlc::ReceiveTracedCallback, LtePdcp::PduRxTracedCallback);

  CHECK(LteUePhy::RsrpSinrTracedCallback, uint16_t, uint16_t, double, double,
        uint8_t);

  CHECK(LteUePhy::StateTracedCallback, uint16_t, uint16_t, LteUePhy::State,
        LteUePhy::State);

  CHECK(LteUeRrc::CellSelectionTracedCallback, uint64_t, uint16_t);

  DUPE(LteUeRrc::ImsiCidRntiTracedCallback,
       LteEnbRrc::ConnectionHandoverTracedCallback);

  DUPE(LteUeRrc::MibSibHandoverTracedCallback,
       LteEnbRrc::HandoverStartTracedCallback);

  CHECK(LteUeRrc::StateTracedCallback, uint64_t, uint16_t, uint16_t,
        LteUeRrc::State, LteUeRrc::State);

  CHECK(Mac48Address::TracedCallback, Mac48Address);

  CHECK(MobilityModel::TracedCallback, Ptr<const MobilityModel>);

  CHECK(olsr::RoutingProtocol::PacketTxRxTracedCallback,
        const olsr::PacketHeader &, const olsr::MessageList &);

  CHECK(olsr::RoutingProtocol::TableChangeTracedCallback, uint32_t);

  CHECK(Packet::AddressTracedCallback, Ptr<const Packet>, const Address &);

  CHECK(Packet::Mac48AddressTracedCallback, Ptr<const Packet>, Mac48Address);

  CHECK(Packet::SinrTracedCallback, Ptr<const Packet>, double);

  CHECK(Packet::SizeTracedCallback, uint32_t, uint32_t);

  CHECK(Packet::TracedCallback, Ptr<const Packet>);

  CHECK(PacketBurst::TracedCallback, Ptr<const PacketBurst>);

  CHECK(dot11s::PeerManagementProtocol::LinkOpenCloseTracedCallback,
        Mac48Address, Mac48Address);

  CHECK(PhyReceptionStatParameters::TracedCallback, PhyReceptionStatParameters);

  CHECK(PhyTransmissionStatParameters::TracedCallback,
        PhyTransmissionStatParameters);

  CHECK(SixLowPanNetDevice::DropTracedCallback, SixLowPanNetDevice::DropReason,
        Ptr<const Packet>, Ptr<SixLowPanNetDevice>, uint32_t);

  CHECK(SixLowPanNetDevice::RxTxTracedCallback, Ptr<const Packet>,
        Ptr<SixLowPanNetDevice>, uint32_t);

  CHECK(SpectrumChannel::LossTracedCallback, Ptr<const SpectrumPhy>,
        Ptr<const SpectrumPhy>, double);

  CHECK(SpectrumValue::TracedCallback, Ptr<SpectrumValue>);

  CHECK(TimeSeriesAdaptor::OutputTracedCallback, double, double);

  CHECK(UanMac::PacketModeTracedCallback, Ptr<const Packet>, UanTxMode);

  CHECK(UanMacCw::QueueTracedCallback, Ptr<const Packet>, uint16_t);

  CHECK(UanMacRc::QueueTracedCallback, Ptr<const Packet>, uint32_t);

  CHECK(UanNetDevice::RxTxTracedCallback, Ptr<const Packet>, Mac8Address);

  CHECK(UanPhy::TracedCallback, Ptr<const Packet>, double, UanTxMode);

  CHECK(UeManager::StateTracedCallback, uint64_t, uint16_t, uint16_t,
        UeManager::State, UeManager::State);

  CHECK(WifiMacHeader::TracedCallback, const WifiMacHeader &);

  CHECK(WifiPhyStateHelper::RxEndErrorTracedCallback, Ptr<const Packet>,
        double);

  CHECK(WifiPhyStateHelper::RxOkTracedCallback, Ptr<const Packet>, double,
        WifiMode, WifiPreamble);

  CHECK(WifiPhyStateHelper::StateTracedCallback, Time, Time, WifiPhyState);

  CHECK(WifiPhyStateHelper::TxTracedCallback, Ptr<const Packet>, WifiMode,
        WifiPreamble, uint8_t);

  CHECK(WifiRemoteStationManager::PowerChangeTracedCallback, double, double,
        Mac48Address);

  CHECK(WifiRemoteStationManager::RateChangeTracedCallback, DataRate, DataRate,
        Mac48Address);
}

class TracedCallbackTypedefTestSuite : public TestSuite {
public:
  TracedCallbackTypedefTestSuite();
};

TracedCallbackTypedefTestSuite::TracedCallbackTypedefTestSuite()
    : TestSuite("traced-callback-typedef", SYSTEM) {
  AddTestCase(new TracedCallbackTypedefTestCase, TestCase::QUICK);
}

static TracedCallbackTypedefTestSuite tracedCallbackTypedefTestSuite;
