
#include "ns3/boolean.h"
#include "ns3/callback.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/enum.h"
#include "ns3/integer.h"
#include "ns3/nstime.h"
#include "ns3/object-factory.h"
#include "ns3/object-map.h"
#include "ns3/object-vector.h"
#include "ns3/object.h"
#include "ns3/pointer.h"
#include "ns3/random-variable-stream.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/traced-value.h"
#include "ns3/uinteger.h"

using namespace ns3;

namespace ns3 {

class ValueClassTest {
public:
  ValueClassTest() {}

  typedef void (*TracedValueCallback)(const ValueClassTest oldValue,
                                      const ValueClassTest newValue);
};

bool operator!=(const ValueClassTest &a [[maybe_unused]],
                const ValueClassTest &b [[maybe_unused]]) {
  return true;
}

std::ostream &operator<<(std::ostream &os, ValueClassTest v [[maybe_unused]]) {
  return os;
}

std::istream &operator>>(std::istream &is, ValueClassTest &v [[maybe_unused]]) {
  return is;
}

ATTRIBUTE_HELPER_HEADER(ValueClassTest);
ATTRIBUTE_HELPER_CPP(ValueClassTest);

} // namespace ns3

class Derived : public Object {
public:
  static TypeId GetTypeId() {
    static TypeId tid =
        TypeId("ns3::Derived").AddConstructor<Derived>().SetParent<Object>();
    return tid;
  }

  Derived() {}
};

NS_OBJECT_ENSURE_REGISTERED(Derived);

class AttributeObjectTest : public Object {
public:
  enum Test_e { TEST_A, TEST_B, TEST_C };

  static TypeId GetTypeId() {
    static TypeId tid =
        TypeId("ns3::AttributeObjectTest")
            .AddConstructor<AttributeObjectTest>()
            .SetParent<Object>()
            .HideFromDocumentation()
            .AddAttribute("TestBoolName", "help text", BooleanValue(false),
                          MakeBooleanAccessor(&AttributeObjectTest::m_boolTest),
                          MakeBooleanChecker())
            .AddAttribute("TestBoolA", "help text", BooleanValue(false),
                          MakeBooleanAccessor(&AttributeObjectTest::DoSetTestA,
                                              &AttributeObjectTest::DoGetTestA),
                          MakeBooleanChecker())
            .AddAttribute("TestInt16", "help text", IntegerValue(-2),
                          MakeIntegerAccessor(&AttributeObjectTest::m_int16),
                          MakeIntegerChecker<int16_t>())
            .AddAttribute(
                "TestInt16WithBounds", "help text", IntegerValue(-2),
                MakeIntegerAccessor(&AttributeObjectTest::m_int16WithBounds),
                MakeIntegerChecker<int16_t>(-5, 10))
            .AddAttribute("TestInt16SetGet", "help text", IntegerValue(6),
                          MakeIntegerAccessor(&AttributeObjectTest::DoSetInt16,
                                              &AttributeObjectTest::DoGetInt16),
                          MakeIntegerChecker<int16_t>())
            .AddAttribute("TestUint8", "help text", UintegerValue(1),
                          MakeUintegerAccessor(&AttributeObjectTest::m_uint8),
                          MakeUintegerChecker<uint8_t>())
            .AddAttribute("TestEnum", "help text", EnumValue(TEST_A),
                          MakeEnumAccessor(&AttributeObjectTest::m_enum),
                          MakeEnumChecker(TEST_A, "TestA", TEST_B, "TestB",
                                          TEST_C, "TestC"))
            .AddAttribute("TestEnumSetGet", "help text", EnumValue(TEST_B),
                          MakeEnumAccessor(&AttributeObjectTest::DoSetEnum,
                                           &AttributeObjectTest::DoGetEnum),
                          MakeEnumChecker(TEST_A, "TestA", TEST_B, "TestB",
                                          TEST_C, "TestC"))
            .AddAttribute(
                "TestRandom", "help text",
                StringValue("ns3::ConstantRandomVariable[Constant=1.0]"),
                MakePointerAccessor(&AttributeObjectTest::m_random),
                MakePointerChecker<RandomVariableStream>())
            .AddAttribute("TestFloat", "help text", DoubleValue(-1.1),
                          MakeDoubleAccessor(&AttributeObjectTest::m_float),
                          MakeDoubleChecker<float>())
            .AddAttribute(
                "TestVector1", "help text", ObjectVectorValue(),
                MakeObjectVectorAccessor(&AttributeObjectTest::m_vector1),
                MakeObjectVectorChecker<Derived>())
            .AddAttribute(
                "TestVector2", "help text", ObjectVectorValue(),
                MakeObjectVectorAccessor(&AttributeObjectTest::DoGetVectorN,
                                         &AttributeObjectTest::DoGetVector),
                MakeObjectVectorChecker<Derived>())
            .AddAttribute("TestMap1", "help text", ObjectMapValue(),
                          MakeObjectMapAccessor(&AttributeObjectTest::m_map1),
                          MakeObjectMapChecker<Derived>())
            .AddAttribute(
                "TestUnorderedMap", "help text", ObjectMapValue(),
                MakeObjectMapAccessor(&AttributeObjectTest::m_unorderedMap),
                MakeObjectMapChecker<Derived>())
            .AddAttribute("IntegerTraceSource1", "help text", IntegerValue(-2),
                          MakeIntegerAccessor(&AttributeObjectTest::m_intSrc1),
                          MakeIntegerChecker<int8_t>())
            .AddAttribute(
                "IntegerTraceSource2", "help text", IntegerValue(-2),
                MakeIntegerAccessor(&AttributeObjectTest::DoSetIntSrc,
                                    &AttributeObjectTest::DoGetIntSrc),
                MakeIntegerChecker<int8_t>())
            .AddAttribute("UIntegerTraceSource", "help text", UintegerValue(2),
                          MakeUintegerAccessor(&AttributeObjectTest::m_uintSrc),
                          MakeIntegerChecker<uint8_t>())
            .AddAttribute("DoubleTraceSource", "help text", DoubleValue(2),
                          MakeDoubleAccessor(&AttributeObjectTest::m_doubleSrc),
                          MakeDoubleChecker<double>())
            .AddAttribute("BoolTraceSource", "help text", BooleanValue(false),
                          MakeBooleanAccessor(&AttributeObjectTest::m_boolSrc),
                          MakeBooleanChecker())
            .AddAttribute("EnumTraceSource", "help text", EnumValue(TEST_A),
                          MakeEnumAccessor(&AttributeObjectTest::m_enumSrc),
                          MakeEnumChecker(TEST_A, "TestA"))
            .AddAttribute(
                "ValueClassSource", "help text",
                ValueClassTestValue(ValueClassTest()),
                MakeValueClassTestAccessor(&AttributeObjectTest::m_valueSrc),
                MakeValueClassTestChecker())
            .AddTraceSource(
                "Source1", "help test",
                MakeTraceSourceAccessor(&AttributeObjectTest::m_intSrc1),
                "ns3::TracedValueCallback::Int8")
            .AddTraceSource("Source2", "help text",
                            MakeTraceSourceAccessor(&AttributeObjectTest::m_cb),
                            "ns3::AttributeObjectTest::NumericTracedCallback")
            .AddTraceSource(
                "ValueSource", "help text",
                MakeTraceSourceAccessor(&AttributeObjectTest::m_valueSrc),
                "ns3::ValueClassTest::TracedValueCallback")
            .AddAttribute("Pointer", "help text", PointerValue(),
                          MakePointerAccessor(&AttributeObjectTest::m_ptr),
                          MakePointerChecker<Derived>())
            .AddAttribute(
                "PointerInitialized", "help text", StringValue("ns3::Derived"),
                MakePointerAccessor(&AttributeObjectTest::m_ptrInitialized),
                MakePointerChecker<Derived>())
            .AddAttribute(
                "PointerInitialized2", "help text",
                StringValue("ns3::Derived[]"),
                MakePointerAccessor(&AttributeObjectTest::m_ptrInitialized2),
                MakePointerChecker<Derived>())
            .AddAttribute("Callback", "help text", CallbackValue(),
                          MakeCallbackAccessor(&AttributeObjectTest::m_cbValue),
                          MakeCallbackChecker())
            .AddAttribute(
                "TestTimeWithBounds", "help text", TimeValue(Seconds(-2)),
                MakeTimeAccessor(&AttributeObjectTest::m_timeWithBounds),
                MakeTimeChecker(Seconds(-5), Seconds(10)))
            .AddAttribute(
                "TestDeprecated", "help text", BooleanValue(false),
                MakeBooleanAccessor(&AttributeObjectTest::m_boolTestDeprecated),
                MakeBooleanChecker(), TypeId::DEPRECATED,
                "DEPRECATED test working.");

    return tid;
  }

