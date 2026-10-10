
#include <ns3/attribute-container.h>
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
#include <list>
#include <map>
#include <utility>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("AttributeContainerTestSuite");

class AttributeContainerObject : public Object {
public:
  AttributeContainerObject();
  ~AttributeContainerObject() override;

  void ReverseDoubleList();

  static TypeId GetTypeId();

  void SetDoubleList(const std::list<double> &doubleList);
  std::list<double> GetDoubleList() const;

  void SetIntVec(std::vector<int> vec);
  std::vector<int> GetIntVec() const;

private:
  std::list<double> m_doublelist;
  std::vector<int> m_intvec;
  std::map<std::string, int> m_map;
  std::map<int64_t, std::list<int64_t>> m_intVecIntMapping;
};

AttributeContainerObject::AttributeContainerObject() {}

AttributeContainerObject::~AttributeContainerObject() {}

TypeId AttributeContainerObject::GetTypeId() {
  using IntVecMapValue =
      PairValue<IntegerValue, AttributeContainerValue<IntegerValue>>;

  static TypeId tid =
      TypeId("ns3::AttributeContainerObject")
          .SetParent<Object>()
          .SetGroupName("Test")
          .AddConstructor<AttributeContainerObject>()
          .AddAttribute("DoubleList", "List of doubles",
                        AttributeContainerValue<DoubleValue>(),
                        MakeAttributeContainerAccessor<DoubleValue>(
                            &AttributeContainerObject::m_doublelist),
                        MakeAttributeContainerChecker<DoubleValue>(
                            MakeDoubleChecker<double>()))
          .AddAttribute(
              "IntegerVector", "Vector of integers",
              AttributeContainerValue<IntegerValue>(),
              MakeAttributeContainerAccessor<IntegerValue, ';', std::list>(
                  &AttributeContainerObject::SetIntVec,
                  &AttributeContainerObject::GetIntVec),
              MakeAttributeContainerChecker<IntegerValue, ';'>(
                  MakeIntegerChecker<int>()))
          .AddAttribute(
              "MapStringInt", "Map of strings to ints",
              AttributeContainerValue<PairValue<StringValue, IntegerValue>>(),
              MakeAttributeContainerAccessor<
                  PairValue<StringValue, IntegerValue>>(
                  &AttributeContainerObject::m_map),
              MakeAttributeContainerChecker<
                  PairValue<StringValue, IntegerValue>>(
                  MakePairChecker<StringValue, IntegerValue>(
                      MakeStringChecker(), MakeIntegerChecker<int>())))
          .AddAttribute(
              "IntVecPairVec",
              "An example of complex attribute that is defined by a vector of "
              "pairs consisting "
              "of an integer value and a vector of integers. In case a string "
              "is used to set "
              "this attribute, the string shall contain the pairs separated by "
              "a semicolon (;); "
              "in every pair, the integer value and the vector of integers are "
              "separated by a "
              "blank space, and the elements of the vectors are separated by a "
              "comma (,) "
              "without spaces. E.g. \"0 1,2,3; 1 0; 2 0,1\" consists of three "
              "pairs containing "
              "vectors of 3, 1 and 2 elements, respectively.",
              StringValue(""),
              MakeAttributeContainerAccessor<IntVecMapValue, ';'>(
                  &AttributeContainerObject::m_intVecIntMapping),
              MakeAttributeContainerChecker<IntVecMapValue, ';'>(
                  MakePairChecker<IntegerValue,
                                  AttributeContainerValue<IntegerValue>>(
                      MakeIntegerChecker<int>(),
                      MakeAttributeContainerChecker<IntegerValue>(
                          MakeIntegerChecker<int>()))));
  return tid;
}

void AttributeContainerObject::ReverseDoubleList() { m_doublelist.reverse(); }

void AttributeContainerObject::SetDoubleList(
    const std::list<double> &doubleList) {
  m_doublelist = doubleList;
}

std::list<double> AttributeContainerObject::GetDoubleList() const {
  return m_doublelist;
}

void AttributeContainerObject::SetIntVec(std::vector<int> vec) {
  m_intvec = vec;
}

std::vector<int> AttributeContainerObject::GetIntVec() const {
  return m_intvec;
}

template <class A, class B, class C, class D>
bool operator==(const std::pair<A, B> &x, const std::pair<C, D> &y) {
  return x.first == y.first && x.second == y.second;
}

class AttributeContainerTestCase : public TestCase {
public:
  AttributeContainerTestCase();

  ~AttributeContainerTestCase() override {}

private:
  void DoRun() override;
};

