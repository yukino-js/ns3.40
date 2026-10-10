
#include "example-as-test.h"

#include "ascii-test.h"
#include "assert.h"
#include "environment-variable.h"
#include "fatal-error.h"
#include "log.h"

#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("ExampleAsTestCase");

#if defined(NS3_ENABLE_EXAMPLES)

ExampleAsTestCase::ExampleAsTestCase(const std::string name,
                                     const std::string program,
                                     const std::string dataDir,
                                     const std::string args,
                                     const bool shouldNotErr)
    : TestCase(name), m_program(program), m_dataDir(dataDir), m_args(args),
      m_shouldNotErr(shouldNotErr) {
  NS_LOG_FUNCTION(this << name << program << dataDir << args);
}

ExampleAsTestCase::~ExampleAsTestCase() { NS_LOG_FUNCTION_NOARGS(); }

std::string ExampleAsTestCase::GetCommandTemplate() const {
  NS_LOG_FUNCTION_NOARGS();
  std::string command("%s ");
  command += m_args;
  return command;
}

std::string ExampleAsTestCase::GetPostProcessingCommand() const {
  NS_LOG_FUNCTION_NOARGS();
  std::string command("");
  return command;
}

void ExampleAsTestCase::DoRun() {
  NS_LOG_FUNCTION_NOARGS();
  SetDataDir(m_dataDir);
  std::string refFile = CreateDataDirFilename(GetName() + ".reflog");
  std::string testFile = CreateTempDirFilename(GetName() + ".reflog");
  std::string post = GetPostProcessingCommand();

  if (!m_shouldNotErr) {
    post += " | sed '1,/" + std::string(NS_FATAL_MSG) + "/!d' ";
  }

  std::stringstream ss;

  ss << "python3 ./ns3 run " << m_program << " --no-build --command-template=\""
     << GetCommandTemplate() << "\"";

  if (post.empty()) {
    ss << " > " << testFile << " 2>&1";
  } else {
    ss << " 2>&1 " << post << " > " << testFile;
  }

  int status = std::system(ss.str().c_str());

  std::cout << "\n"
            << GetName() << ":\n"
            << "    command:  " << ss.str() << "\n"
            << "    status:   " << status << "\n"
            << "    refFile:  " << refFile << "\n"
            << "    testFile: " << testFile << "\n"
            << "    testFile contents:" << std::endl;

  std::ifstream logF(testFile);
  std::string line;
  while (getline(logF, line)) {
    std::cout << "--- " << line << "\n";
  }
  logF.close();

  if (m_shouldNotErr) {
    NS_TEST_ASSERT_MSG_EQ(status, 0, "example " + m_program + " failed");
  }

  auto [found, intro] =
      EnvironmentVariable::Get("NS_COMMANDLINE_INTROSPECTION");
  if (found) {
    return;
  }

  NS_ASCII_TEST_EXPECT_EQ(testFile, refFile);
}

ExampleAsTestSuite::ExampleAsTestSuite(const std::string name,
                                       const std::string program,
                                       const std::string dataDir,
                                       const std::string args,
                                       const TestDuration duration,
                                       const bool shouldNotErr)
    : TestSuite(name, EXAMPLE) {
  NS_LOG_FUNCTION(this << name << program << dataDir << args << duration
                       << shouldNotErr);
  AddTestCase(new ExampleAsTestCase(name, program, dataDir, args, shouldNotErr),
              duration);
}

#endif

} // namespace ns3
