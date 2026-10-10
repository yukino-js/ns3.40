
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/sequence-number.h"
#include "ns3/test.h"
#include "ns3/traced-value.h"
#include "ns3/type-id.h"
#include "ns3/type-name.h"

#include <type_traits>

using namespace ns3;

namespace {

std::string g_Result = "";

template <typename T> void TracedValueCbSink(T oldValue, T newValue) {
  std::cout << ": " << static_cast<int64_t>(oldValue) << " -> "
            << static_cast<int64_t>(newValue) << std::endl;

  if (oldValue != 0) {
    g_Result = "oldValue should be 0";
  }

  if (newValue != 1) {
    g_Result +=
        std::string(g_Result.empty() ? "" : " | ") + "newValue should be 1";
  }
}

template <> void TracedValueCbSink<Time>(Time oldValue, Time newValue) {
  TracedValueCbSink<int64_t>(oldValue.GetInteger(), newValue.GetInteger());
}

template <>
void TracedValueCbSink<SequenceNumber32>(SequenceNumber32 oldValue,
                                         SequenceNumber32 newValue) {
  TracedValueCbSink<int64_t>(oldValue.GetValue(), newValue.GetValue());
}

} // namespace

class TracedValueCallbackTestCase : public TestCase {
public:
  TracedValueCallbackTestCase();

  ~TracedValueCallbackTestCase() override {}

private:
  template <typename T> class CheckTvCb : public Object {
    TracedValue<T> m_value;

  public:
    CheckTvCb() : m_value(0) {}

    static TypeId GetTypeId() {
      static TypeId tid =
          TypeId("CheckTvCb<" + TypeNameGet<T>() + ">")
              .SetParent<Object>()
              .AddTraceSource(
                  "value", "A value being traced.",
                  MakeTraceSourceAccessor(&CheckTvCb<T>::m_value),
                  ("ns3::TracedValueCallback::" + TypeNameGet<T>()));
      return tid;
    }

    template <typename U> void Invoke(U cb) {
      bool ok = TraceConnectWithoutContext("value", MakeCallback(cb));
      std::cout << GetTypeId() << ": "
                << (ok ? "connected " : "failed to connect ")
                << GetTypeId().GetTraceSource(0).callback;

      if (!ok) {
        std::cout << std::endl;

        g_Result = "failed to connect callback";

        return;
      }

      m_value = m_value + static_cast<T>(1);
    }
  };

  template <typename T, typename U> void CheckType() {
    U sink = TracedValueCbSink<T>;
    CreateObject<CheckTvCb<T>>()->Invoke(sink);

    NS_TEST_ASSERT_MSG_EQ(g_Result.empty(), true, g_Result);
    g_Result = "";
  }

  void DoRun() override;
};

TracedValueCallbackTestCase::TracedValueCallbackTestCase()
    : TestCase("Check basic TracedValue callback operation") {}

void TracedValueCallbackTestCase::DoRun() {
  CheckType<bool, TracedValueCallback::Bool>();
  CheckType<int8_t, TracedValueCallback::Int8>();
  CheckType<int16_t, TracedValueCallback::Int16>();
  CheckType<int32_t, TracedValueCallback::Int32>();
  CheckType<int64_t, TracedValueCallback::Int64>();
  CheckType<uint8_t, TracedValueCallback::Uint8>();
  CheckType<uint16_t, TracedValueCallback::Uint16>();
  CheckType<uint32_t, TracedValueCallback::Uint32>();
  CheckType<uint64_t, TracedValueCallback::Uint64>();
  CheckType<double, TracedValueCallback::Double>();
  CheckType<Time, TracedValueCallback::Time>();
  CheckType<SequenceNumber32, TracedValueCallback::SequenceNumber32>();
}

class TracedValueCallbackTestSuite : public TestSuite {
public:
  TracedValueCallbackTestSuite();
};

TracedValueCallbackTestSuite::TracedValueCallbackTestSuite()
    : TestSuite("traced-value-callback", UNIT) {
  AddTestCase(new TracedValueCallbackTestCase, TestCase::QUICK);
}

static TracedValueCallbackTestSuite tracedValueCallbackTestSuite;