AttributeContainerTestCase::AttributeContainerTestCase()
    : TestCase("test instantiation, initialization, access") {}

void AttributeContainerTestCase::DoRun() {
  {
    std::list<double> ref = {1.0, 2.1, 3.145269};

    AttributeContainerValue<DoubleValue> ac(ref);

    NS_TEST_ASSERT_MSG_EQ(ref.size(), ac.GetN(), "Container size mismatch");
    auto aciter = ac.Begin();
    for (auto rend = ref.end(), riter = ref.begin(); riter != rend; ++riter) {
      NS_TEST_ASSERT_MSG_NE(true, (aciter == ac.End()),
                            "AC iterator reached end");
      NS_TEST_ASSERT_MSG_EQ(*riter, (*aciter)->Get(), "Incorrect value");
      ++aciter;
    }
    NS_TEST_ASSERT_MSG_EQ(true, (aciter == ac.End()),
                          "AC iterator did not reach end");
  }

  {
    std::vector<int> ref = {-2, 3, 10, -1042};

    AttributeContainerValue<IntegerValue> ac(ref.begin(), ref.end());

    NS_TEST_ASSERT_MSG_EQ(ref.size(), ac.GetN(), "Container size mismatch");
    auto aciter = ac.Begin();
    for (auto rend = ref.end(), riter = ref.begin(); riter != rend; ++riter) {
      NS_TEST_ASSERT_MSG_NE(true, (aciter == ac.End()),
                            "AC iterator reached end");
      NS_TEST_ASSERT_MSG_EQ(*riter, (*aciter)->Get(), "Incorrect value");
      ++aciter;
    }
    NS_TEST_ASSERT_MSG_EQ(true, (aciter == ac.End()),
                          "AC iterator did not reach end");
  }

  {
    auto ref = {"one", "two", "three"};
    AttributeContainerValue<StringValue> ac(ref.begin(), ref.end());

    NS_TEST_ASSERT_MSG_EQ(ref.size(), ac.GetN(), "Container size mismatch");
    auto aciter = ac.Begin();
    for (auto v : ref) {
      NS_TEST_ASSERT_MSG_NE(true, (aciter == ac.End()),
                            "AC iterator reached end");
      NS_TEST_ASSERT_MSG_EQ(v, (*aciter)->Get(), "Incorrect value");
      ++aciter;
    }
    NS_TEST_ASSERT_MSG_EQ(true, (aciter == ac.End()),
                          "AC iterator did not reach end");
  }

  {
    auto ref = {"one", "two", "three"};
    AttributeContainerValue<StringValue, ',', std::vector> ac(ref);

    NS_TEST_ASSERT_MSG_EQ(ref.size(), ac.GetN(), "Container size mismatch");
    auto aciter = ac.Begin();
    for (auto v : ref) {
      NS_TEST_ASSERT_MSG_NE(true, (aciter == ac.End()),
                            "AC iterator reached end");
      NS_TEST_ASSERT_MSG_EQ(v, (*aciter)->Get(), "Incorrect value");
      ++aciter;
    }
    NS_TEST_ASSERT_MSG_EQ(true, (aciter == ac.End()),
                          "AC iterator did not reach end");
  }

  {
    std::map<std::string, int64_t> ref = {{"one", 1}, {"two", 2}, {"three", 3}};
    AttributeContainerValue<PairValue<StringValue, IntegerValue>> ac(ref);

    NS_TEST_ASSERT_MSG_EQ(ref.size(), ac.GetN(), "Container size mismatch");
    auto aciter = ac.Begin();
    for (const auto &v : ref) {
      NS_TEST_ASSERT_MSG_NE(true, (aciter == ac.End()),
                            "AC iterator reached end");
      NS_TEST_ASSERT_MSG_EQ(v, (*aciter)->Get(), "Incorrect value");
      ++aciter;
    }
    NS_TEST_ASSERT_MSG_EQ(true, (aciter == ac.End()),
                          "AC iterator did not reach end");
  }
}

class AttributeContainerSerializationTestCase : public TestCase {
public:
  AttributeContainerSerializationTestCase();

  ~AttributeContainerSerializationTestCase() override {}

private:
  void DoRun() override;
};

AttributeContainerSerializationTestCase::
    AttributeContainerSerializationTestCase()
    : TestCase("test serialization and deserialization") {}

