#include "tcp-error-model.h"
#include "tcp-general-test.h"

#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/random-variable-stream.h"
#include "ns3/tcp-rx-buffer.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpAdvertisedWindowTestSuite");

class TcpSocketAdvertisedWindowProxy : public TcpSocketMsgBase {
public:
  static TypeId GetTypeId();

  typedef Callback<void, uint16_t, uint16_t> InvalidAwndCallback;

  TcpSocketAdvertisedWindowProxy() : TcpSocketMsgBase(), m_segmentSize(0) {}

  TcpSocketAdvertisedWindowProxy(const TcpSocketAdvertisedWindowProxy &other)
      : TcpSocketMsgBase(other) {
    m_segmentSize = other.m_segmentSize;
    m_inwalidAwndCb = other.m_inwalidAwndCb;
  }

  void SetInvalidAwndCb(InvalidAwndCallback cb);

  void SetExpectedSegmentSize(uint16_t seg) { m_segmentSize = seg; };

protected:
  Ptr<TcpSocketBase> Fork() override;
  uint16_t AdvertisedWindowSize(bool scale = true) const override;

private:
  uint16_t OldAdvertisedWindowSize(bool scale = true) const;
  InvalidAwndCallback m_inwalidAwndCb;

  uint16_t m_segmentSize;
};

void TcpSocketAdvertisedWindowProxy::SetInvalidAwndCb(InvalidAwndCallback cb) {
  NS_ASSERT(!cb.IsNull());
  m_inwalidAwndCb = cb;
}

TypeId TcpSocketAdvertisedWindowProxy::GetTypeId() {
  static TypeId tid = TypeId("ns3::TcpSocketAdvertisedWindowProxy")
                          .SetParent<TcpSocketMsgBase>()
                          .SetGroupName("Internet")
                          .AddConstructor<TcpSocketAdvertisedWindowProxy>();
  return tid;
}

Ptr<TcpSocketBase> TcpSocketAdvertisedWindowProxy::Fork() {
  return CopyObject<TcpSocketAdvertisedWindowProxy>(this);
}

uint16_t
TcpSocketAdvertisedWindowProxy::AdvertisedWindowSize(bool scale) const {
  NS_LOG_FUNCTION(this << scale);

  uint16_t newAwnd = TcpSocketMsgBase::AdvertisedWindowSize(scale);
  uint16_t oldAwnd = OldAdvertisedWindowSize(scale);

  if (!m_tcb->m_rxBuffer->Finished()) {
    if (newAwnd != oldAwnd) {
      uint32_t available = m_tcb->m_rxBuffer->Available();
      uint32_t newAwndKnownDifference = newAwnd;
      if (scale) {
        newAwndKnownDifference += (available >> m_rcvWindShift);
      } else {
        newAwndKnownDifference += available;
      }

      if (newAwndKnownDifference > m_maxWinSize) {
        newAwndKnownDifference = m_maxWinSize;
      }

      if (static_cast<uint16_t>(newAwndKnownDifference) != oldAwnd) {
        if (!m_inwalidAwndCb.IsNull()) {
          m_inwalidAwndCb(oldAwnd, newAwnd);
        }
      }
    }
  }

  return newAwnd;
}

uint16_t
TcpSocketAdvertisedWindowProxy::OldAdvertisedWindowSize(bool scale) const {
  NS_LOG_FUNCTION(this << scale);
  uint32_t w = m_tcb->m_rxBuffer->MaxBufferSize();

  if (scale) {
    w >>= m_rcvWindShift;
  }
  if (w > m_maxWinSize) {
    w = m_maxWinSize;
    NS_LOG_WARN("Adv window size truncated to "
                << m_maxWinSize
                << "; possibly to avoid overflow of the 16-bit integer");
  }
  NS_LOG_DEBUG("Returning AdvertisedWindowSize of "
               << static_cast<uint16_t>(w));
  return static_cast<uint16_t>(w);
}

NS_OBJECT_ENSURE_REGISTERED(TcpSocketAdvertisedWindowProxy);

class TcpDropRatioErrorModel : public TcpGeneralErrorModel {
public:
  static TypeId GetTypeId();

