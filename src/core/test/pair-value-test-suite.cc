
#include <ns3/double.h>
#include <ns3/integer.h>
#include <ns3/log.h>
#include <ns3/object.h>
#include <ns3/pair.h>
#include <ns3/ptr.h>
#include <ns3/string.h>
#include <ns3/test.h>
#include <ns3/type-id.h>

#include <algorithm>
#include <iterator>
#include <sstream>
#include <utility>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("PairTestSuite");

class PairObject : public Object {
public:
  PairObject();
  ~PairObject() override;

  static TypeId GetTypeId();

  friend std::ostream &operator<<(std::ostream &os, const PairObject &obj);

private:
  std::pair<std::string, std::string> m_stringPair;
  std::pair<double, int> m_doubleIntPair;
};

PairObject::PairObject() {}

PairObject::~PairObject() {}

TypeId PairObject::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::PairObject")
          .SetParent<Object>()
          .SetGroupName("Test")
          .AddConstructor<PairObject>()
          .AddAttribute("StringPair", "Pair: string, string",
                        PairValue<StringValue, StringValue>(),
                        MakePairAccessor<StringValue, StringValue>(
                            &PairObject::m_stringPair),
                        MakePairChecker<StringValue, StringValue>(
                            MakeStringChecker(), MakeStringChecker()))
          .AddAttribute(
              "DoubleIntPair", "Pair: double int",
              PairValue<DoubleValue, IntegerValue>(),
              MakePairAccessor<DoubleValue, IntegerValue>(
                  &PairObject::m_doubleIntPair),
              MakePairChecker<DoubleValue, IntegerValue>(
                  MakeDoubleChecker<double>(), MakeIntegerChecker<int>()));
  return tid;
}

std::ostream &operator<<(std::ostream &os, const PairObject &obj) {
  os << "StringPair = { " << obj.m_stringPair << " } ";
  os << "DoubleIntPair = { " << obj.m_doubleIntPair << " }";
  return os;
}

class PairValueTestCase : public TestCase {
public:
  PairValueTestCase();

  ~PairValueTestCase() override {}

private:
  void DoRun() override;
};

PairValueTestCase::PairValueTestCase()
    : TestCase("test instantiation, initialization, access") {}

void PairValueTestCase::DoRun() {
  {
    std::pair<const int, double> ref = {1, 2.4};

    PairValue<IntegerValue, DoubleValue> ac(ref);

    std::pair<const int, double> rv = ac.Get();
    NS_TEST_ASSERT_MSG_EQ(rv, ref, "Attribute value does not equal original");
  }

  {
    std::pair<const std::string, double> ref = {"hello", 3.14};

    PairValue<StringValue, DoubleValue> ac;
    ac.Set(ref);
    std::pair<const std::string, double> rv = ac.Get();
    NS_TEST_ASSERT_MSG_EQ(rv, ref, "Attribute value does not equal original");
  }
}

class PairValueSettingsTestCase : public TestCase {
public:
  PairValueSettingsTestCase();

  void DoRun() override;
};

PairValueSettingsTestCase::PairValueSettingsTestCase()
    : TestCase("test setting through attribute interface") {}

void PairValueSettingsTestCase::DoRun() {
  auto p = CreateObject<PairObject>();
  p->SetAttribute("StringPair", PairValue<StringValue, StringValue>(
                                    std::make_pair("hello", "world")));
  p->SetAttribute("DoubleIntPair", PairValue<DoubleValue, IntegerValue>(
                                       std::make_pair(3.14, 31)));

  std::ostringstream oss;
  oss << *p;

  std::ostringstream ref;
  ref << "StringPair = { (hello,world) } DoubleIntPair = { (3.14,31) }";

  NS_TEST_ASSERT_MSG_EQ((oss.str()), (ref.str()), "Pairs not correctly set");
}

class PairValueTestSuite : public TestSuite {
public:
  PairValueTestSuite();
};

PairValueTestSuite::PairValueTestSuite()
    : TestSuite("pair-value-test-suite", UNIT) {
  AddTestCase(new PairValueTestCase(), TestCase::QUICK);
  AddTestCase(new PairValueSettingsTestCase(), TestCase::QUICK);
}

static PairValueTestSuite g_pairValueTestSuite;
