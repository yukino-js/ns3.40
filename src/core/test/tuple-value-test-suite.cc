
#include <ns3/double.h>
#include <ns3/enum.h>
#include <ns3/log.h>
#include <ns3/object.h>
#include <ns3/ptr.h>
#include <ns3/string.h>
#include <ns3/test.h>
#include <ns3/tuple.h>
#include <ns3/uinteger.h>

#include <algorithm>
#include <iterator>
#include <sstream>
#include <tuple>
#include <utility>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TupleTestSuite");

class TupleObject : public Object {
public:
  enum TupleTestEnum { VALUE1, VALUE2, VALUE3 };

  TupleObject();
  ~TupleObject() override;

  static TypeId GetTypeId();

  using Tuple1Value = TupleValue<StringValue, StringValue, EnumValue>;
  using Tuple1 = Tuple1Value::result_type;
  using Tuple1Pack = Tuple1Value::value_type;

  using Tuple2 = std::tuple<double, uint16_t, std::string>;

  void SetTuple1(const Tuple1 &tuple);
  Tuple1 GetTuple1() const;
  void SetTuple2(const Tuple2 &tuple);
  Tuple2 GetTuple2() const;

private:
  Tuple1 m_tuple1;
  Tuple2 m_tuple2;
};

TypeId TupleObject::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::TupleObject")
          .SetParent<Object>()
          .SetGroupName("Test")
          .AddConstructor<TupleObject>()
          .AddAttribute("StringStringEnumTuple", "Tuple1: string, string, enum",
                        MakeTupleValue<Tuple1Pack>(
                            Tuple1{"Hey", "Jude", TupleObject::VALUE1}),
                        MakeTupleAccessor<Tuple1Pack>(&TupleObject::m_tuple1),
                        MakeTupleChecker<Tuple1Pack>(
                            MakeStringChecker(), MakeStringChecker(),
                            MakeEnumChecker(TupleObject::VALUE1, "VALUE1",
                                            TupleObject::VALUE2, "VALUE2")))
          .AddAttribute(
              "DoubleUintStringTuple", "Tuple2: double, uint16_t, string",
              TupleValue<DoubleValue, UintegerValue, StringValue>(
                  {6.022, 23, "Avogadro"}),
              MakeTupleAccessor<DoubleValue, UintegerValue, StringValue>(
                  &TupleObject::SetTuple2, &TupleObject::GetTuple2),
              MakeTupleChecker<DoubleValue, UintegerValue, StringValue>(
                  MakeDoubleChecker<double>(1.0, 10.0),
                  MakeUintegerChecker<int>(1, 30), MakeStringChecker()));
  return tid;
}

TupleObject::TupleObject() {}

TupleObject::~TupleObject() {}

void TupleObject::SetTuple1(const Tuple1 &tuple) { m_tuple1 = tuple; }

TupleObject::Tuple1 TupleObject::GetTuple1() const { return m_tuple1; }

void TupleObject::SetTuple2(const Tuple2 &tuple) { m_tuple2 = tuple; }

TupleObject::Tuple2 TupleObject::GetTuple2() const { return m_tuple2; }

class TupleValueTestCase : public TestCase {
public:
  TupleValueTestCase();

  ~TupleValueTestCase() override {}

private:
  void DoRun() override;
};

TupleValueTestCase::TupleValueTestCase()
    : TestCase("test TupleValue attribute value") {}

