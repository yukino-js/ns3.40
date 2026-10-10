
#ifndef NS3_EXAMPLE_AS_TEST_SUITE_H
#define NS3_EXAMPLE_AS_TEST_SUITE_H

#include "test.h"

#include <string>

namespace ns3 {

class ExampleAsTestCase : public TestCase {
public:
  ExampleAsTestCase(const std::string name, const std::string program,
                    const std::string dataDir, const std::string args = "",
                    const bool shouldNotErr = true);

  ~ExampleAsTestCase() override;

  virtual std::string GetCommandTemplate() const;

  virtual std::string GetPostProcessingCommand() const;

  void DoRun() override;

protected:
  std::string m_program;
  std::string m_dataDir;
  std::string m_args;
  bool m_shouldNotErr;
};

class ExampleAsTestSuite : public TestSuite {
public:
  ExampleAsTestSuite(const std::string name, const std::string program,
                     const std::string dataDir, const std::string args = "",
                     const TestDuration duration = QUICK,
                     const bool shouldNotErr = true);
};

} // namespace ns3

#endif