void AttributeContainerSerializationTestCase::DoRun() {
  {
    std::string doubles = "1.0001, 20.53, -102.3";

    AttributeContainerValue<DoubleValue> attr;
    auto checker = MakeAttributeContainerChecker(attr);
    auto acchecker = DynamicCast<AttributeContainerChecker>(checker);
    acchecker->SetItemChecker(MakeDoubleChecker<double>());
    NS_TEST_ASSERT_MSG_EQ(attr.DeserializeFromString(doubles, checker), true,
                          "Deserialize failed");
    NS_TEST_ASSERT_MSG_EQ(attr.GetN(), 3, "Incorrect container size");

    std::string reserialized = attr.SerializeToString(checker);
    std::string canonical = doubles;
    canonical.erase(std::remove(canonical.begin(), canonical.end(), ' '),
                    canonical.end());
    NS_TEST_ASSERT_MSG_EQ(reserialized, canonical, "Reserialization failed");
  }

  {
    std::string ints = "1, 2, -3, -4";

    AttributeContainerValue<IntegerValue> attr;
    auto checker = MakeAttributeContainerChecker(attr);
    auto acchecker = DynamicCast<AttributeContainerChecker>(checker);
    acchecker->SetItemChecker(MakeIntegerChecker<int>());
    NS_TEST_ASSERT_MSG_EQ(attr.DeserializeFromString(ints, checker), true,
                          "Deserialize failed");
    NS_TEST_ASSERT_MSG_EQ(attr.GetN(), 4, "Incorrect container size");

    std::string reserialized = attr.SerializeToString(checker);
    std::string canonical = ints;
    canonical.erase(std::remove(canonical.begin(), canonical.end(), ' '),
                    canonical.end());
    NS_TEST_ASSERT_MSG_EQ(reserialized, canonical, "Reserialization failed");
  }

  {
    std::string strings = "this is a sentence with words";

    AttributeContainerValue<StringValue, ' '> attr;
    auto checker = MakeAttributeContainerChecker(attr);
    auto acchecker = DynamicCast<AttributeContainerChecker>(checker);
    acchecker->SetItemChecker(MakeStringChecker());
    NS_TEST_ASSERT_MSG_EQ(attr.DeserializeFromString(strings, checker), true,
                          "Deserialize failed");
    NS_TEST_ASSERT_MSG_EQ(attr.GetN(), 6, "Incorrect container size");

    std::string reserialized = attr.SerializeToString(checker);
    NS_TEST_ASSERT_MSG_EQ(reserialized, strings, "Reserialization failed");
  }

  {
    std::string pairs = "one 1,two 2,three 3";
    AttributeContainerValue<PairValue<StringValue, IntegerValue>> attr;
    auto checker = MakeAttributeContainerChecker(attr);
    auto acchecker = DynamicCast<AttributeContainerChecker>(checker);
    acchecker->SetItemChecker(MakePairChecker<StringValue, IntegerValue>(
        MakeStringChecker(), MakeIntegerChecker<int>()));
    NS_TEST_ASSERT_MSG_EQ(attr.DeserializeFromString(pairs, checker), true,
                          "Deserialization failed");
    NS_TEST_ASSERT_MSG_EQ(attr.GetN(), 3, "Incorrect container size");

    std::string reserialized = attr.SerializeToString(checker);
    NS_TEST_ASSERT_MSG_EQ(reserialized, pairs, "Reserealization failed");
  }
}

class AttributeContainerSetGetTestCase : public TestCase {
public:
  AttributeContainerSetGetTestCase();

  ~AttributeContainerSetGetTestCase() override {}

private:
  void DoRun() override;
};

AttributeContainerSetGetTestCase::AttributeContainerSetGetTestCase()
    : TestCase("test attribute set and get") {}

