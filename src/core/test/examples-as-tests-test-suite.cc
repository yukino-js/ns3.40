
#include "ns3/example-as-test.h"
#include "ns3/system-path.h"

#include <vector>

using namespace ns3;

namespace ns3 {

namespace tests {

class CommandLineExampleTestCase : public ExampleAsTestCase {
public:
  CommandLineExampleTestCase();

  ~CommandLineExampleTestCase() override;

  std::string GetPostProcessingCommand() const override;
};

CommandLineExampleTestCase::CommandLineExampleTestCase()
    : ExampleAsTestCase(
          "core-example-command-line", "command-line-example",
          NS_TEST_SOURCEDIR,
          "--intArg=2 --boolArg --strArg=deadbeef --anti=t "
          "--cbArg=beefstew --charbuf=stewmeat 3 4 extraOne extraTwo") {}

CommandLineExampleTestCase::~CommandLineExampleTestCase() {}

std::string CommandLineExampleTestCase::GetPostProcessingCommand() const {
  return std::string(R"__(| sed -e "/^Program Version:.*$/d")__");
}

class ExamplesAsTestsTestSuite : public TestSuite {
public:
  ExamplesAsTestsTestSuite();
};

ExamplesAsTestsTestSuite::ExamplesAsTestsTestSuite()
    : TestSuite("examples-as-tests-test-suite", UNIT) {
  AddTestCase(new ExampleAsTestCase("core-example-simulator",
                                    "sample-simulator", NS_TEST_SOURCEDIR));

  AddTestCase(new ExampleAsTestCase("core-example-sample-random-variable",
                                    "sample-random-variable",
                                    NS_TEST_SOURCEDIR));

  AddTestCase(new CommandLineExampleTestCase());
}

static ExamplesAsTestsTestSuite g_examplesAsTestsTestSuite;

static ExampleAsTestSuite g_exampleCommandLineTest("core-example-simulator",
                                                   "sample-simulator",
                                                   NS_TEST_SOURCEDIR);

} // namespace tests

} // namespace ns3