  AttributeObjectTest() {}

  ~AttributeObjectTest() override {}

  void AddToVector1() { m_vector1.push_back(CreateObject<Derived>()); }

  void AddToVector2() { m_vector2.push_back(CreateObject<Derived>()); }

  void AddToMap1(uint32_t i) {
    m_map1.insert(
        std::pair<uint32_t, Ptr<Derived>>(i, CreateObject<Derived>()));
  }

  void AddToUnorderedMap(uint64_t i) {
    m_unorderedMap.insert({i, CreateObject<Derived>()});
  }

  void RemoveFromUnorderedMap(uint64_t i) { m_unorderedMap.erase(i); }

  void InvokeCb(double a, int b, float c) { m_cb(a, b, c); }

  void InvokeCbValue(int8_t a) {
    if (!m_cbValue.IsNull()) {
      m_cbValue(a);
    }
  }

private:
  void DoSetTestA(bool v) { m_boolTestA = v; }

  bool DoGetTestA() const { return m_boolTestA; }

  int16_t DoGetInt16() const { return m_int16SetGet; }

  void DoSetInt16(int16_t v) { m_int16SetGet = v; }

  std::size_t DoGetVectorN() const { return m_vector2.size(); }

  Ptr<Derived> DoGetVector(std::size_t i) const { return m_vector2[i]; }

  bool DoSetIntSrc(int8_t v) {
    m_intSrc2 = v;
    return true;
  }

  int8_t DoGetIntSrc() const { return m_intSrc2; }

  bool DoSetEnum(Test_e v) {
    m_enumSetGet = v;
    return true;
  }

  Test_e DoGetEnum() const { return m_enumSetGet; }

  bool m_boolTestA;
  bool m_boolTest;
  bool m_boolTestDeprecated;
  int16_t m_int16;
  int16_t m_int16WithBounds;
  int16_t m_int16SetGet;
  uint8_t m_uint8;
  float m_float;
  Test_e m_enum;
  Test_e m_enumSetGet;
  Ptr<RandomVariableStream> m_random;
  std::vector<Ptr<Derived>> m_vector1;
  std::vector<Ptr<Derived>> m_vector2;
  std::map<uint32_t, Ptr<Derived>> m_map1;
  std::unordered_map<uint64_t, Ptr<Derived>> m_unorderedMap;
  Callback<void, int8_t> m_cbValue;
  TracedValue<int8_t> m_intSrc1;
  TracedValue<int8_t> m_intSrc2;

  typedef void (*NumericTracedCallback)(double, int, float);
  TracedCallback<double, int, float> m_cb;
  TracedValue<ValueClassTest> m_valueSrc;
  Ptr<Derived> m_ptr;
  Ptr<Derived> m_ptrInitialized;
  Ptr<Derived> m_ptrInitialized2;
  TracedValue<uint8_t> m_uintSrc;
  TracedValue<Test_e> m_enumSrc;
  TracedValue<double> m_doubleSrc;
  TracedValue<bool> m_boolSrc;
  Time m_timeWithBounds;
};

NS_OBJECT_ENSURE_REGISTERED(AttributeObjectTest);

template <typename T> class AttributeTestCase : public TestCase {
public:
  AttributeTestCase(std::string description);
  ~AttributeTestCase() override;

private:
  void DoRun() override;
  bool CheckGetCodePaths(Ptr<Object> p, std::string attributeName,
                         std::string expectedString, T expectedValue);
};

template <typename T>
AttributeTestCase<T>::AttributeTestCase(std::string description)
    : TestCase(description) {}

template <typename T> AttributeTestCase<T>::~AttributeTestCase() {}

template <typename T>
bool AttributeTestCase<T>::CheckGetCodePaths(Ptr<Object> p,
                                             std::string attributeName,
                                             std::string expectedString,
                                             T expectedValue) {
  StringValue stringValue;
  T actualValue;

  bool ok1 = p->GetAttributeFailSafe(attributeName, stringValue);
  bool ok2 = stringValue.Get() == expectedString;

  bool ok3 = p->GetAttributeFailSafe(attributeName, actualValue);
  bool ok4 = expectedValue.Get() == actualValue.Get();

  return ok1 && ok2 && ok3 && ok4;
}

