
#include "ns3/test.h"
#include "ns3/type-traits.h"

namespace ns3 {

namespace tests {

class TypeTraitsTestCase : public TestCase {
public:
  TypeTraitsTestCase();

  ~TypeTraitsTestCase() override {}

private:
  void DoRun() override;
};

TypeTraitsTestCase::TypeTraitsTestCase() : TestCase("Check type traits") {}

void TypeTraitsTestCase::DoRun() {
  NS_TEST_ASSERT_MSG_EQ(
      TypeTraits<void (TypeTraitsTestCase::*)()>::IsPointerToMember, 1,
      "Check pointer to member function ()");
  NS_TEST_ASSERT_MSG_EQ(
      TypeTraits<void (TypeTraitsTestCase::*)() const>::IsPointerToMember, 1,
      "Check pointer to member function () const");
  NS_TEST_ASSERT_MSG_EQ(
      TypeTraits<void (TypeTraitsTestCase::*)(int)>::IsPointerToMember, 1,
      "Check pointer to member function (int)");
  NS_TEST_ASSERT_MSG_EQ(
      TypeTraits<void (TypeTraitsTestCase::*)(int) const>::IsPointerToMember, 1,
      "Check pointer to member function (int) const");
  NS_TEST_ASSERT_MSG_EQ(
      TypeTraits<void (TypeTraitsTestCase::*)()
                     const>::PointerToMemberTraits::nArgs,
      0, "Check number of arguments for pointer to member function () const");
  NS_TEST_ASSERT_MSG_EQ(
      TypeTraits<void (TypeTraitsTestCase::*)(int)
                     const>::PointerToMemberTraits::nArgs,
      1,
      "Check number of arguments for pointer to member function (int) const");
}

class TypeTraitsTestSuite : public TestSuite {
public:
  TypeTraitsTestSuite();
};

TypeTraitsTestSuite::TypeTraitsTestSuite() : TestSuite("type-traits") {
  AddTestCase(new TypeTraitsTestCase);
}

static TypeTraitsTestSuite g_typeTraitsTestSuite;

} // namespace tests

} // namespace ns3
