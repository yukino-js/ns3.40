#include "ns3/callback.h"
#include "ns3/config.h"
#include "ns3/integer.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/object-vector.h"
#include "ns3/object.h"
#include "ns3/pointer.h"
#include "ns3/singleton.h"
#include "ns3/test.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/traced-value.h"

#include <sstream>

namespace ns3 {

namespace tests {

class ConfigTestObject : public Object {
public:
  static TypeId GetTypeId();

  void AddNodeA(Ptr<ConfigTestObject> a);
  void AddNodeB(Ptr<ConfigTestObject> b);

  void SetNodeA(Ptr<ConfigTestObject> a);
  void SetNodeB(Ptr<ConfigTestObject> b);

  int8_t GetA() const;
  int8_t GetB() const;

private:
  std::vector<Ptr<ConfigTestObject>> m_nodesA;
  std::vector<Ptr<ConfigTestObject>> m_nodesB;
  Ptr<ConfigTestObject> m_nodeA;
  Ptr<ConfigTestObject> m_nodeB;
  int8_t m_a;
  int8_t m_b;
  TracedValue<int16_t> m_trace;
};

TypeId ConfigTestObject::GetTypeId() {
  static TypeId tid =
      TypeId("ConfigTestObject")
          .SetParent<Object>()
          .AddAttribute("NodesA", "", ObjectVectorValue(),
                        MakeObjectVectorAccessor(&ConfigTestObject::m_nodesA),
                        MakeObjectVectorChecker<ConfigTestObject>())
          .AddAttribute("NodesB", "", ObjectVectorValue(),
                        MakeObjectVectorAccessor(&ConfigTestObject::m_nodesB),
                        MakeObjectVectorChecker<ConfigTestObject>())
          .AddAttribute("NodeA", "", PointerValue(),
                        MakePointerAccessor(&ConfigTestObject::m_nodeA),
                        MakePointerChecker<ConfigTestObject>())
          .AddAttribute("NodeB", "", PointerValue(),
                        MakePointerAccessor(&ConfigTestObject::m_nodeB),
                        MakePointerChecker<ConfigTestObject>())
          .AddAttribute("A", "", IntegerValue(10),
                        MakeIntegerAccessor(&ConfigTestObject::m_a),
                        MakeIntegerChecker<int8_t>())
          .AddAttribute("B", "", IntegerValue(9),
                        MakeIntegerAccessor(&ConfigTestObject::m_b),
                        MakeIntegerChecker<int8_t>())
          .AddAttribute("Source", "XX", IntegerValue(-1),
                        MakeIntegerAccessor(&ConfigTestObject::m_trace),
                        MakeIntegerChecker<int16_t>())
          .AddTraceSource("Source", "XX",
                          MakeTraceSourceAccessor(&ConfigTestObject::m_trace),
                          "ns3::TracedValueCallback::Int16");
  return tid;
}

void ConfigTestObject::SetNodeA(Ptr<ConfigTestObject> a) { m_nodeA = a; }

void ConfigTestObject::SetNodeB(Ptr<ConfigTestObject> b) { m_nodeB = b; }

void ConfigTestObject::AddNodeA(Ptr<ConfigTestObject> a) {
  m_nodesA.push_back(a);
}

void ConfigTestObject::AddNodeB(Ptr<ConfigTestObject> b) {
  m_nodesB.push_back(b);
}

int8_t ConfigTestObject::GetA() const { return m_a; }

int8_t ConfigTestObject::GetB() const { return m_b; }

class DerivedConfigTestObject : public ConfigTestObject {
public:
  static TypeId GetTypeId();

  DerivedConfigTestObject() {}

  ~DerivedConfigTestObject() override {}
};

TypeId DerivedConfigTestObject::GetTypeId() {
  static TypeId tid =
      TypeId("DerivedConfigTestObject").SetParent<ConfigTestObject>();
  return tid;
}

class BaseConfigObject : public Object {
public:
  static TypeId GetTypeId();

  BaseConfigObject() : m_x(15) {}

  ~BaseConfigObject() override {}

private:
  int8_t m_x;
};

TypeId BaseConfigObject::GetTypeId() {
  static TypeId tid =
      TypeId("BaseConfigObject")
          .SetParent<Object>()
          .AddAttribute("X", "", IntegerValue(10),
                        MakeIntegerAccessor(&BaseConfigObject::m_x),
                        MakeIntegerChecker<int8_t>());
  return tid;
}

class DerivedConfigObject : public BaseConfigObject {
public:
  static TypeId GetTypeId();

  DerivedConfigObject() {}