template <> void AttributeTestCase<BooleanValue>::DoRun() {
  Ptr<AttributeObjectTest> p;
  bool ok;

  p = CreateObject<AttributeObjectTest>();
  NS_TEST_ASSERT_MSG_NE(p, nullptr, "Unable to CreateObject");

  Config::SetDefault("ns3::AttributeObjectTest::TestBoolName",
                     StringValue("true"));
  p = CreateObject<AttributeObjectTest>();
  NS_TEST_ASSERT_MSG_NE(p, nullptr, "Unable to CreateObject");

  ok = CheckGetCodePaths(p, "TestBoolName", "true", BooleanValue(true));
  NS_TEST_ASSERT_MSG_EQ(ok, true,
                        "Attribute not set properly by default value");

  std::string expected(
      "Attribute 'TestDeprecated' is deprecated: DEPRECATED test working.\n");
  std::stringstream buffer;
  std::streambuf *oldBuffer = std::cerr.rdbuf(buffer.rdbuf());
  Config::SetDefault("ns3::AttributeObjectTest::TestDeprecated",
                     BooleanValue(true));

  NS_TEST_ASSERT_MSG_EQ(buffer.str(), expected,
                        "Deprecated attribute not working");
  std::cerr.rdbuf(oldBuffer);

  Config::SetDefaultFailSafe("ns3::AttributeObjectTest::TestBoolName",
                             StringValue("false"));

  p = CreateObject<AttributeObjectTest>();
  NS_TEST_ASSERT_MSG_NE(p, nullptr, "Unable to CreateObject");

  ok = CheckGetCodePaths(p, "TestBoolName", "false", BooleanValue(false));
  NS_TEST_ASSERT_MSG_EQ(ok, true, "Attribute not et properly by default value");

  ok = p->SetAttributeFailSafe("TestBoolName", StringValue("true"));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true, "Could not SetAttributeFailSafe() \"TestBoolName\" to true");

  ok = CheckGetCodePaths(p, "TestBoolName", "true", BooleanValue(true));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true,
      "Attribute not set properly by SetAttributeFailSafe() via StringValue");

  ok = p->SetAttributeFailSafe("TestBoolName", StringValue("false"));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true, "Could not SetAttributeFailSafe() \"TestBoolName\" to false");

  ok = CheckGetCodePaths(p, "TestBoolName", "false", BooleanValue(false));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true,
      "Attribute not set properly by SetAttributeFailSafe() via StringValue");

  p = CreateObject<AttributeObjectTest>();
  NS_TEST_ASSERT_MSG_NE(p, nullptr, "Unable to CreateObject");

  ok = p->SetAttributeFailSafe("TestBoolA", StringValue("true"));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true, "Could not SetAttributeFailSafe() a boolean value to true");

  ok = CheckGetCodePaths(p, "TestBoolA", "true", BooleanValue(true));
  NS_TEST_ASSERT_MSG_EQ(ok, true,
                        "Attribute not set properly by SetAttributeFailSafe() "
                        "(getter/setter) via StringValue");

  ok = p->SetAttributeFailSafe("TestBoolA", StringValue("false"));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true, "Could not SetAttributeFailSafe() a boolean value to false");

  ok = CheckGetCodePaths(p, "TestBoolA", "false", BooleanValue(false));
  NS_TEST_ASSERT_MSG_EQ(ok, true,
                        "Attribute not set properly by SetAttributeFailSafe() "
                        "(getter/setter) via StringValue");
}

template <> void AttributeTestCase<IntegerValue>::DoRun() {
  Ptr<AttributeObjectTest> p;
  bool ok;

  p = CreateObject<AttributeObjectTest>();
  NS_TEST_ASSERT_MSG_NE(p, nullptr, "Unable to CreateObject");

  ok = CheckGetCodePaths(p, "TestInt16", "-2", IntegerValue(-2));
  NS_TEST_ASSERT_MSG_EQ(ok, true,
                        "Attribute not set properly by default value");

  ok = p->SetAttributeFailSafe("TestInt16", StringValue("-5"));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true, "Could not SetAttributeFailSafe() via StringValue to -5");

  ok = CheckGetCodePaths(p, "TestInt16", "-5", IntegerValue(-5));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true,
      "Attribute not set properly by SetAttributeFailSafe() via StringValue");

  ok = p->SetAttributeFailSafe("TestInt16", StringValue("+2"));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true, "Could not SetAttributeFailSafe() via StringValue to +2");

  ok = CheckGetCodePaths(p, "TestInt16", "2", IntegerValue(2));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true,
      "Attribute not set properly by SetAttributeFailSafe() via StringValue");

  ok = p->SetAttributeFailSafe("TestInt16", StringValue("-32768"));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true, "Could not SetAttributeFailSafe() via StringValue to -32768");

  ok = CheckGetCodePaths(p, "TestInt16", "-32768", IntegerValue(-32768));
  NS_TEST_ASSERT_MSG_EQ(ok, true,
                        "Attribute not set properly by SetAttributeFailSafe() "
                        "(most negative) via StringValue");

  ok = p->SetAttributeFailSafe("TestInt16", StringValue("-32769"));
  NS_TEST_ASSERT_MSG_EQ(
      ok, false,
      "Unexpectedly could SetAttributeFailSafe() via StringValue to -32769");

  ok = CheckGetCodePaths(p, "TestInt16", "-32768", IntegerValue(-32768));
  NS_TEST_ASSERT_MSG_EQ(ok, true,
                        "Error in SetAttributeFailSafe() but value changes");

  ok = p->SetAttributeFailSafe("TestInt16", StringValue("32767"));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true, "Could not SetAttributeFailSafe() via StringValue to 32767");

  ok = CheckGetCodePaths(p, "TestInt16", "32767", IntegerValue(32767));
  NS_TEST_ASSERT_MSG_EQ(ok, true,
                        "Attribute not set properly by SetAttributeFailSafe() "
                        "(most positive) via StringValue");

  ok = p->SetAttributeFailSafe("TestInt16", StringValue("32768"));
  NS_TEST_ASSERT_MSG_EQ(
      ok, false,
      "Unexpectedly could SetAttributeFailSafe() via StringValue to 32768");

  ok = CheckGetCodePaths(p, "TestInt16", "32767", IntegerValue(32767));
  NS_TEST_ASSERT_MSG_EQ(ok, true,
                        "Error in SetAttributeFailSafe() but value changes");

  ok = p->SetAttributeFailSafe("TestInt16WithBounds", IntegerValue(10));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true, "Could not SetAttributeFailSafe() via IntegerValue to 10");

  ok = CheckGetCodePaths(p, "TestInt16WithBounds", "10", IntegerValue(10));
  NS_TEST_ASSERT_MSG_EQ(ok, true,
                        "Attribute not set properly by SetAttributeFailSafe() "
                        "(positive limit) via StringValue");

  ok = p->SetAttributeFailSafe("TestInt16WithBounds", IntegerValue(11));
  NS_TEST_ASSERT_MSG_EQ(
      ok, false,
      "Unexpectedly could SetAttributeFailSafe() via IntegerValue to 11");

  ok = CheckGetCodePaths(p, "TestInt16WithBounds", "10", IntegerValue(10));
  NS_TEST_ASSERT_MSG_EQ(ok, true,
                        "Error in SetAttributeFailSafe() but value changes");

  ok = p->SetAttributeFailSafe("TestInt16WithBounds", IntegerValue(-5));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true, "Could not SetAttributeFailSafe() via IntegerValue to -5");

  ok = CheckGetCodePaths(p, "TestInt16WithBounds", "-5", IntegerValue(-5));
  NS_TEST_ASSERT_MSG_EQ(ok, true,
                        "Attribute not set properly by SetAttributeFailSafe() "
                        "(negative limit) via StringValue");

  ok = p->SetAttributeFailSafe("TestInt16WithBounds", IntegerValue(-6));
  NS_TEST_ASSERT_MSG_EQ(
      ok, false,
      "Unexpectedly could SetAttributeFailSafe() via IntegerValue to -6");

  ok = CheckGetCodePaths(p, "TestInt16WithBounds", "-5", IntegerValue(-5));
  NS_TEST_ASSERT_MSG_EQ(ok, true,
                        "Error in SetAttributeFailSafe() but value changes");
}

