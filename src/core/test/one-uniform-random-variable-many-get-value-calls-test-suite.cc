
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/random-variable-stream.h"
#include "ns3/test.h"

#include <vector>

namespace ns3 {

namespace tests {

class OneUniformRandomVariableManyGetValueCallsTestCase : public TestCase {
public:
  OneUniformRandomVariableManyGetValueCallsTestCase();
  ~OneUniformRandomVariableManyGetValueCallsTestCase() override;

private:
  void DoRun() override;
};

OneUniformRandomVariableManyGetValueCallsTestCase::
    OneUniformRandomVariableManyGetValueCallsTestCase()
    : TestCase("One Uniform Random Variable with Many GetValue() Calls") {}

OneUniformRandomVariableManyGetValueCallsTestCase::
    ~OneUniformRandomVariableManyGetValueCallsTestCase() {}

void OneUniformRandomVariableManyGetValueCallsTestCase::DoRun() {
  const double min = 0.0;
  const double max = 10.0;

  Config::SetDefault("ns3::UniformRandomVariable::Min", DoubleValue(min));
  Config::SetDefault("ns3::UniformRandomVariable::Max", DoubleValue(max));

  Ptr<UniformRandomVariable> uniform = CreateObject<UniformRandomVariable>();

  double value;
  const int count = 100000000;
  for (int i = 0; i < count; i++) {
    value = uniform->GetValue();

    NS_TEST_ASSERT_MSG_GT(value, min, "Value less than minimum.");
    NS_TEST_ASSERT_MSG_LT(value, max, "Value greater than maximum.");
  }
}

class OneUniformRandomVariableManyGetValueCallsTestSuite : public TestSuite {
public:
  OneUniformRandomVariableManyGetValueCallsTestSuite();
};

OneUniformRandomVariableManyGetValueCallsTestSuite::
    OneUniformRandomVariableManyGetValueCallsTestSuite()
    : TestSuite("one-uniform-random-variable-many-get-value-calls",
                PERFORMANCE) {
  AddTestCase(new OneUniformRandomVariableManyGetValueCallsTestCase);
}

static OneUniformRandomVariableManyGetValueCallsTestSuite
    g_oneUniformRandomVariableManyGetValueCallsTestSuite;

} // namespace tests

} // namespace ns3