void TupleValueTestCase::DoRun() {
  auto tupleObject = CreateObject<TupleObject>();

  auto t1 = tupleObject->GetTuple1();
  NS_TEST_ASSERT_MSG_EQ((std::get<0>(t1) == "Hey"), true,
                        "First element of tuple 1 not correctly set");
  NS_TEST_ASSERT_MSG_EQ((std::get<1>(t1) == "Jude"), true,
                        "Second element of tuple 1 not correctly set");
  NS_TEST_ASSERT_MSG_EQ(std::get<2>(t1), (int)(TupleObject::VALUE1),
                        "Third element of tuple 1 not correctly set");

  auto t2 = tupleObject->GetTuple2();
  NS_TEST_ASSERT_MSG_EQ(std::get<0>(t2), 6.022,
                        "First element of tuple 2 not correctly set");
  NS_TEST_ASSERT_MSG_EQ(std::get<1>(t2), 23,
                        "Second element of tuple 2 not correctly set");
  NS_TEST_ASSERT_MSG_EQ((std::get<2>(t2) == "Avogadro"), true,
                        "Third element of tuple 2 not correctly set");

  bool ret1 = tupleObject->SetAttributeFailSafe(
      "StringStringEnumTuple",
      MakeTupleValue<TupleObject::Tuple1Pack>(
          TupleObject::Tuple1{"Norwegian", "Wood", TupleObject::VALUE2}));
  NS_TEST_ASSERT_MSG_EQ(ret1, true, "Setting valid values to tuple 1 failed");

  TupleValue<StringValue, StringValue, EnumValue> tupleValue1;
  ret1 =
      tupleObject->GetAttributeFailSafe("StringStringEnumTuple", tupleValue1);
  NS_TEST_ASSERT_MSG_EQ(ret1, true, "Getting values for tuple 1 failed");

  t1 = tupleValue1.Get();
  NS_TEST_ASSERT_MSG_EQ((std::get<0>(t1) == "Norwegian"), true,
                        "First element of tuple 1 not correctly set");
  NS_TEST_ASSERT_MSG_EQ((std::get<1>(t1) == "Wood"), true,
                        "Second element of tuple 1 not correctly set");
  NS_TEST_ASSERT_MSG_EQ(std::get<2>(t1), (int)(TupleObject::VALUE2),
                        "Third element of tuple 1 not correctly set");

  bool ret2 = tupleObject->SetAttributeFailSafe(
      "DoubleUintStringTuple",
      TupleValue<DoubleValue, UintegerValue, StringValue>(
          {8.987, 9, "Coulomb"}));
  NS_TEST_ASSERT_MSG_EQ(ret2, true, "Setting valid values to tuple 2 failed");

  TupleValue<DoubleValue, UintegerValue, StringValue> tupleValue2;
  ret2 =
      tupleObject->GetAttributeFailSafe("DoubleUintStringTuple", tupleValue2);
  NS_TEST_ASSERT_MSG_EQ(ret2, true, "Getting values for tuple 2 failed");

  t2 = tupleValue2.Get();
  NS_TEST_ASSERT_MSG_EQ(std::get<0>(t2), 8.987,
                        "First element of tuple 2 not correctly set");
  NS_TEST_ASSERT_MSG_EQ(std::get<1>(t2), 9,
                        "Second element of tuple 2 not correctly set");
  NS_TEST_ASSERT_MSG_EQ((std::get<2>(t2) == "Coulomb"), true,
                        "Third element of tuple 2 not correctly set");

  ret1 = tupleObject->SetAttributeFailSafe(
      "StringStringEnumTuple", StringValue("{Come, Together, VALUE1}"));
  NS_TEST_ASSERT_MSG_EQ(ret1, true, "Setting valid values to tuple 1 failed");

  t1 = tupleObject->GetTuple1();
  NS_TEST_ASSERT_MSG_EQ((std::get<0>(t1) == "Come"), true,
                        "First element of tuple 1 not correctly set");
  NS_TEST_ASSERT_MSG_EQ((std::get<1>(t1) == "Together"), true,
                        "Second element of tuple 1 not correctly set");
  NS_TEST_ASSERT_MSG_EQ(std::get<2>(t1), (int)(TupleObject::VALUE1),
                        "Third element of tuple 1 not correctly set");

  ret2 = tupleObject->SetAttributeFailSafe(
      "DoubleUintStringTuple", StringValue("{2.99, 8, LightSpeed}"));
  NS_TEST_ASSERT_MSG_EQ(ret2, true, "Setting valid values to tuple 2 failed");

  t2 = tupleObject->GetTuple2();
  NS_TEST_ASSERT_MSG_EQ(std::get<0>(t2), 2.99,
                        "First element of tuple 2 not correctly set");
  NS_TEST_ASSERT_MSG_EQ(std::get<1>(t2), 8,
                        "Second element of tuple 2 not correctly set");
  NS_TEST_ASSERT_MSG_EQ((std::get<2>(t2) == "LightSpeed"), true,
                        "Third element of tuple 2 not correctly set");

  ret1 = tupleObject->SetAttributeFailSafe(
      "StringStringEnumTuple",
      TupleValue<StringValue, StringValue>({"Get", "Back"}));
  NS_TEST_ASSERT_MSG_EQ(ret1, false, "Too few values");
  NS_TEST_ASSERT_MSG_EQ(
      (tupleObject->GetTuple1() ==
       std::make_tuple("Come", "Together", (int)(TupleObject::VALUE1))),
      true, "Tuple modified after failed assignment");

  ret1 = tupleObject->SetAttributeFailSafe(
      "StringStringEnumTuple",
      MakeTupleValue<TupleObject::Tuple1Pack>(
          TupleObject::Tuple1{"Get", "Back", TupleObject::VALUE3}));
  NS_TEST_ASSERT_MSG_EQ(ret1, false, "Invalid enum value");
  NS_TEST_ASSERT_MSG_EQ(
      (tupleObject->GetTuple1() ==
       std::make_tuple("Come", "Together", (int)(TupleObject::VALUE1))),
      true, "Tuple modified after failed assignment");

  ret2 = tupleObject->SetAttributeFailSafe(
      "DoubleUintStringTuple",
      TupleValue<DoubleValue, UintegerValue, StringValue, StringValue>(
          {4.83, 14, "Josephson", "constant"}));
  NS_TEST_ASSERT_MSG_EQ(ret2, false, "Too many values");
  NS_TEST_ASSERT_MSG_EQ(
      (tupleObject->GetTuple2() == std::make_tuple(2.99, 8, "LightSpeed")),
      true, "Tuple modified after failed assignment");

  ret2 = tupleObject->SetAttributeFailSafe(
      "DoubleUintStringTuple",
      TupleValue<DoubleValue, UintegerValue, StringValue>(
          {48.3, 13, "Josephson"}));
  NS_TEST_ASSERT_MSG_EQ(ret2, false, "Double value out of range");
  NS_TEST_ASSERT_MSG_EQ(
      (tupleObject->GetTuple2() == std::make_tuple(2.99, 8, "LightSpeed")),
      true, "Tuple modified after failed assignment");

  ret2 = tupleObject->SetAttributeFailSafe(
      "DoubleUintStringTuple",
      TupleValue<DoubleValue, UintegerValue, StringValue>(
          {4.83, 130, "Josephson"}));
  NS_TEST_ASSERT_MSG_EQ(ret2, false, "Uinteger value out of range");
  NS_TEST_ASSERT_MSG_EQ(
      (tupleObject->GetTuple2() == std::make_tuple(2.99, 8, "LightSpeed")),
      true, "Tuple modified after failed assignment");
}

class TupleValueTestSuite : public TestSuite {
public:
  TupleValueTestSuite();
};

TupleValueTestSuite::TupleValueTestSuite()
    : TestSuite("tuple-value-test-suite", UNIT) {
  AddTestCase(new TupleValueTestCase(), TestCase::QUICK);
}

static TupleValueTestSuite g_tupleValueTestSuite;