template <> void AttributeTestCase<UintegerValue>::DoRun() {
  Ptr<AttributeObjectTest> p;
  bool ok;

  p = CreateObject<AttributeObjectTest>();
  NS_TEST_ASSERT_MSG_NE(p, nullptr, "Unable to CreateObject");

  ok = CheckGetCodePaths(p, "TestUint8", "1", UintegerValue(1));
  NS_TEST_ASSERT_MSG_EQ(ok, true,
                        "Attribute not set properly by default value");

  ok = p->SetAttributeFailSafe("TestUint8", UintegerValue(0));
  NS_TEST_ASSERT_MSG_EQ(ok, true, "Could not SetAttributeFailSafe() to 0");

  ok = CheckGetCodePaths(p, "TestUint8", "0", UintegerValue(0));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true,
      "Attribute not set properly by SetAttributeFailSafe() via StringValue");

  ok = p->SetAttributeFailSafe("TestUint8", UintegerValue(255));
  NS_TEST_ASSERT_MSG_EQ(ok, true, "Could not SetAttributeFailSafe() to 255");

  ok = CheckGetCodePaths(p, "TestUint8", "255", UintegerValue(255));
  NS_TEST_ASSERT_MSG_EQ(ok, true,
                        "Attribute not set properly by SetAttributeFailSafe() "
                        "(positive limit) via UintegerValue");

  ok = p->SetAttributeFailSafe("TestUint8", UintegerValue(256));
  NS_TEST_ASSERT_MSG_EQ(ok, false,
                        "Unexpectedly could SetAttributeFailSafe() to 256");

  ok = CheckGetCodePaths(p, "TestUint8", "255", UintegerValue(255));
  NS_TEST_ASSERT_MSG_EQ(ok, true,
                        "Error in SetAttributeFailSafe() but value changes");

  ok = p->SetAttributeFailSafe("TestUint8", StringValue("255"));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true, "Could not SetAttributeFailSafe() via StringValue to 255");

  ok = CheckGetCodePaths(p, "TestUint8", "255", UintegerValue(255));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true,
      "Attribute not set properly by SetAttributeFailSafe() via StringValue");

  ok = p->SetAttributeFailSafe("TestUint8", StringValue("256"));
  NS_TEST_ASSERT_MSG_EQ(
      ok, false,
      "Unexpectedly could SetAttributeFailSafe() via StringValue to 256");

  ok = CheckGetCodePaths(p, "TestUint8", "255", UintegerValue(255));
  NS_TEST_ASSERT_MSG_EQ(ok, true,
                        "Error in SetAttributeFailSafe() but value changes");

  ok = p->SetAttributeFailSafe("TestUint8", StringValue("-1"));
  NS_TEST_ASSERT_MSG_EQ(
      ok, false,
      "Unexpectedly could SetAttributeFailSafe() via StringValue to -1");
}

template <> void AttributeTestCase<DoubleValue>::DoRun() {
  Ptr<AttributeObjectTest> p;
  bool ok;

  p = CreateObject<AttributeObjectTest>();
  NS_TEST_ASSERT_MSG_NE(p, nullptr, "Unable to CreateObject");

  ok = CheckGetCodePaths(p, "TestFloat", "-1.1", DoubleValue(-1.1F));
  NS_TEST_ASSERT_MSG_EQ(ok, true,
                        "Attribute not set properly by default value");

  ok = p->SetAttributeFailSafe("TestFloat", DoubleValue(2.3F));
  NS_TEST_ASSERT_MSG_EQ(ok, true, "Could not SetAttributeFailSafe() to 2.3");

  ok = CheckGetCodePaths(p, "TestFloat", "2.3", DoubleValue(2.3F));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true,
      "Attribute not set properly by SetAttributeFailSafe() via DoubleValue");
}

template <> void AttributeTestCase<EnumValue>::DoRun() {
  Ptr<AttributeObjectTest> p;
  bool ok;

  p = CreateObject<AttributeObjectTest>();
  NS_TEST_ASSERT_MSG_NE(p, nullptr, "Unable to CreateObject");

  ok = CheckGetCodePaths(p, "TestEnum", "TestA",
                         EnumValue(AttributeObjectTest::TEST_A));
  NS_TEST_ASSERT_MSG_EQ(ok, true,
                        "Attribute not set properly by default value");

  ok = p->SetAttributeFailSafe("TestEnum",
                               EnumValue(AttributeObjectTest::TEST_C));
  NS_TEST_ASSERT_MSG_EQ(ok, true, "Could not SetAttributeFailSafe() to TEST_C");

  ok = CheckGetCodePaths(p, "TestEnum", "TestC",
                         EnumValue(AttributeObjectTest::TEST_C));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true,
      "Attribute not set properly by SetAttributeFailSafe() via EnumValue");

  ok = CheckGetCodePaths(p, "TestEnumSetGet", "TestB",
                         EnumValue(AttributeObjectTest::TEST_B));
  NS_TEST_ASSERT_MSG_EQ(ok, true,
                        "Attribute not set properly by default value");

  ok = p->SetAttributeFailSafe("TestEnumSetGet",
                               EnumValue(AttributeObjectTest::TEST_C));
  NS_TEST_ASSERT_MSG_EQ(ok, true, "Could not SetAttributeFailSafe() to TEST_C");

  ok = CheckGetCodePaths(p, "TestEnumSetGet", "TestC",
                         EnumValue(AttributeObjectTest::TEST_C));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true,
      "Attribute not set properly by SetAttributeFailSafe() via EnumValue");

  ok = p->SetAttributeFailSafe("TestEnum", StringValue("TestB"));
  NS_TEST_ASSERT_MSG_EQ(ok, true, "Could not SetAttributeFailSafe() to TEST_B");

  ok = CheckGetCodePaths(p, "TestEnum", "TestB",
                         EnumValue(AttributeObjectTest::TEST_B));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true,
      "Attribute not set properly by SetAttributeFailSafe() via StringValue");

  ok = CheckGetCodePaths(p, "TestEnum", "TestB",
                         EnumValue(AttributeObjectTest::TEST_B));
  NS_TEST_ASSERT_MSG_EQ(ok, true,
                        "Error in SetAttributeFailSafe() but value changes");

  ok = p->SetAttributeFailSafe("TestEnum", EnumValue(5));
  NS_TEST_ASSERT_MSG_EQ(ok, false,
                        "Unexpectedly could SetAttributeFailSafe() to 5");

  ok = CheckGetCodePaths(p, "TestEnum", "TestB",
                         EnumValue(AttributeObjectTest::TEST_B));
  NS_TEST_ASSERT_MSG_EQ(ok, true,
                        "Error in SetAttributeFailSafe() but value changes");
}