  ~DerivedConfigObject() override {}
};

TypeId DerivedConfigObject::GetTypeId() {
  static TypeId tid =
      TypeId("DerivedConfigObject").SetParent<BaseConfigObject>();
  return tid;
}

class RootNamespaceConfigTestCase : public TestCase {
public:
  RootNamespaceConfigTestCase();

  ~RootNamespaceConfigTestCase() override {}

private:
  void DoRun() override;
};

RootNamespaceConfigTestCase::RootNamespaceConfigTestCase()
    : TestCase("Check ability to register a root namespace and use it") {}

void RootNamespaceConfigTestCase::DoRun() {
  IntegerValue iv;
  Ptr<ConfigTestObject> root = CreateObject<ConfigTestObject>();
  Config::RegisterRootNamespaceObject(root);

  root->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), 10,
                        "Object Attribute \"A\" not initialized as expected");

  Config::Set("/A", IntegerValue(1));
  root->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), 1,
                        "Object Attribute \"A\" not set correctly");

  root->GetAttribute("B", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), 9,
                        "Object Attribute \"B\" not initialized as expected");

  Config::Set("/B", IntegerValue(-1));
  root->GetAttribute("B", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), -1,
                        "Object Attribute \"B\" not set correctly");
}

class UnderRootNamespaceConfigTestCase : public TestCase {
public:
  UnderRootNamespaceConfigTestCase();

  ~UnderRootNamespaceConfigTestCase() override {}

private:
  void DoRun() override;
};

UnderRootNamespaceConfigTestCase::UnderRootNamespaceConfigTestCase()
    : TestCase("Check ability to register an object under the root namespace "
               "and use it") {}

void UnderRootNamespaceConfigTestCase::DoRun() {
  IntegerValue iv;
  Ptr<ConfigTestObject> root = CreateObject<ConfigTestObject>();
  Config::RegisterRootNamespaceObject(root);

  Ptr<ConfigTestObject> a = CreateObject<ConfigTestObject>();
  root->SetNodeA(a);

  a->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), 10,
                        "Object Attribute \"A\" not initialized as expected");

  Config::Set("/NodeA/A", IntegerValue(1));
  a->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), 1,
                        "Object Attribute \"A\" not set correctly");

  a->GetAttribute("B", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), 9,
                        "Object Attribute \"B\" not initialized as expected");

  Config::Set("/NodeA/B", IntegerValue(-1));
  a->GetAttribute("B", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), -1,
                        "Object Attribute \"B\" not set correctly");

  Config::Set("/NodeB/A", IntegerValue(1234));
  a->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), 1,
                        "Object Attribute \"A\" unexpectedly set via bad path");

  Config::Set("/NodeB/B", IntegerValue(1234));
  a->GetAttribute("B", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), -1,
                        "Object Attribute \"B\" unexpectedly set via bad path");

  Ptr<ConfigTestObject> b = CreateObject<ConfigTestObject>();

  b->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), 10,
                        "Object Attribute \"A\" not initialized as expected");
  b->GetAttribute("B", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), 9,
                        "Object Attribute \"B\" not initialized as expected");

  a->SetNodeB(b);

  Config::Set("/NodeA/NodeB/A", IntegerValue(4));
  Config::Set("/NodeA/NodeB/B", IntegerValue(-4));
  b->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), 4,
                        "Object Attribute \"A\" not set as expected");
  b->GetAttribute("B", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), -4,
                        "Object Attribute \"B\" not set as expected");

  Config::Set("/*/A", IntegerValue(2));
  a->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), 2,
                        "Object Attribute \"A\" not set correctly");
  b->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), 4,
                        "Object Attribute \"A\" not set correctly");
}

class ObjectVectorConfigTestCase : public TestCase {
public:
  ObjectVectorConfigTestCase();

  ~ObjectVectorConfigTestCase() override {}

private:
  void DoRun() override;
};

ObjectVectorConfigTestCase::ObjectVectorConfigTestCase()
    : TestCase("Check ability to configure vectors of Object using regular "
               "expressions") {}