void AttributeContainerSetGetTestCase::DoRun() {
  Ptr<AttributeContainerObject> obj = CreateObject<AttributeContainerObject>();
  {
    auto doubleList = obj->GetDoubleList();
    NS_TEST_ASSERT_MSG_EQ(doubleList.empty(), true,
                          "DoubleList initialized incorrectly");
  }

  const std::list<double> doubles = {1.1, 2.22, 3.333};
  obj->SetAttribute("DoubleList",
                    AttributeContainerValue<DoubleValue>(doubles));
  {
    auto doubleList = obj->GetDoubleList();
    NS_TEST_ASSERT_MSG_EQ(
        std::equal(doubles.begin(), doubles.end(), doubleList.begin()), true,
        "DoubleList incorrectly set");
  }

  obj->ReverseDoubleList();
  {
    auto doubleList = obj->GetDoubleList();
    NS_TEST_ASSERT_MSG_EQ(
        std::equal(doubles.rbegin(), doubles.rend(), doubleList.begin()), true,
        "DoubleList incorrectly reversed");

    AttributeContainerValue<DoubleValue> value;
    obj->GetAttribute("DoubleList", value);
    NS_TEST_ASSERT_MSG_EQ(doubles.size(), value.GetN(),
                          "AttributeContainerValue wrong size");

    AttributeContainerValue<DoubleValue>::result_type doublevec = value.Get();
    NS_TEST_ASSERT_MSG_EQ(doubles.size(), doublevec.size(),
                          "DoublesVec wrong size");
    NS_TEST_ASSERT_MSG_EQ(
        std::equal(doubles.rbegin(), doubles.rend(), doublevec.begin()), true,
        "Incorrect value in doublesvec");
  }

  const std::vector<int> ints = {-1, 0, 1, 2, 3};
  obj->SetAttribute("IntegerVector",
                    AttributeContainerValue<IntegerValue, ';'>(ints));

  {
    AttributeContainerValue<IntegerValue, ';'> value;
    obj->GetAttribute("IntegerVector", value);
    NS_TEST_ASSERT_MSG_EQ(ints.size(), value.GetN(),
                          "AttributeContainerValue wrong size");

    AttributeContainerValue<IntegerValue>::result_type intlist = value.Get();
    NS_TEST_ASSERT_MSG_EQ(ints.size(), intlist.size(), "Intvec wrong size");

    NS_TEST_ASSERT_MSG_EQ(std::equal(ints.begin(), ints.end(), intlist.begin()),
                          true, "Incorrect value in intvec");
  }

  std::string intVecPairString("0 1,2,3; 1 0; 2 0,1");
  obj->SetAttribute("IntVecPairVec", StringValue(intVecPairString));

  {
    using IntVecMapValue =
        PairValue<IntegerValue, AttributeContainerValue<IntegerValue>>;

    AttributeContainerValue<IntVecMapValue, ';'> value;
    obj->GetAttribute("IntVecPairVec", value);
    NS_TEST_ASSERT_MSG_EQ(3, value.GetN(),
                          "AttributeContainerValue wrong size");

    AttributeContainerValue<IntVecMapValue>::result_type reslist = value.Get();
    NS_TEST_ASSERT_MSG_EQ(3, reslist.size(), "IntVecMapValue wrong size");
    auto reslistIt = reslist.begin();
    NS_TEST_ASSERT_MSG_EQ(reslistIt->first, 0,
                          "Incorrect integer value in first pair");
    NS_TEST_ASSERT_MSG_EQ(reslistIt->second.size(), 3,
                          "Incorrect number of integer values in first pair");
    ++reslistIt;
    NS_TEST_ASSERT_MSG_EQ(reslistIt->first, 1,
                          "Incorrect integer value in second pair");
    NS_TEST_ASSERT_MSG_EQ(reslistIt->second.size(), 1,
                          "Incorrect number of integer values in second pair");
    ++reslistIt;
    NS_TEST_ASSERT_MSG_EQ(reslistIt->first, 2,
                          "Incorrect integer value in third pair");
    NS_TEST_ASSERT_MSG_EQ(reslistIt->second.size(), 2,
                          "Incorrect number of integer values in third pair");
  }

  std::map<std::string, int> map = {{"one", 1}, {"two", 2}, {"three", 3}};
  obj->SetAttribute(
      "MapStringInt",
      AttributeContainerValue<PairValue<StringValue, IntegerValue>>(map));

  {
    AttributeContainerValue<PairValue<StringValue, IntegerValue>> value;
    obj->GetAttribute("MapStringInt", value);
    NS_TEST_ASSERT_MSG_EQ(map.size(), value.GetN(),
                          "AttributeContainerValue wrong size");

    std::map<std::string, int> mapstrint;
    auto lst = value.Get();
    for (const auto &l : lst) {
      mapstrint[l.first] = l.second;
    }

    NS_TEST_ASSERT_MSG_EQ(map.size(), mapstrint.size(), "mapstrint wrong size");
    auto iter = map.begin();
    for (const auto &v : mapstrint) {
      NS_TEST_ASSERT_MSG_EQ(v, *iter, "Incorrect value in mapstrint");
      ++iter;
    }
  }
}

class AttributeContainerTestSuite : public TestSuite {
public:
  AttributeContainerTestSuite();
};

AttributeContainerTestSuite::AttributeContainerTestSuite()
    : TestSuite("attribute-container-test-suite", UNIT) {
  AddTestCase(new AttributeContainerTestCase(), TestCase::QUICK);
  AddTestCase(new AttributeContainerSerializationTestCase(), TestCase::QUICK);
  AddTestCase(new AttributeContainerSetGetTestCase(), TestCase::QUICK);
}

static AttributeContainerTestSuite g_attributeContainerTestSuite;
