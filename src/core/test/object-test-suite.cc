#include "ns3/assert.h"
#include "ns3/object-factory.h"
#include "ns3/object.h"
#include "ns3/test.h"

namespace {

class BaseA : public ns3::Object {
public:
  static ns3::TypeId GetTypeId() {
    static ns3::TypeId tid = ns3::TypeId("ObjectTest:BaseA")
                                 .SetParent<Object>()
                                 .SetGroupName("Core")
                                 .HideFromDocumentation()
                                 .AddConstructor<BaseA>();
    return tid;
  }

  BaseA() {}
};

class DerivedA : public BaseA {
public:
  static ns3::TypeId GetTypeId() {
    static ns3::TypeId tid = ns3::TypeId("ObjectTest:DerivedA")
                                 .SetParent<BaseA>()
                                 .SetGroupName("Core")
                                 .HideFromDocumentation()
                                 .AddConstructor<DerivedA>();
    return tid;
  }

  DerivedA() {}

protected:
  void DoDispose() override { BaseA::DoDispose(); }
};

class BaseB : public ns3::Object {
public:
  static ns3::TypeId GetTypeId() {
    static ns3::TypeId tid = ns3::TypeId("ObjectTest:BaseB")
                                 .SetParent<Object>()
                                 .SetGroupName("Core")
                                 .HideFromDocumentation()
                                 .AddConstructor<BaseB>();
    return tid;
  }

  BaseB() {}
};

class DerivedB : public BaseB {
public:
  static ns3::TypeId GetTypeId() {
    static ns3::TypeId tid = ns3::TypeId("ObjectTest:DerivedB")
                                 .SetParent<BaseB>()
                                 .SetGroupName("Core")
                                 .HideFromDocumentation()
                                 .AddConstructor<DerivedB>();
    return tid;
  }

  DerivedB() {}

protected:
  void DoDispose() override { BaseB::DoDispose(); }
};

NS_OBJECT_ENSURE_REGISTERED(BaseA);
NS_OBJECT_ENSURE_REGISTERED(DerivedA);
NS_OBJECT_ENSURE_REGISTERED(BaseB);
NS_OBJECT_ENSURE_REGISTERED(DerivedB);

} // namespace