void ObjectVectorConfigTestCase::DoRun() {
  IntegerValue iv;

  Ptr<ConfigTestObject> root = CreateObject<ConfigTestObject>();
  Config::RegisterRootNamespaceObject(root);

  Ptr<ConfigTestObject> a = CreateObject<ConfigTestObject>();
  root->SetNodeA(a);

  Ptr<ConfigTestObject> b = CreateObject<ConfigTestObject>();
  a->SetNodeB(b);

  Ptr<ConfigTestObject> obj0 = CreateObject<ConfigTestObject>();
  Ptr<ConfigTestObject> obj1 = CreateObject<ConfigTestObject>();
  Ptr<ConfigTestObject> obj2 = CreateObject<ConfigTestObject>();
  Ptr<ConfigTestObject> obj3 = CreateObject<ConfigTestObject>();
  b->AddNodeB(obj0);
  b->AddNodeB(obj1);
  b->AddNodeB(obj2);
  b->AddNodeB(obj3);

  Config::Set("/NodeA/NodeB/NodesB/0/A", IntegerValue(-11));
  obj0->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), -11,
                        "Object Attribute \"A\" not set as expected");

  obj1->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), 10,
                        "Object Attribute \"A\" unexpectedly set");

  obj2->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), 10,
                        "Object Attribute \"A\" unexpectedly set");

  obj3->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), 10,
                        "Object Attribute \"A\" unexpectedly set");

  Config::Set("/NodeA/NodeB/NodesB/0|1/A", IntegerValue(-12));
  obj0->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), -12,
                        "Object Attribute \"A\" not set as expected");

  obj1->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), -12,
                        "Object Attribute \"A\" not set as expected");

  obj2->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), 10,
                        "Object Attribute \"A\" unexpectedly set");

  obj3->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), 10,
                        "Object Attribute \"A\" unexpectedly set");

  Config::Set("/NodeA/NodeB/NodesB/|0|1|/A", IntegerValue(-13));
  obj0->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), -13,
                        "Object Attribute \"A\" not set as expected");

  obj1->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), -13,
                        "Object Attribute \"A\" not set as expected");

  obj2->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), 10,
                        "Object Attribute \"A\" unexpectedly set");

  obj3->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), 10,
                        "Object Attribute \"A\" unexpectedly set");

  Config::Set("/NodeA/NodeB/NodesB/[0-2]/A", IntegerValue(-14));
  obj0->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), -14,
                        "Object Attribute \"A\" not set as expected");

  obj1->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), -14,
                        "Object Attribute \"A\" not set as expected");

  obj2->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), -14,
                        "Object Attribute \"A\" not set as expected");

  obj3->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), 10,
                        "Object Attribute \"A\" unexpectedly set");

  Config::Set("/NodeA/NodeB/NodesB/[1-3]/A", IntegerValue(-15));
  obj0->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), -14,
                        "Object Attribute \"A\" unexpectedly set");

  obj1->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), -15,
                        "Object Attribute \"A\" not set as expected");

  obj2->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), -15,
                        "Object Attribute \"A\" not set as expected");

  obj3->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), -15,
                        "Object Attribute \"A\" not set as expected");

  Config::Set("/NodeA/NodeB/NodesB/[0-1]|3/A", IntegerValue(-16));
  obj0->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), -16,
                        "Object Attribute \"A\" not set as expected");

  obj1->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), -16,
                        "Object Attribute \"A\" not set as expected");

  obj2->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), -15,
                        "Object Attribute \"A\" unexpectedly set");

  obj3->GetAttribute("A", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), -16,
                        "Object Attribute \"A\" not set as expected");
}

class ObjectVectorTraceConfigTestCase : public TestCase {
public:
  ObjectVectorTraceConfigTestCase();

  ~ObjectVectorTraceConfigTestCase() override {}

  void Trace(int16_t oldValue [[maybe_unused]], int16_t newValue) {
    m_newValue = newValue;
  }

  void TraceWithPath(std::string path, int16_t old [[maybe_unused]],
                     int16_t newValue) {
    m_newValue = newValue;
    m_path = path;
  }

private:
  void DoRun() override;

  int16_t m_newValue;
  std::string m_path;
};

ObjectVectorTraceConfigTestCase::ObjectVectorTraceConfigTestCase()
    : TestCase("Check ability to trace connect through vectors of Object using "
               "regular expressions") {}