  TcpDropRatioErrorModel(double dropRatio)
      : TcpGeneralErrorModel(), m_dropRatio(dropRatio) {
    m_prng = CreateObject<UniformRandomVariable>();
  }

protected:
  bool ShouldDrop(const Ipv4Header &ipHeader, const TcpHeader &tcpHeader,
                  uint32_t packetSize) override;

private:
  void DoReset() override {};
  double m_dropRatio;
  Ptr<UniformRandomVariable> m_prng;
};

NS_OBJECT_ENSURE_REGISTERED(TcpDropRatioErrorModel);

TypeId TcpDropRatioErrorModel::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::TcpDropRatioErrorModel").SetParent<TcpGeneralErrorModel>();
  return tid;
}

bool TcpDropRatioErrorModel::ShouldDrop(const Ipv4Header &ipHeader,
                                        const TcpHeader &tcpHeader,
                                        uint32_t packetSize) {
  return m_prng->GetValue() < m_dropRatio;
}

class TcpAdvertisedWindowTest : public TcpGeneralTest {
public:
  TcpAdvertisedWindowTest(const std::string &desc, uint32_t size,
                          uint32_t packets, double lossRatio);

protected:
  void ConfigureEnvironment() override;
  Ptr<TcpSocketMsgBase> CreateReceiverSocket(Ptr<Node> node) override;
  Ptr<ErrorModel> CreateReceiverErrorModel() override;

private:
  void InvalidAwndCb(uint16_t oldAwnd, uint16_t newAwnd);
  uint32_t m_pktSize;
  uint32_t m_pktCount;
  double m_lossRatio;
};

TcpAdvertisedWindowTest::TcpAdvertisedWindowTest(const std::string &desc,
                                                 uint32_t size,
                                                 uint32_t packets,
                                                 double lossRatio)
    : TcpGeneralTest(desc), m_pktSize(size), m_pktCount(packets),
      m_lossRatio(lossRatio) {}

void TcpAdvertisedWindowTest::ConfigureEnvironment() {
  TcpGeneralTest::ConfigureEnvironment();
  SetAppPktCount(m_pktCount);
  SetPropagationDelay(MilliSeconds(50));
  SetTransmitStart(Seconds(2.0));
  SetAppPktSize(m_pktSize);
}

Ptr<TcpSocketMsgBase>
TcpAdvertisedWindowTest::CreateReceiverSocket(Ptr<Node> node) {
  NS_LOG_FUNCTION(this);

  Ptr<TcpSocketMsgBase> sock = CreateSocket(
      node, TcpSocketAdvertisedWindowProxy::GetTypeId(), m_congControlTypeId);
  DynamicCast<TcpSocketAdvertisedWindowProxy>(sock)->SetExpectedSegmentSize(
      500);
  DynamicCast<TcpSocketAdvertisedWindowProxy>(sock)->SetInvalidAwndCb(
      MakeCallback(&TcpAdvertisedWindowTest::InvalidAwndCb, this));

  return sock;
}

Ptr<ErrorModel> TcpAdvertisedWindowTest::CreateReceiverErrorModel() {
  return CreateObject<TcpDropRatioErrorModel>(m_lossRatio);
}

void TcpAdvertisedWindowTest::InvalidAwndCb(uint16_t oldAwnd,
                                            uint16_t newAwnd) {
  NS_TEST_ASSERT_MSG_EQ(oldAwnd, newAwnd,
                        "Old and new AWND calculations do not match.");
}

class TcpAdvWindowOnLossTest : public TcpGeneralTest {
public:
  TcpAdvWindowOnLossTest(const std::string &desc, uint32_t size,
                         uint32_t packets, std::vector<uint32_t> &toDrop);

protected:
  void ConfigureEnvironment() override;
  Ptr<TcpSocketMsgBase> CreateReceiverSocket(Ptr<Node> node) override;
  Ptr<TcpSocketMsgBase> CreateSenderSocket(Ptr<Node> node) override;
  Ptr<ErrorModel> CreateReceiverErrorModel() override;

private:
  void InvalidAwndCb(uint16_t oldAwnd, uint16_t newAwnd);
  uint32_t m_pktSize;
  uint32_t m_pktCount;
  std::vector<uint32_t> m_toDrop;
};