namespace ns3 {

namespace tests {

class CreateObjectTestCase : public TestCase {
public:
  CreateObjectTestCase();
  ~CreateObjectTestCase() override;

private:
  void DoRun() override;
};

CreateObjectTestCase::CreateObjectTestCase()
    : TestCase("Check CreateObject<Type> template function") {}

CreateObjectTestCase::~CreateObjectTestCase() {}

void CreateObjectTestCase::DoRun() {
  Ptr<BaseA> baseA = CreateObject<BaseA>();
  NS_TEST_ASSERT_MSG_NE(baseA, nullptr, "Unable to CreateObject<BaseA>");

  NS_TEST_ASSERT_MSG_EQ(baseA->GetObject<BaseA>(), baseA,
                        "GetObject() of same type returns different Ptr");

  NS_TEST_ASSERT_MSG_NE(
      !baseA->GetObject<DerivedA>(), 0,
      "GetObject() of unrelated type returns nonzero pointer");

  NS_TEST_ASSERT_MSG_NE(!baseA->GetObject<BaseA>(DerivedA::GetTypeId()), 0,
                        "GetObject() of unrelated returns nonzero Ptr");

  baseA = CreateObject<DerivedA>();
  NS_TEST_ASSERT_MSG_NE(
      baseA, nullptr,
      "Unable to CreateObject<DerivedA> with implicit cast to BaseA");

  NS_TEST_ASSERT_MSG_EQ(baseA->GetObject<BaseA>(), baseA,
                        "Unable to GetObject<BaseA> on BaseA");

  NS_TEST_ASSERT_MSG_EQ(
      baseA->GetObject<DerivedA>(), baseA,
      "GetObject() of the original type returns different Ptr");

  NS_TEST_ASSERT_MSG_EQ(baseA->GetObject<BaseA>(DerivedA::GetTypeId()), baseA,
                        "GetObject returns different Ptr");
}

class AggregateObjectTestCase : public TestCase {
public:
  AggregateObjectTestCase();
  ~AggregateObjectTestCase() override;

private:
  void DoRun() override;
};

AggregateObjectTestCase::AggregateObjectTestCase()
    : TestCase("Check Object aggregation functionality") {}

AggregateObjectTestCase::~AggregateObjectTestCase() {}

void AggregateObjectTestCase::DoRun() {
  Ptr<BaseA> baseA = CreateObject<BaseA>();
  NS_TEST_ASSERT_MSG_NE(baseA, nullptr, "Unable to CreateObject<BaseA>");

  Ptr<BaseB> baseB = CreateObject<BaseB>();
  NS_TEST_ASSERT_MSG_NE(baseB, nullptr, "Unable to CreateObject<BaseB>");

  Ptr<BaseB> baseBCopy = baseB;
  NS_TEST_ASSERT_MSG_NE(baseBCopy, nullptr, "Unable to copy BaseB");

  baseA->AggregateObject(baseB);

  NS_TEST_ASSERT_MSG_NE(baseA->GetObject<BaseA>(), nullptr,
                        "Cannot GetObject (through baseA) for BaseA Object");

  NS_TEST_ASSERT_MSG_NE(!baseA->GetObject<DerivedA>(), 0,
                        "Unexpectedly found a DerivedA through baseA");

  NS_TEST_ASSERT_MSG_NE(baseA->GetObject<BaseB>(), nullptr,
                        "Cannot GetObject (through baseA) for BaseB Object");

  NS_TEST_ASSERT_MSG_NE(!baseA->GetObject<DerivedB>(), 0,
                        "Unexpectedly found a DerivedB through baseA");

  NS_TEST_ASSERT_MSG_NE(baseB->GetObject<BaseB>(), nullptr,
                        "Cannot GetObject (through baseB) for BaseB Object");

  NS_TEST_ASSERT_MSG_NE(!baseB->GetObject<DerivedB>(), 0,
                        "Unexpectedly found a DerivedB through baseB");

  NS_TEST_ASSERT_MSG_NE(baseB->GetObject<BaseA>(), nullptr,
                        "Cannot GetObject (through baseB) for BaseA Object");

  NS_TEST_ASSERT_MSG_NE(!baseB->GetObject<DerivedA>(), 0,
                        "Unexpectedly found a DerivedA through baseB");

  NS_TEST_ASSERT_MSG_NE(
      baseBCopy->GetObject<BaseA>(), nullptr,
      "Cannot GetObject (through baseBCopy) for a BaseA Object");

  baseA = CreateObject<DerivedA>();
  NS_TEST_ASSERT_MSG_NE(
      baseA, nullptr,
      "Unable to CreateObject<DerivedA> with implicit cast to BaseA");

  baseB = CreateObject<DerivedB>();
  NS_TEST_ASSERT_MSG_NE(
      baseB, nullptr,
      "Unable to CreateObject<DerivedB> with implicit cast to BaseB");

  baseBCopy = baseB;
  baseA->AggregateObject(baseB);

  NS_TEST_ASSERT_MSG_NE(baseA->GetObject<DerivedB>(), nullptr,
                        "Cannot GetObject (through baseA) for DerivedB Object");

  NS_TEST_ASSERT_MSG_NE(baseA->GetObject<BaseB>(), nullptr,
                        "Cannot GetObject (through baseA) for BaseB Object");

  NS_TEST_ASSERT_MSG_NE(baseB->GetObject<DerivedA>(), nullptr,
                        "Cannot GetObject (through baseB) for DerivedA Object");

  NS_TEST_ASSERT_MSG_NE(baseB->GetObject<BaseA>(), nullptr,
                        "Cannot GetObject (through baseB) for BaseA Object");

  NS_TEST_ASSERT_MSG_NE(
      baseBCopy->GetObject<BaseA>(), nullptr,
      "Cannot GetObject (through baseBCopy) for a BaseA Object");
  NS_TEST_ASSERT_MSG_NE(
      baseBCopy->GetObject<DerivedA>(), nullptr,
      "Cannot GetObject (through baseBCopy) for a BaseA Object");

  NS_TEST_ASSERT_MSG_NE(baseB->GetObject<DerivedB>(), nullptr,
                        "Cannot GetObject (through baseB) for DerivedB Object");

  NS_TEST_ASSERT_MSG_NE(baseB->GetObject<BaseB>(), nullptr,
                        "Cannot GetObject (through baseB) for BaseB Object");

  baseA = CreateObject<BaseA>();
  NS_TEST_ASSERT_MSG_NE(baseA, nullptr, "Unable to CreateObject<BaseA>");

  baseB = CreateObject<BaseB>();
  NS_TEST_ASSERT_MSG_NE(baseB, nullptr, "Unable to CreateObject<BaseA>");

  baseA->AggregateObject(baseB);
  baseA = nullptr;

  baseA = baseB->GetObject<BaseA>();
  NS_TEST_ASSERT_MSG_NE(baseA, nullptr,
                        "Unable to GetObject on released object");
}

class ObjectFactoryTestCase : public TestCase {
public:
  ObjectFactoryTestCase();
  ~ObjectFactoryTestCase() override;

private:
  void DoRun() override;
};

ObjectFactoryTestCase::ObjectFactoryTestCase()
    : TestCase("Check ObjectFactory functionality") {}

ObjectFactoryTestCase::~ObjectFactoryTestCase() {}

void ObjectFactoryTestCase::DoRun() {
  ObjectFactory factory;

  factory.SetTypeId(BaseA::GetTypeId());
  Ptr<Object> a = factory.Create();
  NS_TEST_ASSERT_MSG_EQ(!a, 0, "Unable to factory.Create() a BaseA");

  NS_TEST_ASSERT_MSG_NE(!a->GetObject<BaseA>(DerivedA::GetTypeId()), 0,
                        "BaseA is unexpectedly a DerivedA also");

  NS_TEST_ASSERT_MSG_NE(
      !a->GetObject<DerivedA>(), 0,
      "BaseA unexpectedly responds to GetObject for DerivedA");

  factory.SetTypeId(DerivedA::GetTypeId());
  a = factory.Create();

  NS_TEST_ASSERT_MSG_EQ(a->GetObject<BaseA>(), a,
                        "Unable to use GetObject as dynamic_cast<BaseA>()");

  NS_TEST_ASSERT_MSG_EQ(a->GetObject<BaseA>(DerivedA::GetTypeId()), a,
                        "GetObject with implied cast returns different Ptr");

  NS_TEST_ASSERT_MSG_EQ(!a->GetObject<DerivedA>(), 0,
                        "Unexpectedly able to work around C++ type system");
}

class ObjectTestSuite : public TestSuite {
public:
  ObjectTestSuite();
};

ObjectTestSuite::ObjectTestSuite() : TestSuite("object") {
  AddTestCase(new CreateObjectTestCase);
  AddTestCase(new AggregateObjectTestCase);
  AddTestCase(new ObjectFactoryTestCase);
}

static ObjectTestSuite g_objectTestSuite;

} // namespace tests

} // namespace ns3
