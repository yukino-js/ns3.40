

#include "ns3/names.h"
#include "ns3/test.h"

namespace ns3 {

namespace tests {

class TestObject : public Object {
public:
  static TypeId GetTypeId() {
    static TypeId tid = TypeId("TestObject")
                            .SetParent<Object>()
                            .SetGroupName("Core")
                            .HideFromDocumentation()
                            .AddConstructor<TestObject>();
    return tid;
  }

  TestObject() {}
};

class AlternateTestObject : public Object {
public:
  static TypeId GetTypeId() {
    static TypeId tid = TypeId("AlternateTestObject")
                            .SetParent<Object>()
                            .SetGroupName("Core")
                            .HideFromDocumentation()
                            .AddConstructor<AlternateTestObject>();
    return tid;
  }

  AlternateTestObject() {}
};

class BasicAddTestCase : public TestCase {
public:
  BasicAddTestCase();
  ~BasicAddTestCase() override;

private:
  void DoRun() override;
  void DoTeardown() override;
};

BasicAddTestCase::BasicAddTestCase()
    : TestCase("Check low level Names::Add and Names::FindName functionality") {
}

BasicAddTestCase::~BasicAddTestCase() {}

void BasicAddTestCase::DoTeardown() { Names::Clear(); }

void BasicAddTestCase::DoRun() {
  std::string found;

  Ptr<TestObject> objectOne = CreateObject<TestObject>();
  Names::Add(Ptr<Object>(nullptr, false), "Name One", objectOne);

  Ptr<TestObject> objectTwo = CreateObject<TestObject>();
  Names::Add(Ptr<Object>(nullptr, false), "Name Two", objectTwo);

  Ptr<TestObject> childOfObjectOne = CreateObject<TestObject>();
  Names::Add(objectOne, "Child", childOfObjectOne);

  Ptr<TestObject> childOfObjectTwo = CreateObject<TestObject>();
  Names::Add(objectTwo, "Child", childOfObjectTwo);

  found = Names::FindName(objectOne);
  NS_TEST_ASSERT_MSG_EQ(found, "Name One",
                        "Could not Names::Add and Names::FindName an Object");

  found = Names::FindName(objectTwo);
  NS_TEST_ASSERT_MSG_EQ(
      found, "Name Two",
      "Could not Names::Add and Names::FindName a second Object");

  found = Names::FindName(childOfObjectOne);
  NS_TEST_ASSERT_MSG_EQ(
      found, "Child",
      "Could not Names::Add and Names::FindName a child Object");

  found = Names::FindName(childOfObjectTwo);
  NS_TEST_ASSERT_MSG_EQ(
      found, "Child",
      "Could not Names::Add and Names::FindName a child Object");
}

class StringContextAddTestCase : public TestCase {
public:
  StringContextAddTestCase();
  ~StringContextAddTestCase() override;

private:
  void DoRun() override;
  void DoTeardown() override;
};

StringContextAddTestCase::StringContextAddTestCase()
    : TestCase(
          "Check string context Names::Add and Names::FindName functionality")

{}

StringContextAddTestCase::~StringContextAddTestCase() {}

void StringContextAddTestCase::DoTeardown() { Names::Clear(); }

void StringContextAddTestCase::DoRun() {
  std::string found;

  Ptr<TestObject> objectOne = CreateObject<TestObject>();
  Names::Add("/Names", "Name One", objectOne);

  Ptr<TestObject> objectTwo = CreateObject<TestObject>();
  Names::Add("/Names", "Name Two", objectTwo);

  Ptr<TestObject> childOfObjectOne = CreateObject<TestObject>();
  Names::Add("/Names/Name One", "Child", childOfObjectOne);

  Ptr<TestObject> childOfObjectTwo = CreateObject<TestObject>();
  Names::Add("/Names/Name Two", "Child", childOfObjectTwo);

  found = Names::FindName(objectOne);
  NS_TEST_ASSERT_MSG_EQ(found, "Name One",
                        "Could not Names::Add and Names::FindName an Object");

  found = Names::FindName(objectTwo);
  NS_TEST_ASSERT_MSG_EQ(
      found, "Name Two",
      "Could not Names::Add and Names::FindName a second Object");

  found = Names::FindName(childOfObjectOne);
  NS_TEST_ASSERT_MSG_EQ(
      found, "Child",
      "Could not Names::Add and Names::FindName a child Object");

  found = Names::FindName(childOfObjectTwo);
  NS_TEST_ASSERT_MSG_EQ(
      found, "Child",
      "Could not Names::Add and Names::FindName a child Object");
}

class FullyQualifiedAddTestCase : public TestCase {
public:
  FullyQualifiedAddTestCase();
  ~FullyQualifiedAddTestCase() override;

private:
  void DoRun() override;
  void DoTeardown() override;
};

FullyQualifiedAddTestCase::FullyQualifiedAddTestCase()
    : TestCase("Check fully qualified path Names::Add and Names::FindName "
               "functionality")

{}

FullyQualifiedAddTestCase::~FullyQualifiedAddTestCase() {}

void FullyQualifiedAddTestCase::DoTeardown() { Names::Clear(); }

void FullyQualifiedAddTestCase::DoRun() {
  std::string found;

  Ptr<TestObject> objectOne = CreateObject<TestObject>();
  Names::Add("/Names/Name One", objectOne);

  Ptr<TestObject> objectTwo = CreateObject<TestObject>();
  Names::Add("/Names/Name Two", objectTwo);

  Ptr<TestObject> childOfObjectOne = CreateObject<TestObject>();
  Names::Add("/Names/Name One/Child", childOfObjectOne);

  Ptr<TestObject> childOfObjectTwo = CreateObject<TestObject>();
  Names::Add("/Names/Name Two/Child", childOfObjectTwo);

  found = Names::FindName(objectOne);
  NS_TEST_ASSERT_MSG_EQ(found, "Name One",
                        "Could not Names::Add and Names::FindName an Object");

  found = Names::FindName(objectTwo);
  NS_TEST_ASSERT_MSG_EQ(
      found, "Name Two",
      "Could not Names::Add and Names::FindName a second Object");

  found = Names::FindName(childOfObjectOne);
  NS_TEST_ASSERT_MSG_EQ(
      found, "Child",
      "Could not Names::Add and Names::FindName a child Object");

  found = Names::FindName(childOfObjectTwo);
  NS_TEST_ASSERT_MSG_EQ(
      found, "Child",
      "Could not Names::Add and Names::FindName a child Object");
}

class RelativeAddTestCase : public TestCase {
public:
  RelativeAddTestCase();
  ~RelativeAddTestCase() override;

private:
  void DoRun() override;
  void DoTeardown() override;
};

RelativeAddTestCase::RelativeAddTestCase()
    : TestCase(
          "Check relative path Names::Add and Names::FindName functionality")

{}

RelativeAddTestCase::~RelativeAddTestCase() {}

void RelativeAddTestCase::DoTeardown() { Names::Clear(); }

void RelativeAddTestCase::DoRun() {
  std::string found;

  Ptr<TestObject> objectOne = CreateObject<TestObject>();
  Names::Add("Name One", objectOne);

  Ptr<TestObject> objectTwo = CreateObject<TestObject>();
  Names::Add("Name Two", objectTwo);

  Ptr<TestObject> childOfObjectOne = CreateObject<TestObject>();
  Names::Add("Name One/Child", childOfObjectOne);

  Ptr<TestObject> childOfObjectTwo = CreateObject<TestObject>();
  Names::Add("Name Two/Child", childOfObjectTwo);

  found = Names::FindName(objectOne);
  NS_TEST_ASSERT_MSG_EQ(found, "Name One",
                        "Could not Names::Add and Names::FindName an Object");

  found = Names::FindName(objectTwo);
  NS_TEST_ASSERT_MSG_EQ(
      found, "Name Two",
      "Could not Names::Add and Names::FindName a second Object");

  found = Names::FindName(childOfObjectOne);
  NS_TEST_ASSERT_MSG_EQ(
      found, "Child",
      "Could not Names::Add and Names::FindName a child Object");

  found = Names::FindName(childOfObjectTwo);
  NS_TEST_ASSERT_MSG_EQ(
      found, "Child",
      "Could not Names::Add and Names::FindName a child Object");
}

class BasicRenameTestCase : public TestCase {
public:
  BasicRenameTestCase();
  ~BasicRenameTestCase() override;

private:
  void DoRun() override;
  void DoTeardown() override;
};

BasicRenameTestCase::BasicRenameTestCase()
    : TestCase("Check low level Names::Rename functionality") {}

BasicRenameTestCase::~BasicRenameTestCase() {}

void BasicRenameTestCase::DoTeardown() { Names::Clear(); }

void BasicRenameTestCase::DoRun() {
  std::string found;

  Ptr<TestObject> objectOne = CreateObject<TestObject>();
  Names::Add(Ptr<Object>(nullptr, false), "Name", objectOne);

  Ptr<TestObject> childOfObjectOne = CreateObject<TestObject>();
  Names::Add(objectOne, "Child", childOfObjectOne);

  found = Names::FindName(objectOne);
  NS_TEST_ASSERT_MSG_EQ(found, "Name",
                        "Could not Names::Add and Names::FindName an Object");

  Names::Rename(Ptr<Object>(nullptr, false), "Name", "New Name");

  found = Names::FindName(objectOne);
  NS_TEST_ASSERT_MSG_EQ(found, "New Name", "Could not Names::Rename an Object");

  found = Names::FindName(childOfObjectOne);
  NS_TEST_ASSERT_MSG_EQ(
      found, "Child",
      "Could not Names::Add and Names::FindName a child Object");

  Names::Rename(objectOne, "Child", "New Child");

  found = Names::FindName(childOfObjectOne);
  NS_TEST_ASSERT_MSG_EQ(found, "New Child",
                        "Could not Names::Rename a child Object");
}

class StringContextRenameTestCase : public TestCase {
public:
  StringContextRenameTestCase();
  ~StringContextRenameTestCase() override;

private:
  void DoRun() override;
  void DoTeardown() override;
};

StringContextRenameTestCase::StringContextRenameTestCase()
    : TestCase("Check string context-based Names::Rename functionality") {}

StringContextRenameTestCase::~StringContextRenameTestCase() {}

void StringContextRenameTestCase::DoTeardown() { Names::Clear(); }

void StringContextRenameTestCase::DoRun() {
  std::string found;

  Ptr<TestObject> objectOne = CreateObject<TestObject>();
  Names::Add("/Names", "Name", objectOne);

  Ptr<TestObject> childOfObjectOne = CreateObject<TestObject>();
  Names::Add("/Names/Name", "Child", childOfObjectOne);

  found = Names::FindName(objectOne);
  NS_TEST_ASSERT_MSG_EQ(found, "Name",
                        "Could not Names::Add and Names::FindName an Object");

  Names::Rename("/Names", "Name", "New Name");

  found = Names::FindName(objectOne);
  NS_TEST_ASSERT_MSG_EQ(found, "New Name", "Could not Names::Rename an Object");

  found = Names::FindName(childOfObjectOne);
  NS_TEST_ASSERT_MSG_EQ(
      found, "Child",
      "Could not Names::Add and Names::FindName a child Object");

  Names::Rename("/Names/New Name", "Child", "New Child");

  found = Names::FindName(childOfObjectOne);
  NS_TEST_ASSERT_MSG_EQ(found, "New Child",
                        "Could not Names::Rename a child Object");
}

class FullyQualifiedRenameTestCase : public TestCase {
public:
  FullyQualifiedRenameTestCase();
  ~FullyQualifiedRenameTestCase() override;

private:
  void DoRun() override;
  void DoTeardown() override;
};

FullyQualifiedRenameTestCase::FullyQualifiedRenameTestCase()
    : TestCase("Check fully qualified path Names::Rename functionality") {}

FullyQualifiedRenameTestCase::~FullyQualifiedRenameTestCase() {}

void FullyQualifiedRenameTestCase::DoTeardown() { Names::Clear(); }

void FullyQualifiedRenameTestCase::DoRun() {
  std::string found;

  Ptr<TestObject> objectOne = CreateObject<TestObject>();
  Names::Add("/Names/Name", objectOne);

  Ptr<TestObject> childOfObjectOne = CreateObject<TestObject>();
  Names::Add("/Names/Name/Child", childOfObjectOne);

  found = Names::FindName(objectOne);
  NS_TEST_ASSERT_MSG_EQ(found, "Name",
                        "Could not Names::Add and Names::FindName an Object");

  Names::Rename("/Names/Name", "New Name");

  found = Names::FindName(objectOne);
  NS_TEST_ASSERT_MSG_EQ(found, "New Name", "Could not Names::Rename an Object");

  found = Names::FindName(childOfObjectOne);
  NS_TEST_ASSERT_MSG_EQ(
      found, "Child",
      "Could not Names::Add and Names::FindName a child Object");

  Names::Rename("/Names/New Name/Child", "New Child");

  found = Names::FindName(childOfObjectOne);
  NS_TEST_ASSERT_MSG_EQ(found, "New Child",
                        "Could not Names::Rename a child Object");
}

class RelativeRenameTestCase : public TestCase {
public:
  RelativeRenameTestCase();
  ~RelativeRenameTestCase() override;

private:
  void DoRun() override;
  void DoTeardown() override;
};

RelativeRenameTestCase::RelativeRenameTestCase()
    : TestCase("Check relative path Names::Rename functionality") {}

RelativeRenameTestCase::~RelativeRenameTestCase() {}

void RelativeRenameTestCase::DoTeardown() { Names::Clear(); }

void RelativeRenameTestCase::DoRun() {
  std::string found;

  Ptr<TestObject> objectOne = CreateObject<TestObject>();
  Names::Add("Name", objectOne);

  Ptr<TestObject> childOfObjectOne = CreateObject<TestObject>();
  Names::Add("Name/Child", childOfObjectOne);

  found = Names::FindName(objectOne);
  NS_TEST_ASSERT_MSG_EQ(found, "Name",
                        "Could not Names::Add and Names::FindName an Object");

  Names::Rename("Name", "New Name");

  found = Names::FindName(objectOne);
  NS_TEST_ASSERT_MSG_EQ(found, "New Name", "Could not Names::Rename an Object");

  found = Names::FindName(childOfObjectOne);
  NS_TEST_ASSERT_MSG_EQ(
      found, "Child",
      "Could not Names::Add and Names::FindName a child Object");

  Names::Rename("New Name/Child", "New Child");

  found = Names::FindName(childOfObjectOne);
  NS_TEST_ASSERT_MSG_EQ(found, "New Child",
                        "Could not Names::Rename a child Object");
}

class FindPathTestCase : public TestCase {
public:
  FindPathTestCase();
  ~FindPathTestCase() override;

private:
  void DoRun() override;
  void DoTeardown() override;
};

FindPathTestCase::FindPathTestCase()
    : TestCase("Check Names::FindPath functionality") {}

FindPathTestCase::~FindPathTestCase() {}

void FindPathTestCase::DoTeardown() { Names::Clear(); }

void FindPathTestCase::DoRun() {
  std::string found;

  Ptr<TestObject> objectOne = CreateObject<TestObject>();
  Names::Add("Name", objectOne);

  Ptr<TestObject> childOfObjectOne = CreateObject<TestObject>();
  Names::Add("/Names/Name/Child", childOfObjectOne);

  found = Names::FindPath(objectOne);
  NS_TEST_ASSERT_MSG_EQ(found, "/Names/Name",
                        "Could not Names::Add and Names::FindPath an Object");

  found = Names::FindPath(childOfObjectOne);
  NS_TEST_ASSERT_MSG_EQ(
      found, "/Names/Name/Child",
      "Could not Names::Add and Names::FindPath a child Object");

  Ptr<TestObject> objectNotThere = CreateObject<TestObject>();
  found = Names::FindPath(objectNotThere);
  NS_TEST_ASSERT_MSG_EQ(found.empty(), true,
                        "Unexpectedly found a non-existent Object");
}

class BasicFindTestCase : public TestCase {
public:
  BasicFindTestCase();
  ~BasicFindTestCase() override;

private:
  void DoRun() override;
  void DoTeardown() override;
};

BasicFindTestCase::BasicFindTestCase()
    : TestCase("Check low level Names::Find functionality") {}

BasicFindTestCase::~BasicFindTestCase() {}

void BasicFindTestCase::DoTeardown() { Names::Clear(); }

void BasicFindTestCase::DoRun() {
  Ptr<TestObject> found;

  Ptr<TestObject> objectOne = CreateObject<TestObject>();
  Names::Add("Name One", objectOne);

  Ptr<TestObject> objectTwo = CreateObject<TestObject>();
  Names::Add("Name Two", objectTwo);

  Ptr<TestObject> childOfObjectOne = CreateObject<TestObject>();
  Names::Add("Name One/Child", childOfObjectOne);

  Ptr<TestObject> childOfObjectTwo = CreateObject<TestObject>();
  Names::Add("Name Two/Child", childOfObjectTwo);

  found = Names::Find<TestObject>(Ptr<Object>(nullptr, false), "Name One");
  NS_TEST_ASSERT_MSG_EQ(
      found, objectOne,
      "Could not find a previously named Object via object context");

  found = Names::Find<TestObject>(Ptr<Object>(nullptr, false), "Name Two");
  NS_TEST_ASSERT_MSG_EQ(
      found, objectTwo,
      "Could not find a previously named Object via object context");

  found = Names::Find<TestObject>(objectOne, "Child");
  NS_TEST_ASSERT_MSG_EQ(
      found, childOfObjectOne,
      "Could not find a previously named child Object via object context");

  found = Names::Find<TestObject>(objectTwo, "Child");
  NS_TEST_ASSERT_MSG_EQ(
      found, childOfObjectTwo,
      "Could not find a previously named child Object via object context");
}

class StringContextFindTestCase : public TestCase {
public:
  StringContextFindTestCase();
  ~StringContextFindTestCase() override;

private:
  void DoRun() override;
  void DoTeardown() override;
};

StringContextFindTestCase::StringContextFindTestCase()
    : TestCase("Check string context-based Names::Find functionality") {}

StringContextFindTestCase::~StringContextFindTestCase() {}

void StringContextFindTestCase::DoTeardown() { Names::Clear(); }

void StringContextFindTestCase::DoRun() {
  Ptr<TestObject> found;

  Ptr<TestObject> objectOne = CreateObject<TestObject>();
  Names::Add("Name One", objectOne);

  Ptr<TestObject> objectTwo = CreateObject<TestObject>();
  Names::Add("Name Two", objectTwo);

  Ptr<TestObject> childOfObjectOne = CreateObject<TestObject>();
  Names::Add("Name One/Child", childOfObjectOne);

  Ptr<TestObject> childOfObjectTwo = CreateObject<TestObject>();
  Names::Add("Name Two/Child", childOfObjectTwo);

  found = Names::Find<TestObject>("/Names", "Name One");
  NS_TEST_ASSERT_MSG_EQ(
      found, objectOne,
      "Could not find a previously named Object via string context");

  found = Names::Find<TestObject>("/Names", "Name Two");
  NS_TEST_ASSERT_MSG_EQ(
      found, objectTwo,
      "Could not find a previously named Object via stribng context");

  found = Names::Find<TestObject>("/Names/Name One", "Child");
  NS_TEST_ASSERT_MSG_EQ(
      found, childOfObjectOne,
      "Could not find a previously named child Object via string context");

  found = Names::Find<TestObject>("/Names/Name Two", "Child");
  NS_TEST_ASSERT_MSG_EQ(
      found, childOfObjectTwo,
      "Could not find a previously named child Object via string context");
}

class FullyQualifiedFindTestCase : public TestCase {
public:
  FullyQualifiedFindTestCase();
  ~FullyQualifiedFindTestCase() override;

private:
  void DoRun() override;
  void DoTeardown() override;
};

FullyQualifiedFindTestCase::FullyQualifiedFindTestCase()
    : TestCase("Check fully qualified path Names::Find functionality") {}

FullyQualifiedFindTestCase::~FullyQualifiedFindTestCase() {}

void FullyQualifiedFindTestCase::DoTeardown() { Names::Clear(); }

void FullyQualifiedFindTestCase::DoRun() {
  Ptr<TestObject> found;

  Ptr<TestObject> objectOne = CreateObject<TestObject>();
  Names::Add("/Names/Name One", objectOne);

  Ptr<TestObject> objectTwo = CreateObject<TestObject>();
  Names::Add("/Names/Name Two", objectTwo);

  Ptr<TestObject> childOfObjectOne = CreateObject<TestObject>();
  Names::Add("/Names/Name One/Child", childOfObjectOne);

  Ptr<TestObject> childOfObjectTwo = CreateObject<TestObject>();
  Names::Add("/Names/Name Two/Child", childOfObjectTwo);

  found = Names::Find<TestObject>("/Names/Name One");
  NS_TEST_ASSERT_MSG_EQ(
      found, objectOne,
      "Could not find a previously named Object via string context");

  found = Names::Find<TestObject>("/Names/Name Two");
  NS_TEST_ASSERT_MSG_EQ(
      found, objectTwo,
      "Could not find a previously named Object via stribng context");

  found = Names::Find<TestObject>("/Names/Name One/Child");
  NS_TEST_ASSERT_MSG_EQ(
      found, childOfObjectOne,
      "Could not find a previously named child Object via string context");

  found = Names::Find<TestObject>("/Names/Name Two/Child");
  NS_TEST_ASSERT_MSG_EQ(
      found, childOfObjectTwo,
      "Could not find a previously named child Object via string context");
}

class RelativeFindTestCase : public TestCase {
public:
  RelativeFindTestCase();
  ~RelativeFindTestCase() override;

private:
  void DoRun() override;
  void DoTeardown() override;
};

RelativeFindTestCase::RelativeFindTestCase()
    : TestCase("Check relative path Names::Find functionality") {}

RelativeFindTestCase::~RelativeFindTestCase() {}

void RelativeFindTestCase::DoTeardown() { Names::Clear(); }

void RelativeFindTestCase::DoRun() {
  Ptr<TestObject> found;

  Ptr<TestObject> objectOne = CreateObject<TestObject>();
  Names::Add("Name One", objectOne);

  Ptr<TestObject> objectTwo = CreateObject<TestObject>();
  Names::Add("Name Two", objectTwo);

  Ptr<TestObject> childOfObjectOne = CreateObject<TestObject>();
  Names::Add("Name One/Child", childOfObjectOne);

  Ptr<TestObject> childOfObjectTwo = CreateObject<TestObject>();
  Names::Add("Name Two/Child", childOfObjectTwo);

  found = Names::Find<TestObject>("Name One");
  NS_TEST_ASSERT_MSG_EQ(
      found, objectOne,
      "Could not find a previously named Object via string context");

  found = Names::Find<TestObject>("Name Two");
  NS_TEST_ASSERT_MSG_EQ(
      found, objectTwo,
      "Could not find a previously named Object via stribng context");

  found = Names::Find<TestObject>("Name One/Child");
  NS_TEST_ASSERT_MSG_EQ(
      found, childOfObjectOne,
      "Could not find a previously named child Object via string context");

  found = Names::Find<TestObject>("Name Two/Child");
  NS_TEST_ASSERT_MSG_EQ(
      found, childOfObjectTwo,
      "Could not find a previously named child Object via string context");
}

class AlternateFindTestCase : public TestCase {
public:
  AlternateFindTestCase();
  ~AlternateFindTestCase() override;

private:
  void DoRun() override;
  void DoTeardown() override;
};

AlternateFindTestCase::AlternateFindTestCase()
    : TestCase("Check GetObject operation in Names::Find") {}

AlternateFindTestCase::~AlternateFindTestCase() {}

void AlternateFindTestCase::DoTeardown() { Names::Clear(); }

void AlternateFindTestCase::DoRun() {
  Ptr<TestObject> testObject = CreateObject<TestObject>();
  Names::Add("Test Object", testObject);

  Ptr<AlternateTestObject> alternateTestObject =
      CreateObject<AlternateTestObject>();
  Names::Add("Alternate Test Object", alternateTestObject);

  Ptr<TestObject> foundTestObject;
  Ptr<AlternateTestObject> foundAlternateTestObject;

  foundTestObject = Names::Find<TestObject>("Test Object");
  NS_TEST_ASSERT_MSG_EQ(
      foundTestObject, testObject,
      "Could not find a previously named TestObject via GetObject");

  foundAlternateTestObject =
      Names::Find<AlternateTestObject>("Alternate Test Object");
  NS_TEST_ASSERT_MSG_EQ(
      foundAlternateTestObject, alternateTestObject,
      "Could not find a previously named AlternateTestObject via GetObject");

  foundAlternateTestObject = Names::Find<AlternateTestObject>("Test Object");
  NS_TEST_ASSERT_MSG_EQ(
      foundAlternateTestObject, nullptr,
      "Unexpectedly able to GetObject<AlternateTestObject> on a TestObject");

  foundTestObject = Names::Find<TestObject>("Alternate Test Object");
  NS_TEST_ASSERT_MSG_EQ(
      foundTestObject, nullptr,
      "Unexpectedly able to GetObject<TestObject> on an AlternateTestObject");
}

class NamesTestSuite : public TestSuite {
public:
  NamesTestSuite();
};

NamesTestSuite::NamesTestSuite() : TestSuite("object-name-service") {
  AddTestCase(new BasicAddTestCase);
  AddTestCase(new StringContextAddTestCase);
  AddTestCase(new FullyQualifiedAddTestCase);
  AddTestCase(new RelativeAddTestCase);
  AddTestCase(new BasicRenameTestCase);
  AddTestCase(new StringContextRenameTestCase);
  AddTestCase(new FullyQualifiedRenameTestCase);
  AddTestCase(new RelativeRenameTestCase);
  AddTestCase(new FindPathTestCase);
  AddTestCase(new BasicFindTestCase);
  AddTestCase(new StringContextFindTestCase);
  AddTestCase(new FullyQualifiedFindTestCase);
  AddTestCase(new RelativeFindTestCase);
  AddTestCase(new AlternateFindTestCase);
}

static NamesTestSuite g_namesTestSuite;

} // namespace tests

} // namespace ns3
