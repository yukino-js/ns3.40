
#include "ns3/log.h"
#include "ns3/tcp-congestion-ops.h"
#include "ns3/tcp-highspeed.h"
#include "ns3/tcp-socket-base.h"
#include "ns3/test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpHighSpeedTestSuite");

class TcpHighSpeedIncrementTest : public TestCase {
public:
  TcpHighSpeedIncrementTest(uint32_t cWnd, uint32_t segmentSize,
                            const std::string &name);

private:
  void DoRun() override;

  uint32_t m_cWnd;
  uint32_t m_segmentSize;
  Ptr<TcpSocketState> m_state;
};

TcpHighSpeedIncrementTest::TcpHighSpeedIncrementTest(uint32_t cWnd,
                                                     uint32_t segmentSize,
                                                     const std::string &name)
    : TestCase(name), m_cWnd(cWnd), m_segmentSize(segmentSize) {}

void TcpHighSpeedIncrementTest::DoRun() {
  m_state = CreateObject<TcpSocketState>();

  m_state->m_cWnd = m_cWnd;
  m_state->m_segmentSize = m_segmentSize;

  Ptr<TcpHighSpeed> cong = CreateObject<TcpHighSpeed>();

  uint32_t segCwnd = m_cWnd / m_segmentSize;
  uint32_t coeffA = TcpHighSpeed::TableLookupA(segCwnd);

  cong->IncreaseWindow(m_state, (segCwnd / coeffA) + 1);

  NS_TEST_ASSERT_MSG_EQ(m_state->m_cWnd.Get(), m_cWnd + m_segmentSize,
                        "CWnd has not increased");
}

class TcpHighSpeedDecrementTest : public TestCase {
public:
  TcpHighSpeedDecrementTest(uint32_t cWnd, uint32_t segmentSize,
                            const std::string &name);

private:
  void DoRun() override;

  uint32_t m_cWnd;
  uint32_t m_segmentSize;
  Ptr<TcpSocketState> m_state;
};

TcpHighSpeedDecrementTest::TcpHighSpeedDecrementTest(uint32_t cWnd,
                                                     uint32_t segmentSize,
                                                     const std::string &name)
    : TestCase(name), m_cWnd(cWnd), m_segmentSize(segmentSize) {}

void TcpHighSpeedDecrementTest::DoRun() {
  m_state = CreateObject<TcpSocketState>();

  m_state->m_cWnd = m_cWnd;
  m_state->m_segmentSize = m_segmentSize;

  Ptr<TcpHighSpeed> cong = CreateObject<TcpHighSpeed>();

  uint32_t segCwnd = m_cWnd / m_segmentSize;
  double coeffB = 1.0 - TcpHighSpeed::TableLookupB(segCwnd);

  uint32_t ret = cong->GetSsThresh(m_state, m_state->m_cWnd);

  uint32_t ssThHS = std::max(2.0, segCwnd * coeffB);

  NS_TEST_ASSERT_MSG_EQ(ret / m_segmentSize, ssThHS,
                        "HighSpeed decrement fn not used");
}

struct HighSpeedImportantValues {
  unsigned int cwnd;
  unsigned int md;
};