template <> void AttributeTestCase<TimeValue>::DoRun() {
  Ptr<AttributeObjectTest> p;
  bool ok;

  p = CreateObject<AttributeObjectTest>();
  NS_TEST_ASSERT_MSG_NE(p, nullptr, "Unable to CreateObject");

  Time::SetResolution(Time::NS);

  ok = p->SetAttributeFailSafe("TestTimeWithBounds", TimeValue(Seconds(5)));
  NS_TEST_ASSERT_MSG_EQ(ok, true,
                        "Could not SetAttributeFailSafe() via TimeValue to 5s");

  ok = CheckGetCodePaths(p, "TestTimeWithBounds", "+5e+09ns",
                         TimeValue(Seconds(5)));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true,
      "Attribute not set properly by SetAttributeFailSafe(5s) via TimeValue");

  ok = p->SetAttributeFailSafe("TestTimeWithBounds", StringValue("3s"));
  NS_TEST_ASSERT_MSG_EQ(ok, true,
                        "Could not SetAttributeFailSafe() via TimeValue to 3s");

  ok = CheckGetCodePaths(p, "TestTimeWithBounds", "+3e+09ns",
                         TimeValue(Seconds(3)));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true,
      "Attribute not set properly by SetAttributeFailSafe(3s) via StringValue");

  ok = p->SetAttributeFailSafe("TestTimeWithBounds", TimeValue(Seconds(10)));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true, "Could not SetAttributeFailSafe() via TimeValue to 10s");

  ok = CheckGetCodePaths(p, "TestTimeWithBounds", "+1e+10ns",
                         TimeValue(Seconds(10)));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true,
      "Attribute not set properly by SetAttributeFailSafe(10s [positive "
      "limit]) via StringValue");

  ok = p->SetAttributeFailSafe("TestTimeWithBounds", TimeValue(Seconds(11)));
  NS_TEST_ASSERT_MSG_EQ(
      ok, false,
      "Unexpectedly could SetAttributeFailSafe() via TimeValue to 11s [greater "
      "than positive limit]");

  ok = CheckGetCodePaths(p, "TestTimeWithBounds", "+1e+10ns",
                         TimeValue(Seconds(10)));
  NS_TEST_ASSERT_MSG_EQ(ok, true,
                        "Error in SetAttributeFailSafe() but value changes");

  ok = p->SetAttributeFailSafe("TestTimeWithBounds", TimeValue(Seconds(-5)));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true, "Could not SetAttributeFailSafe() via TimeValue to -5s");

  ok = CheckGetCodePaths(p, "TestTimeWithBounds", "-5e+09ns",
                         TimeValue(Seconds(-5)));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true,
      "Attribute not set properly by SetAttributeFailSafe(-5s [negative "
      "limit]) via StringValue");

  ok = p->SetAttributeFailSafe("TestTimeWithBounds", TimeValue(Seconds(-6)));
  NS_TEST_ASSERT_MSG_EQ(
      ok, false,
      "Unexpectedly could SetAttributeFailSafe() via TimeValue to -6s");

  ok = CheckGetCodePaths(p, "TestTimeWithBounds", "-5e+09ns",
                         TimeValue(Seconds(-5)));
  NS_TEST_ASSERT_MSG_EQ(ok, true,
                        "Error in SetAttributeFailSafe() but value changes");
}

class RandomVariableStreamAttributeTestCase : public TestCase {
public:
  RandomVariableStreamAttributeTestCase(std::string description);

  ~RandomVariableStreamAttributeTestCase() override {}

  void InvokeCbValue(int8_t a) {
    if (!m_cbValue.IsNull()) {
      m_cbValue(a);
    }
  }

private:
  void DoRun() override;

  Callback<void, int8_t> m_cbValue;

  void NotifyCallbackValue(int8_t a) { m_gotCbValue = a; }

  int16_t m_gotCbValue;
};

RandomVariableStreamAttributeTestCase::RandomVariableStreamAttributeTestCase(
    std::string description)
    : TestCase(description) {}

void RandomVariableStreamAttributeTestCase::DoRun() {
  Ptr<AttributeObjectTest> p;
  bool ok;

  p = CreateObject<AttributeObjectTest>();
  NS_TEST_ASSERT_MSG_NE(p, nullptr, "Unable to CreateObject");

  ok = p->SetAttributeFailSafe(
      "TestRandom", StringValue("ns3::UniformRandomVariable[Min=0.|Max=1.]"));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true, "Could not SetAttributeFailSafe() a UniformRandomVariable");

  ok = p->SetAttributeFailSafe(
      "TestRandom", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true, "Could not SetAttributeFailSafe() a ConstantRandomVariable");
}

class ObjectVectorAttributeTestCase : public TestCase {
public:
  ObjectVectorAttributeTestCase(std::string description);

  ~ObjectVectorAttributeTestCase() override {}

private:
  void DoRun() override;
};

ObjectVectorAttributeTestCase::ObjectVectorAttributeTestCase(
    std::string description)
    : TestCase(description) {}

