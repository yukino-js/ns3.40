
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/random-variable-stream.h"
#include "ns3/test.h"

#include <vector>

namespace ns3 {

namespace tests {

class ManyUniformRandomVariablesOneGetValueCallTestCase : public TestCase {
public:
  ManyUniformRandomVariablesOneGetValueCallTestCase();
  ~ManyUniformRandomVariablesOneGetValueCallTestCase() override;

private:
  void DoRun() override;
};

ManyUniformRandomVariablesOneGetValueCallTestCase::
    ManyUniformRandomVariablesOneGetValueCallTestCase()
    : TestCase("Many Uniform Random Variables with One GetValue() Call") {}

ManyUniformRandomVariablesOneGetValueCallTestCase::
    ~ManyUniformRandomVariablesOneGetValueCallTestCase() {}

void ManyUniformRandomVariablesOneGetValueCallTestCase::DoRun() {
  const double min = 0.0;
  const double max = 10.0;

  Config::SetDefault("ns3::UniformRandomVariable::Min", DoubleValue(min));
  Config::SetDefault("ns3::UniformRandomVariable::Max", DoubleValue(max));

  double value;
  const int count = 1000000;
  std::vector<Ptr<UniformRandomVariable>> uniformStreamVector(count);
  for (int i = 0; i < count; i++) {
    uniformStreamVector.push_back(CreateObject<UniformRandomVariable>());
    value = uniformStreamVector.back()->GetValue();

    NS_TEST_ASSERT_MSG_GT(value, min, "Value less than minimum.");
    NS_TEST_ASSERT_MSG_LT(value, max, "Value greater than maximum.");
  }
}

class ManyUniformRandomVariablesOneGetValueCallTestSuite : public TestSuite {
public:
  ManyUniformRandomVariablesOneGetValueCallTestSuite();
};

ManyUniformRandomVariablesOneGetValueCallTestSuite::
    ManyUniformRandomVariablesOneGetValueCallTestSuite()
    : TestSuite("many-uniform-random-variables-one-get-value-call",
                PERFORMANCE) {
  AddTestCase(new ManyUniformRandomVariablesOneGetValueCallTestCase);
}

static ManyUniformRandomVariablesOneGetValueCallTestSuite
    g_manyUniformRandomVariablesOneGetValueCallTestSuite;

} // namespace tests

} // namespace ns3