static const HighSpeedImportantValues highSpeedImportantValues[]{
    {
        38,
        128,
    },
    {
        118,
        112,
    },
    {
        221,
        104,
    },
    {
        347,
        98,
    },
    {
        495,
        93,
    },
    {
        663,
        89,
    },
    {
        851,
        86,
    },
    {
        1058,
        83,
    },
    {
        1284,
        81,
    },
    {
        1529,
        78,
    },
    {
        1793,
        76,
    },
    {
        2076,
        74,
    },
    {
        2378,
        72,
    },
    {
        2699,
        71,
    },
    {
        3039,
        69,
    },
    {
        3399,
        68,
    },
    {
        3778,
        66,
    },
    {
        4177,
        65,
    },
    {
        4596,
        64,
    },
    {
        5036,
        62,
    },
    {
        5497,
        61,
    },
    {
        5979,
        60,
    },
    {
        6483,
        59,
    },
    {
        7009,
        58,
    },
    {
        7558,
        57,
    },
    {
        8130,
        56,
    },
    {
        8726,
        55,
    },
    {
        9346,
        54,
    },
    {
        9991,
        53,
    },
    {
        10661,
        52,
    },
    {
        11358,
        52,
    },
    {
        12082,
        51,
    },
    {
        12834,
        50,
    },
    {
        13614,
        49,
    },
    {
        14424,
        48,
    },
    {
        15265,
        48,
    },
    {
        16137,
        47,
    },
    {
        17042,
        46,
    },
    {
        17981,
        45,
    },
    {
        18955,
        45,
    },
    {
        19965,
        44,
    },
    {
        21013,
        43,
    },
    {
        22101,
        43,
    },
    {
        23230,
        42,
    },
    {
        24402,
        41,
    },
    {
        25618,
        41,
    },
    {
        26881,
        40,
    },
    {
        28193,
        39,
    },
    {
        29557,
        39,
    },
    {
        30975,
        38,
    },
    {
        32450,
        38,
    },
    {
        33986,
        37,
    },
    {
        35586,
        36,
    },
    {
        37253,
        36,
    },
    {
        38992,
        35,
    },
    {
        40808,
        35,
    },
    {
        42707,
        34,
    },
    {
        44694,
        33,
    },
    {
        46776,
        33,
    },
    {
        48961,
        32,
    },
    {
        51258,
        32,
    },
    {
        53677,
        31,
    },
    {
        56230,
        30,
    },
    {
        58932,
        30,
    },
    {
        61799,
        29,
    },
    {
        64851,
        28,
    },
    {
        68113,
        28,
    },
    {
        71617,
        27,
    },
    {
        75401,
        26,
    },
    {
        79517,
        26,
    },
    {
        84035,
        25,
    },
    {
        89053,
        24,
    },
};

#define HIGHSPEED_VALUES_N 71

class TcpHighSpeedTestSuite : public TestSuite {
public:
  TcpHighSpeedTestSuite() : TestSuite("tcp-highspeed-test", UNIT) {
    std::stringstream ss;

    for (uint32_t i = 0; i < HIGHSPEED_VALUES_N; ++i) {
      ss << highSpeedImportantValues[i].cwnd;
      AddTestCase(new TcpHighSpeedIncrementTest(
                      highSpeedImportantValues[i].cwnd, 1,
                      "Highspeed increment test on cWnd " + ss.str()),
                  TestCase::QUICK);
      AddTestCase(new TcpHighSpeedIncrementTest(
                      highSpeedImportantValues[i].cwnd * 536, 536,
                      "Highspeed increment test on cWnd " + ss.str()),
                  TestCase::QUICK);
      AddTestCase(new TcpHighSpeedIncrementTest(
                      highSpeedImportantValues[i].cwnd * 1446, 1446,
                      "Highspeed increment test on cWnd " + ss.str()),
                  TestCase::QUICK);
      AddTestCase(new TcpHighSpeedDecrementTest(
                      highSpeedImportantValues[i].cwnd, 1,
                      "Highspeed Decrement test on cWnd " + ss.str()),
                  TestCase::QUICK);
      AddTestCase(new TcpHighSpeedDecrementTest(
                      highSpeedImportantValues[i].cwnd * 536, 536,
                      "Highspeed Decrement test on cWnd " + ss.str()),
                  TestCase::QUICK);
      AddTestCase(new TcpHighSpeedDecrementTest(
                      highSpeedImportantValues[i].cwnd * 1446, 1446,
                      "Highspeed Decrement test on cWnd " + ss.str()),
                  TestCase::QUICK);
      ss.flush();
    }
  }
};

static TcpHighSpeedTestSuite g_tcpHighSpeedTest;