TcpAdvWindowOnLossTest::TcpAdvWindowOnLossTest(const std::string &desc,
                                               uint32_t size, uint32_t packets,
                                               std::vector<uint32_t> &toDrop)
    : TcpGeneralTest(desc), m_pktSize(size), m_pktCount(packets),
      m_toDrop(toDrop) {}

void TcpAdvWindowOnLossTest::ConfigureEnvironment() {
  TcpGeneralTest::ConfigureEnvironment();
  SetAppPktCount(m_pktCount);
  SetPropagationDelay(MilliSeconds(50));
  SetTransmitStart(Seconds(2.0));
  SetAppPktSize(m_pktSize);
}

Ptr<TcpSocketMsgBase>
TcpAdvWindowOnLossTest::CreateReceiverSocket(Ptr<Node> node) {
  NS_LOG_FUNCTION(this);

  Ptr<TcpSocketMsgBase> sock = CreateSocket(
      node, TcpSocketAdvertisedWindowProxy::GetTypeId(), m_congControlTypeId);
  DynamicCast<TcpSocketAdvertisedWindowProxy>(sock)->SetExpectedSegmentSize(
      500);
  DynamicCast<TcpSocketAdvertisedWindowProxy>(sock)->SetInvalidAwndCb(
      MakeCallback(&TcpAdvWindowOnLossTest::InvalidAwndCb, this));

  return sock;
}

Ptr<TcpSocketMsgBase>
TcpAdvWindowOnLossTest::CreateSenderSocket(Ptr<Node> node) {
  auto socket = TcpGeneralTest::CreateSenderSocket(node);
  socket->SetAttribute("InitialCwnd", UintegerValue(10 * m_pktSize));

  return socket;
}

Ptr<ErrorModel> TcpAdvWindowOnLossTest::CreateReceiverErrorModel() {
  Ptr<TcpSeqErrorModel> m_errorModel = CreateObject<TcpSeqErrorModel>();
  for (auto it = m_toDrop.begin(); it != m_toDrop.end(); ++it) {
    m_errorModel->AddSeqToKill(SequenceNumber32(*it));
  }

  return m_errorModel;
}

void TcpAdvWindowOnLossTest::InvalidAwndCb(uint16_t oldAwnd, uint16_t newAwnd) {
  NS_TEST_ASSERT_MSG_EQ(oldAwnd, newAwnd,
                        "Old and new AWND calculations do not match.");
}

class TcpAdvertisedWindowTestSuite : public TestSuite {
public:
  TcpAdvertisedWindowTestSuite()
      : TestSuite("tcp-advertised-window-test", UNIT) {
    AddTestCase(
        new TcpAdvertisedWindowTest(
            "TCP advertised window size, small seg + no loss", 500, 100, 0.0),
        TestCase::QUICK);
    AddTestCase(
        new TcpAdvertisedWindowTest(
            "TCP advertised window size, small seg + loss", 500, 100, 0.1),
        TestCase::QUICK);
    AddTestCase(
        new TcpAdvertisedWindowTest(
            "TCP advertised window size, large seg + no loss", 1000, 100, 0.0),
        TestCase::QUICK);
    AddTestCase(new TcpAdvertisedWindowTest(
                    "TCP advertised window size, large seg + small loss", 1000,
                    100, 0.1),
                TestCase::QUICK);
    AddTestCase(
        new TcpAdvertisedWindowTest(
            "TCP advertised window size, large seg + big loss", 1000, 100, 0.3),
        TestCase::QUICK);
    AddTestCase(
        new TcpAdvertisedWindowTest("TCP advertised window size, complete loss",
                                    1000, 100, 1.0),
        TestCase::QUICK);

    std::vector<uint32_t> toDrop;
    toDrop.push_back(8001);
    toDrop.push_back(9001);
    AddTestCase(new TcpAdvWindowOnLossTest(
        "TCP advertised window size, after FIN loss", 1000, 10, toDrop));
  }
};

static TcpAdvertisedWindowTestSuite g_tcpAdvertisedWindowTestSuite;