void ObjectVectorAttributeTestCase::DoRun() {
  Ptr<AttributeObjectTest> p;
  ObjectVectorValue vector;

  p = CreateObject<AttributeObjectTest>();
  NS_TEST_ASSERT_MSG_NE(p, nullptr, "Unable to CreateObject");

  p->GetAttribute("TestVector1", vector);
  NS_TEST_ASSERT_MSG_EQ(
      vector.GetN(), 0,
      "Initial count of ObjectVectorValue \"TestVector1\" should be zero");

  p->AddToVector1();
  NS_TEST_ASSERT_MSG_EQ(vector.GetN(), 0,
                        "Initial count of ObjectVectorValue \"TestVector1\" "
                        "should still be zero");

  p->GetAttribute("TestVector1", vector);
  NS_TEST_ASSERT_MSG_EQ(
      vector.GetN(), 1,
      "ObjectVectorValue \"TestVector1\" should be incremented");

  Ptr<Object> a = vector.Get(0);
  NS_TEST_ASSERT_MSG_NE(a, nullptr,
                        "Ptr<Object> from VectorValue \"TestVector1\" is zero");

  p->AddToVector1();
  NS_TEST_ASSERT_MSG_EQ(
      vector.GetN(), 1,
      "Count of ObjectVectorValue \"TestVector1\" should still be one");

  p->GetAttribute("TestVector1", vector);
  NS_TEST_ASSERT_MSG_EQ(
      vector.GetN(), 2,
      "ObjectVectorValue \"TestVector1\" should be incremented");
}

class ObjectMapAttributeTestCase : public TestCase {
public:
  ObjectMapAttributeTestCase(std::string description);

  ~ObjectMapAttributeTestCase() override {}

private:
  void DoRun() override;
};

ObjectMapAttributeTestCase::ObjectMapAttributeTestCase(std::string description)
    : TestCase(description) {}

void ObjectMapAttributeTestCase::DoRun() {
  Ptr<AttributeObjectTest> p;
  ObjectMapValue map;

  p = CreateObject<AttributeObjectTest>();
  NS_TEST_ASSERT_MSG_NE(p, nullptr, "Unable to CreateObject");

  p->GetAttribute("TestMap1", map);
  NS_TEST_ASSERT_MSG_EQ(
      map.GetN(), 0,
      "Initial count of ObjectVectorValue \"TestMap1\" should be zero");

  p->AddToMap1(1);
  NS_TEST_ASSERT_MSG_EQ(
      map.GetN(), 0,
      "Initial count of ObjectVectorValue \"TestMap1\" should still be zero");

  p->GetAttribute("TestMap1", map);
  NS_TEST_ASSERT_MSG_EQ(map.GetN(), 1,
                        "ObjectVectorValue \"TestMap1\" should be incremented");

  Ptr<Object> a = map.Get(1);
  NS_TEST_ASSERT_MSG_NE(a, nullptr,
                        "Ptr<Object> from VectorValue \"TestMap1\" is zero");

  p->AddToMap1(2);
  NS_TEST_ASSERT_MSG_EQ(
      map.GetN(), 1,
      "Count of ObjectVectorValue \"TestMap1\" should still be one");

  p->GetAttribute("TestMap1", map);
  NS_TEST_ASSERT_MSG_EQ(map.GetN(), 2,
                        "ObjectVectorValue \"TestMap1\" should be incremented");

  ObjectMapValue unorderedMap;
  p->AddToUnorderedMap(4);
  p->AddToUnorderedMap(2);
  p->AddToUnorderedMap(1);
  p->AddToUnorderedMap(3);
  p->RemoveFromUnorderedMap(2);
  p->GetAttribute("TestUnorderedMap", unorderedMap);
  NS_TEST_ASSERT_MSG_EQ(
      unorderedMap.GetN(), 3,
      "ObjectMapValue \"TestUnorderedMap\" should have three values");
  Ptr<Object> o1 = unorderedMap.Get(1);
  NS_TEST_ASSERT_MSG_NE(
      o1, nullptr,
      "ObjectMapValue \"TestUnorderedMap\" should have value with key 1");
  Ptr<Object> o2 = unorderedMap.Get(2);
  NS_TEST_ASSERT_MSG_EQ(
      o2, nullptr,
      "ObjectMapValue \"TestUnorderedMap\" should not have value with key 2");
  auto it = unorderedMap.Begin();
  NS_TEST_ASSERT_MSG_EQ(
      it->first, 1,
      "ObjectMapValue \"TestUnorderedMap\" should have a value with key 1");
  it++;
  NS_TEST_ASSERT_MSG_EQ(
      it->first, 3,
      "ObjectMapValue \"TestUnorderedMap\" should have a value with key 3");
  it++;
  NS_TEST_ASSERT_MSG_EQ(
      it->first, 4,
      "ObjectMapValue \"TestUnorderedMap\" should have a value with key 4");
}

class IntegerTraceSourceAttributeTestCase : public TestCase {
public:
  IntegerTraceSourceAttributeTestCase(std::string description);

  ~IntegerTraceSourceAttributeTestCase() override {}

private:
  void DoRun() override;
};

IntegerTraceSourceAttributeTestCase::IntegerTraceSourceAttributeTestCase(
    std::string description)
    : TestCase(description) {}

void IntegerTraceSourceAttributeTestCase::DoRun() {
  Ptr<AttributeObjectTest> p;
  IntegerValue iv;
  bool ok;

  p = CreateObject<AttributeObjectTest>();
  NS_TEST_ASSERT_MSG_NE(p, nullptr, "Unable to CreateObject");

  p->GetAttribute("IntegerTraceSource1", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), -2,
                        "Attribute not set properly by default value");

  ok = p->SetAttributeFailSafe("IntegerTraceSource1", IntegerValue(5));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true, "Could not SetAttributeFailSafe() via IntegerValue to 5");

  p->GetAttribute("IntegerTraceSource1", iv);
  NS_TEST_ASSERT_MSG_EQ(
      iv.Get(), 5,
      "Attribute not set properly by SetAttributeFailSafe() via IntegerValue");

  ok = p->SetAttributeFailSafe("IntegerTraceSource1", IntegerValue(127));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true, "Could not SetAttributeFailSafe() via IntegerValue to 127");

  ok = p->SetAttributeFailSafe("IntegerTraceSource1", IntegerValue(128));
  NS_TEST_ASSERT_MSG_EQ(
      ok, false,
      "Unexpectedly could SetAttributeFailSafe() via IntegerValue to 128");

  ok = p->SetAttributeFailSafe("IntegerTraceSource1", IntegerValue(-128));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true, "Could not SetAttributeFailSafe() via IntegerValue to -128");

  ok = p->SetAttributeFailSafe("IntegerTraceSource1", IntegerValue(-129));
  NS_TEST_ASSERT_MSG_EQ(
      ok, false,
      "Unexpectedly could SetAttributeFailSafe() via IntegerValue to -129");

  p->GetAttribute("IntegerTraceSource2", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), -2,
                        "Attribute not set properly by default value");

  ok = p->SetAttributeFailSafe("IntegerTraceSource2", IntegerValue(5));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true, "Could not SetAttributeFailSafe() via IntegerValue to 5");

  p->GetAttribute("IntegerTraceSource2", iv);
  NS_TEST_ASSERT_MSG_EQ(
      iv.Get(), 5,
      "Attribute not set properly by SetAttributeFailSafe() via IntegerValue");

  ok = p->SetAttributeFailSafe("IntegerTraceSource2", IntegerValue(127));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true, "Could not SetAttributeFailSafe() via IntegerValue to 127");

  ok = p->SetAttributeFailSafe("IntegerTraceSource2", IntegerValue(128));
  NS_TEST_ASSERT_MSG_EQ(
      ok, false,
      "Unexpectedly could SetAttributeFailSafe() via IntegerValue to 128");

  ok = p->SetAttributeFailSafe("IntegerTraceSource2", IntegerValue(-128));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true, "Could not SetAttributeFailSafe() via IntegerValue to -128");

  ok = p->SetAttributeFailSafe("IntegerTraceSource2", IntegerValue(-129));
  NS_TEST_ASSERT_MSG_EQ(
      ok, false,
      "Unexpectedly could SetAttributeFailSafe() via IntegerValue to -129");
}

