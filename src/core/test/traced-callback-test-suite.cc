
#include "ns3/test.h"
#include "ns3/traced-callback.h"

using namespace ns3;

class BasicTracedCallbackTestCase : public TestCase {
public:
  BasicTracedCallbackTestCase();

  ~BasicTracedCallbackTestCase() override {}

private:
  void DoRun() override;

  void CbOne(uint8_t a, double b);

  CallbackBase m_cbTwo;
  bool m_one;
  bool m_two;
};

BasicTracedCallbackTestCase::BasicTracedCallbackTestCase()
    : TestCase("Check basic TracedCallback operation") {}

void BasicTracedCallbackTestCase::CbOne(uint8_t, double) { m_one = true; }

void BasicTracedCallbackTestCase::DoRun() {
  m_cbTwo = Callback<void, uint8_t, double>(
      [this](uint8_t, double) { m_two = true; });

  TracedCallback<uint8_t, double> trace;

  trace.ConnectWithoutContext(
      MakeCallback(&BasicTracedCallbackTestCase::CbOne, this));
  trace.ConnectWithoutContext(m_cbTwo);
  m_one = false;
  m_two = false;
  trace(1, 2);
  NS_TEST_ASSERT_MSG_EQ(m_one, true, "Callback CbOne not called");
  NS_TEST_ASSERT_MSG_EQ(m_two, true, "Callback CbTwo not called");

  trace.DisconnectWithoutContext(
      MakeCallback(&BasicTracedCallbackTestCase::CbOne, this));
  m_one = false;
  m_two = false;
  trace(1, 2);
  NS_TEST_ASSERT_MSG_EQ(m_one, false, "Callback CbOne unexpectedly called");
  NS_TEST_ASSERT_MSG_EQ(m_two, true, "Callback CbTwo not called");

  trace.DisconnectWithoutContext(m_cbTwo);
  m_one = false;
  m_two = false;
  trace(1, 2);
  NS_TEST_ASSERT_MSG_EQ(m_one, false, "Callback CbOne unexpectedly called");
  NS_TEST_ASSERT_MSG_EQ(m_two, false, "Callback CbTwo unexpectedly called");

  trace.ConnectWithoutContext(
      MakeCallback(&BasicTracedCallbackTestCase::CbOne, this));
  trace.ConnectWithoutContext(m_cbTwo);
  m_one = false;
  m_two = false;
  trace(1, 2);
  NS_TEST_ASSERT_MSG_EQ(m_one, true, "Callback CbOne not called");
  NS_TEST_ASSERT_MSG_EQ(m_two, true, "Callback CbTwo not called");
}

class TracedCallbackTestSuite : public TestSuite {
public:
  TracedCallbackTestSuite();
};

TracedCallbackTestSuite::TracedCallbackTestSuite()
    : TestSuite("traced-callback", UNIT) {
  AddTestCase(new BasicTracedCallbackTestCase, TestCase::QUICK);
}

static TracedCallbackTestSuite g_tracedCallbackTestSuite;