void ObjectVectorTraceConfigTestCase::DoRun() {
  IntegerValue iv;

  Ptr<ConfigTestObject> root = CreateObject<ConfigTestObject>();
  Config::RegisterRootNamespaceObject(root);

  Ptr<ConfigTestObject> a = CreateObject<ConfigTestObject>();
  root->SetNodeA(a);

  Ptr<ConfigTestObject> b = CreateObject<ConfigTestObject>();
  a->SetNodeB(b);

  Ptr<ConfigTestObject> obj0 = CreateObject<ConfigTestObject>();
  Ptr<ConfigTestObject> obj1 = CreateObject<ConfigTestObject>();
  Ptr<ConfigTestObject> obj2 = CreateObject<ConfigTestObject>();
  Ptr<ConfigTestObject> obj3 = CreateObject<ConfigTestObject>();
  b->AddNodeB(obj0);
  b->AddNodeB(obj1);
  b->AddNodeB(obj2);
  b->AddNodeB(obj3);

  Config::ConnectWithoutContext(
      "/NodeA/NodeB/NodesB/[0-1]|3/Source",
      MakeCallback(&ObjectVectorTraceConfigTestCase::Trace, this));

  m_newValue = 0;
  obj0->SetAttribute("Source", IntegerValue(-1));
  NS_TEST_ASSERT_MSG_EQ(m_newValue, -1, "Trace 0 did not fire as expected");

  m_newValue = 0;
  obj1->SetAttribute("Source", IntegerValue(-2));
  NS_TEST_ASSERT_MSG_EQ(m_newValue, -2, "Trace 1 did not fire as expected");

  m_newValue = 0;
  obj2->SetAttribute("Source", IntegerValue(-3));
  NS_TEST_ASSERT_MSG_EQ(m_newValue, 0, "Trace 2 fired unexpectedly");

  m_newValue = 0;
  obj3->SetAttribute("Source", IntegerValue(-4));
  NS_TEST_ASSERT_MSG_EQ(m_newValue, -4, "Trace 3 did not fire as expected");

  Config::Connect(
      "/NodeA/NodeB/NodesB/[0-1]|3/Source",
      MakeCallback(&ObjectVectorTraceConfigTestCase::TraceWithPath, this));

  m_newValue = 0;
  m_path = "";
  obj0->SetAttribute("Source", IntegerValue(-1));
  NS_TEST_ASSERT_MSG_EQ(m_newValue, -1, "Trace 0 did not fire as expected");
  NS_TEST_ASSERT_MSG_EQ(m_path, "/NodeA/NodeB/NodesB/0/Source",
                        "Trace 0 did not provide expected context");

  m_newValue = 0;
  m_path = "";
  obj1->SetAttribute("Source", IntegerValue(-2));
  NS_TEST_ASSERT_MSG_EQ(m_newValue, -2, "Trace 1 did not fire as expected");
  NS_TEST_ASSERT_MSG_EQ(m_path, "/NodeA/NodeB/NodesB/1/Source",
                        "Trace 1 did not provide expected context");

  m_newValue = 0;
  m_path = "";
  obj2->SetAttribute("Source", IntegerValue(-3));
  NS_TEST_ASSERT_MSG_EQ(m_newValue, 0, "Trace 2 fired unexpectedly");

  m_newValue = 0;
  m_path = "";
  obj3->SetAttribute("Source", IntegerValue(-4));
  NS_TEST_ASSERT_MSG_EQ(m_newValue, -4, "Trace 3 did not fire as expected");
  NS_TEST_ASSERT_MSG_EQ(m_path, "/NodeA/NodeB/NodesB/1/Source",
                        "Trace 1 did not provide expected context");
}

class SearchAttributesOfParentObjectsTestCase : public TestCase {
public:
  SearchAttributesOfParentObjectsTestCase();

  ~SearchAttributesOfParentObjectsTestCase() override {}

private:
  void DoRun() override;
};

SearchAttributesOfParentObjectsTestCase::
    SearchAttributesOfParentObjectsTestCase()
    : TestCase("Check that attributes of base class are searchable from paths "
               "including objects of "
               "derived class") {}

void SearchAttributesOfParentObjectsTestCase::DoRun() {
  IntegerValue iv;
  Ptr<DerivedConfigTestObject> root = CreateObject<DerivedConfigTestObject>();
  Config::RegisterRootNamespaceObject(root);

  Ptr<DerivedConfigTestObject> a = CreateObject<DerivedConfigTestObject>();
  root->SetNodeA(a);

  Ptr<DerivedConfigObject> derived = CreateObject<DerivedConfigObject>();
  a->AggregateObject(derived);
  Config::Set("/NodeA/$DerivedConfigObject/X", IntegerValue(42));
  derived->GetAttribute("X", iv);
  NS_TEST_ASSERT_MSG_EQ(iv.Get(), 42,
                        "Object Attribute \"X\" not settable in derived class");
}

class ConfigTestSuite : public TestSuite {
public:
  ConfigTestSuite();
};

ConfigTestSuite::ConfigTestSuite() : TestSuite("config") {
  AddTestCase(new RootNamespaceConfigTestCase);
  AddTestCase(new UnderRootNamespaceConfigTestCase);
  AddTestCase(new ObjectVectorConfigTestCase);
  AddTestCase(new SearchAttributesOfParentObjectsTestCase);
}

static ConfigTestSuite g_configTestSuite;

} // namespace tests

} // namespace ns3