class IntegerTraceSourceTestCase : public TestCase {
public:
  IntegerTraceSourceTestCase(std::string description);

  ~IntegerTraceSourceTestCase() override {}

private:
  void DoRun() override;

  void NotifySource1(int8_t old [[maybe_unused]], int8_t n) { m_got1 = n; }

  int64_t m_got1;
};

IntegerTraceSourceTestCase::IntegerTraceSourceTestCase(std::string description)
    : TestCase(description) {}

void IntegerTraceSourceTestCase::DoRun() {
  Ptr<AttributeObjectTest> p;
  bool ok;

  p = CreateObject<AttributeObjectTest>();
  NS_TEST_ASSERT_MSG_NE(p, nullptr, "Unable to CreateObject");

  m_got1 = 1234;

  ok = p->SetAttributeFailSafe("IntegerTraceSource1", IntegerValue(-1));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true, "Could not SetAttributeFailSafe() via IntegerValue to -1");

  ok = p->TraceConnectWithoutContext(
      "Source1",
      MakeCallback(&IntegerTraceSourceTestCase::NotifySource1, this));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true,
      "Could not TraceConnectWithoutContext() \"Source1\" to NodifySource1()");

  ok = p->SetAttributeFailSafe("IntegerTraceSource1", IntegerValue(0));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true, "Could not SetAttributeFailSafe() via IntegerValue to 0");

  NS_TEST_ASSERT_MSG_EQ(
      m_got1, 0,
      "Hitting a TracedValue does not cause trace callback to be called");

  ok = p->TraceDisconnectWithoutContext(
      "Source1",
      MakeCallback(&IntegerTraceSourceTestCase::NotifySource1, this));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true,
      "Could not TraceConnectWithoutContext() \"Source1\" to NodifySource1()");

  ok = p->SetAttributeFailSafe("IntegerTraceSource1", IntegerValue(1));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true, "Could not SetAttributeFailSafe() via IntegerValue to 1");

  NS_TEST_ASSERT_MSG_EQ(
      m_got1, 0,
      "Hitting a TracedValue after disconnect still causes callback");
}

class TracedCallbackTestCase : public TestCase {
public:
  TracedCallbackTestCase(std::string description);

  ~TracedCallbackTestCase() override {}

private:
  void DoRun() override;

  void NotifySource2(double a, int b [[maybe_unused]],
                     float c [[maybe_unused]]) {
    m_got2 = a;
  }

  double m_got2;
};

TracedCallbackTestCase::TracedCallbackTestCase(std::string description)
    : TestCase(description) {}

void TracedCallbackTestCase::DoRun() {
  Ptr<AttributeObjectTest> p;
  bool ok;

  p = CreateObject<AttributeObjectTest>();
  NS_TEST_ASSERT_MSG_NE(p, nullptr, "Unable to CreateObject");

  m_got2 = 4.3;

  p->InvokeCb(1.0, -5, 0.0);
  NS_TEST_ASSERT_MSG_EQ(m_got2, 4.3,
                        "Invoking a newly created TracedCallback results in an "
                        "unexpected callback");

  ok = p->TraceConnectWithoutContext(
      "Source2", MakeCallback(&TracedCallbackTestCase::NotifySource2, this));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true, "Could not TraceConnectWithoutContext() to NotifySource2");

  p->InvokeCb(1.0, -5, 0.0);
  NS_TEST_ASSERT_MSG_EQ(
      m_got2, 1.0, "Invoking TracedCallback does not result in trace callback");

  ok = p->TraceDisconnectWithoutContext(
      "Source2", MakeCallback(&TracedCallbackTestCase::NotifySource2, this));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true, "Could not TraceDisconnectWithoutContext() from NotifySource2");

  p->InvokeCb(-1.0, -5, 0.0);
  NS_TEST_ASSERT_MSG_EQ(m_got2, 1.0,
                        "Invoking disconnected TracedCallback unexpectedly "
                        "results in trace callback");
}

class PointerAttributeTestCase : public TestCase {
public:
  PointerAttributeTestCase(std::string description);

  ~PointerAttributeTestCase() override {}

private:
  void DoRun() override;

  void NotifySource2(double a, int b [[maybe_unused]],
                     float c [[maybe_unused]]) {
    m_got2 = a;
  }

  double m_got2;
};

PointerAttributeTestCase::PointerAttributeTestCase(std::string description)
    : TestCase(description) {}

void PointerAttributeTestCase::DoRun() {
  Ptr<AttributeObjectTest> p;
  bool ok;

  p = CreateObject<AttributeObjectTest>();
  NS_TEST_ASSERT_MSG_NE(p, nullptr, "Unable to CreateObject");

  PointerValue ptr;
  p->GetAttribute("Pointer", ptr);
  Ptr<Derived> derived = ptr.Get<Derived>();
  NS_TEST_ASSERT_MSG_EQ((bool)derived, false,
                        "Unexpectedly found non-null pointer in newly "
                        "initialized PointerValue Attribute");

  derived = Create<Derived>();
  ok = p->SetAttributeFailSafe("Pointer", PointerValue(derived));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true,
      "Could not SetAttributeFailSafe() a PointerValue of the correct type");

  p->GetAttribute("Pointer", ptr);
  Ptr<Derived> stored = ptr.Get<Derived>();
  NS_TEST_ASSERT_MSG_EQ(
      stored, derived,
      "Retrieved Attribute does not match stored PointerValue");

  p->GetAttribute("Pointer", ptr);
  Ptr<Object> storedBase = ptr.Get<Object>();
  NS_TEST_ASSERT_MSG_EQ(
      storedBase, stored,
      "Retrieved Ptr<Object> does not match stored Ptr<Derived>");

  p->GetAttribute("Pointer", ptr);
  Ptr<AttributeObjectTest> x = ptr.Get<AttributeObjectTest>();
  NS_TEST_ASSERT_MSG_EQ(
      (bool)x, false,
      "Unexpectedly retrieved unrelated Ptr<type> from stored Ptr<Derived>");

  p->GetAttribute("PointerInitialized", ptr);
  Ptr<Derived> storedPtr = ptr.Get<Derived>();
  Ptr<AttributeObjectTest> p2 = CreateObject<AttributeObjectTest>();
  PointerValue ptr2;
  p2->GetAttribute("PointerInitialized", ptr2);
  Ptr<Derived> storedPtr2 = ptr2.Get<Derived>();
  NS_TEST_ASSERT_MSG_NE(
      storedPtr, storedPtr2,
      "ptr and ptr2 both have PointerInitialized pointing to the same object");
  PointerValue ptr3;
  p2->GetAttribute("PointerInitialized", ptr3);
  Ptr<Derived> storedPtr3 = ptr3.Get<Derived>();
  NS_TEST_ASSERT_MSG_NE(
      storedPtr, storedPtr3,
      "ptr and ptr3 both have PointerInitialized pointing to the same object");

  ObjectFactory factory;
  factory.SetTypeId("ns3::AttributeObjectTest");
  factory.Set("PointerInitialized", StringValue("ns3::Derived"));
  Ptr<AttributeObjectTest> aotPtr =
      factory.Create()->GetObject<AttributeObjectTest>();
  NS_TEST_ASSERT_MSG_NE(aotPtr, nullptr,
                        "Unable to factory.Create() a AttributeObjectTest");
  Ptr<AttributeObjectTest> aotPtr2 =
      factory.Create()->GetObject<AttributeObjectTest>();
  NS_TEST_ASSERT_MSG_NE(aotPtr2, nullptr,
                        "Unable to factory.Create() a AttributeObjectTest");
  NS_TEST_ASSERT_MSG_NE(aotPtr, aotPtr2,
                        "factory object not creating unique objects");
  PointerValue ptr4;
  aotPtr->GetAttribute("PointerInitialized", ptr4);
  Ptr<Derived> storedPtr4 = ptr4.Get<Derived>();
  PointerValue ptr5;
  aotPtr2->GetAttribute("PointerInitialized", ptr5);
  Ptr<Derived> storedPtr5 = ptr5.Get<Derived>();
  NS_TEST_ASSERT_MSG_NE(
      storedPtr4, storedPtr5,
      "aotPtr and aotPtr2 are unique, but their Derived member is not");
}

class CallbackValueTestCase : public TestCase {
public:
  CallbackValueTestCase(std::string description);

  ~CallbackValueTestCase() override {}

  void InvokeCbValue(int8_t a) {
    if (!m_cbValue.IsNull()) {
      m_cbValue(a);
    }
  }

private:
  void DoRun() override;

  Callback<void, int8_t> m_cbValue;

  void NotifyCallbackValue(int8_t a) { m_gotCbValue = a; }

  int16_t m_gotCbValue;
};

CallbackValueTestCase::CallbackValueTestCase(std::string description)
    : TestCase(description) {}

void CallbackValueTestCase::DoRun() {
  Ptr<AttributeObjectTest> p;
  bool ok;

  p = CreateObject<AttributeObjectTest>();
  NS_TEST_ASSERT_MSG_NE(p, nullptr, "Unable to CreateObject");

  m_gotCbValue = 1;

  p->InvokeCbValue(2);
  CallbackValue cbValue =
      MakeCallback(&CallbackValueTestCase::NotifyCallbackValue, this);

  NS_TEST_ASSERT_MSG_EQ(m_gotCbValue, 1, "Callback unexpectedly fired");

  ok = p->SetAttributeFailSafe("Callback", cbValue);
  NS_TEST_ASSERT_MSG_EQ(ok, true,
                        "Could not SetAttributeFailSafe() a CallbackValue");

  p->InvokeCbValue(2);
  NS_TEST_ASSERT_MSG_EQ(m_gotCbValue, 2,
                        "Callback Attribute set by CallbackValue did not fire");

  ok = p->SetAttributeFailSafe("Callback",
                               CallbackValue(MakeNullCallback<void, int8_t>()));
  NS_TEST_ASSERT_MSG_EQ(
      ok, true, "Could not SetAttributeFailSafe() a null CallbackValue");

  p->InvokeCbValue(3);
  NS_TEST_ASSERT_MSG_EQ(
      m_gotCbValue, 2,
      "Callback Attribute set to null callback unexpectedly fired");
}

class AttributesTestSuite : public TestSuite {
public:
  AttributesTestSuite();
};

AttributesTestSuite::AttributesTestSuite() : TestSuite("attributes", UNIT) {
  AddTestCase(new AttributeTestCase<BooleanValue>(
                  "Check Attributes of type BooleanValue"),
              TestCase::QUICK);
  AddTestCase(new AttributeTestCase<IntegerValue>(
                  "Check Attributes of type IntegerValue"),
              TestCase::QUICK);
  AddTestCase(new AttributeTestCase<UintegerValue>(
                  "Check Attributes of type UintegerValue"),
              TestCase::QUICK);
  AddTestCase(new AttributeTestCase<DoubleValue>(
                  "Check Attributes of type DoubleValue"),
              TestCase::QUICK);
  AddTestCase(
      new AttributeTestCase<EnumValue>("Check Attributes of type EnumValue"),
      TestCase::QUICK);
  AddTestCase(
      new AttributeTestCase<TimeValue>("Check Attributes of type TimeValue"),
      TestCase::QUICK);
  AddTestCase(new RandomVariableStreamAttributeTestCase(
                  "Check Attributes of type RandomVariableStream"),
              TestCase::QUICK);
  AddTestCase(new ObjectVectorAttributeTestCase(
                  "Check Attributes of type ObjectVectorValue"),
              TestCase::QUICK);
  AddTestCase(
      new ObjectMapAttributeTestCase("Check Attributes of type ObjectMapValue"),
      TestCase::QUICK);
  AddTestCase(
      new PointerAttributeTestCase("Check Attributes of type PointerValue"),
      TestCase::QUICK);
  AddTestCase(
      new CallbackValueTestCase("Check Attributes of type CallbackValue"),
      TestCase::QUICK);
  AddTestCase(new IntegerTraceSourceAttributeTestCase(
                  "Ensure TracedValue<uint8_t> can be set like IntegerValue"),
              TestCase::QUICK);
  AddTestCase(new IntegerTraceSourceTestCase(
                  "Ensure TracedValue<uint8_t> also works as trace source"),
              TestCase::QUICK);
  AddTestCase(
      new TracedCallbackTestCase(
          "Ensure TracedCallback<double, int, float> works as trace source"),
      TestCase::QUICK);
}

static AttributesTestSuite g_attributesTestSuite;
