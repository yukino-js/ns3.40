
#include "test.h"

#include "abort.h"
#include "assert.h"
#include "des-metrics.h"
#include "log.h"
#include "singleton.h"
#include "system-path.h"

#include <cmath>
#include <cstring>
#include <list>
#include <map>
#include <vector>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Test");

bool TestDoubleIsEqual(const double x1, const double x2, const double epsilon) {
  NS_LOG_FUNCTION(x1 << x2 << epsilon);
  int exponent;
  double delta;
  double difference;

  {
    double max = (std::fabs(x1) > std::fabs(x2)) ? x1 : x2;
    std::frexp(max, &exponent);
  }

  delta = std::ldexp(epsilon, exponent);
  difference = x1 - x2;

  return difference <= delta && difference >= -delta;
}

struct TestCaseFailure {
  TestCaseFailure(std::string _cond, std::string _actual, std::string _limit,
                  std::string _message, std::string _file, int32_t _line);
  std::string cond;
  std::string actual;
  std::string limit;
  std::string message;
  std::string file;
  int32_t line;
};

std::ostream &operator<<(std::ostream &os, const TestCaseFailure &failure) {
  os << "    test=\"" << failure.cond << "\" actual=\"" << failure.actual
     << "\" limit=\"" << failure.limit << "\" in=\"" << failure.file << ":"
     << failure.line << "\" " << failure.message;

  return os;
}

struct TestCase::Result {
  Result();

  SystemWallClockMs clock;
  std::vector<TestCaseFailure> failure;
  bool childrenFailed;
};

class TestRunnerImpl : public Singleton<TestRunnerImpl> {
public:
  TestRunnerImpl();

  void AddTestSuite(TestSuite *testSuite);
  bool MustAssertOnFailure() const;
  bool MustContinueOnFailure() const;
  bool MustUpdateData() const;
  std::string GetTopLevelSourceDir() const;
  std::string GetTempDir() const;
  int Run(int argc, char *argv[]);

private:
  bool IsTopLevelSourceDir(std::string path) const;
  std::string ReplaceXmlSpecialCharacters(std::string xml) const;
  void PrintReport(TestCase *test, std::ostream *os, bool xml, int level);
  void PrintTestNameList(std::list<TestCase *>::const_iterator begin,
                         std::list<TestCase *>::const_iterator end,
                         bool printTestType) const;
  void PrintTestTypeList() const;
  void PrintHelp(const char *programName) const;
  std::list<TestCase *> FilterTests(std::string testName,
                                    TestSuite::Type testType,
                                    TestCase::TestDuration maximumTestDuration);

  typedef std::vector<TestSuite *> TestSuiteVector;

  TestSuiteVector m_suites;
  std::string m_tempDir;
  bool m_verbose;
  bool m_assertOnFailure;
  bool m_continueOnFailure;
  bool m_updateData;
};

TestCaseFailure::TestCaseFailure(std::string _cond, std::string _actual,
                                 std::string _limit, std::string _message,
                                 std::string _file, int32_t _line)
    : cond(_cond), actual(_actual), limit(_limit), message(_message),
      file(_file), line(_line) {
  NS_LOG_FUNCTION(this << _cond << _actual << _limit << _message << _file
                       << _line);
}

TestCase::Result::Result() : childrenFailed(false) { NS_LOG_FUNCTION(this); }

TestCase::TestCase(std::string name)
    : m_parent(nullptr), m_dataDir(""), m_runner(nullptr), m_result(nullptr),
      m_name(name), m_duration(TestCase::QUICK) {
  NS_LOG_FUNCTION(this << name);
}

TestCase::~TestCase() {
  NS_LOG_FUNCTION(this);
  NS_ASSERT(m_runner == nullptr);
  m_parent = nullptr;
  delete m_result;
  for (auto i = m_children.begin(); i != m_children.end(); ++i) {
    delete *i;
  }
  m_children.clear();
}

void TestCase::AddTestCase(TestCase *testCase,
                           TestCase::TestDuration duration) {
  NS_LOG_FUNCTION(&testCase << duration);

  std::string badchars = "\"/\\|?";

  std::string::size_type badch = testCase->m_name.find_first_of(badchars);
  if (badch != std::string::npos) {
    NS_LOG_UNCOND("Invalid test name: cannot contain any of '"
                  << badchars << "': " << testCase->m_name);
  }

  testCase->m_duration = duration;
  testCase->m_parent = this;
  m_children.push_back(testCase);
}

bool TestCase::IsFailed() const {
  NS_LOG_FUNCTION(this);
  return m_result->childrenFailed || !m_result->failure.empty();
}

void TestCase::Run(TestRunnerImpl *runner) {
  NS_LOG_FUNCTION(this << runner);
  m_result = new Result();
  m_runner = runner;
  DoSetup();
  m_result->clock.Start();
  for (auto i = m_children.begin(); i != m_children.end(); ++i) {
    TestCase *test = *i;
    test->Run(runner);
    if (IsFailed()) {
      goto out;
    }
  }
  DoRun();
out:
  m_result->clock.End();
  DoTeardown();
  m_runner = nullptr;
}

std::string TestCase::GetName() const {
  NS_LOG_FUNCTION(this);
  return m_name;
}

TestCase *TestCase::GetParent() const { return m_parent; }

void TestCase::ReportTestFailure(std::string cond, std::string actual,
                                 std::string limit, std::string message,
                                 std::string file, int32_t line) {
  NS_LOG_FUNCTION(this << cond << actual << limit << message << file << line);
  m_result->failure.emplace_back(cond, actual, limit, message, file, line);
  TestCase *current = m_parent;
  while (current != nullptr) {
    current->m_result->childrenFailed = true;
    current = current->m_parent;
  }
}

bool TestCase::MustAssertOnFailure() const {
  NS_LOG_FUNCTION(this);
  return m_runner->MustAssertOnFailure();
}

bool TestCase::MustContinueOnFailure() const {
  NS_LOG_FUNCTION(this);
  return m_runner->MustContinueOnFailure();
}

std::string TestCase::CreateDataDirFilename(std::string filename) {
  NS_LOG_FUNCTION(this << filename);
  const TestCase *current = this;
  while (current != nullptr && current->m_dataDir.empty()) {
    current = current->m_parent;
  }
  if (current == nullptr) {
    NS_FATAL_ERROR("No one called SetDataDir prior to calling this function");
  }

  std::string a =
      SystemPath::Append(m_runner->GetTopLevelSourceDir(), current->m_dataDir);
  std::string b = SystemPath::Append(a, filename);
  return b;
}

std::string TestCase::CreateTempDirFilename(std::string filename) {
  NS_LOG_FUNCTION(this << filename);
  if (m_runner->MustUpdateData()) {
    return CreateDataDirFilename(filename);
  } else {
    std::list<std::string> names;
    const TestCase *current = this;
    while (current != nullptr) {
      names.push_front(current->m_name);
      current = current->m_parent;
    }
    std::string tempDir = SystemPath::Append(
        m_runner->GetTempDir(), SystemPath::Join(names.begin(), names.end()));
    tempDir = SystemPath::CreateValidSystemPath(tempDir);

    SystemPath::MakeDirectories(tempDir);
    return SystemPath::Append(tempDir, filename);
  }
}

bool TestCase::IsStatusFailure() const {
  NS_LOG_FUNCTION(this);
  return !IsStatusSuccess();
}

bool TestCase::IsStatusSuccess() const {
  NS_LOG_FUNCTION(this);
  return m_result->failure.empty();
}

void TestCase::SetDataDir(std::string directory) {
  NS_LOG_FUNCTION(this << directory);
  m_dataDir = directory;
}

void TestCase::DoSetup() { NS_LOG_FUNCTION(this); }

void TestCase::DoTeardown() { NS_LOG_FUNCTION(this); }

TestSuite::TestSuite(std::string name, TestSuite::Type type)
    : TestCase(name), m_type(type) {
  NS_LOG_FUNCTION(this << name << type);
  TestRunnerImpl::Get()->AddTestSuite(this);
}

TestSuite::Type TestSuite::GetTestType() {
  NS_LOG_FUNCTION(this);
  return m_type;
}

void TestSuite::DoRun() { NS_LOG_FUNCTION(this); }

TestRunnerImpl::TestRunnerImpl()
    : m_tempDir(""), m_assertOnFailure(false), m_continueOnFailure(true),
      m_updateData(false) {
  NS_LOG_FUNCTION(this);
}

void TestRunnerImpl::AddTestSuite(TestSuite *testSuite) {
  NS_LOG_FUNCTION(this << testSuite);
  m_suites.push_back(testSuite);
}

bool TestRunnerImpl::MustAssertOnFailure() const {
  NS_LOG_FUNCTION(this);
  return m_assertOnFailure;
}

bool TestRunnerImpl::MustContinueOnFailure() const {
  NS_LOG_FUNCTION(this);
  return m_continueOnFailure;
}

bool TestRunnerImpl::MustUpdateData() const {
  NS_LOG_FUNCTION(this);
  return m_updateData;
}

std::string TestRunnerImpl::GetTempDir() const {
  NS_LOG_FUNCTION(this);
  return m_tempDir;
}

bool TestRunnerImpl::IsTopLevelSourceDir(std::string path) const {
  NS_LOG_FUNCTION(this << path);
  bool haveVersion = false;
  bool haveLicense = false;

  std::list<std::string> files = SystemPath::ReadFiles(path);
  for (auto i = files.begin(); i != files.end(); ++i) {
    if (*i == "VERSION") {
      haveVersion = true;
    } else if (*i == "LICENSE") {
      haveLicense = true;
    }
  }

  return haveVersion && haveLicense;
}

std::string TestRunnerImpl::GetTopLevelSourceDir() const {
  NS_LOG_FUNCTION(this);
  std::string self = SystemPath::FindSelfDirectory();
  std::list<std::string> elements = SystemPath::Split(self);
  while (!elements.empty()) {
    std::string path = SystemPath::Join(elements.begin(), elements.end());
    if (IsTopLevelSourceDir(path)) {
      return path;
    }
    elements.pop_back();
  }
  NS_FATAL_ERROR("Could not find source directory from self=" << self);
  return self;
}

std::string TestRunnerImpl::ReplaceXmlSpecialCharacters(std::string xml) const {
  NS_LOG_FUNCTION(this << xml);
  typedef std::map<char, std::string> specials_map;
  specials_map specials;
  specials['<'] = "&lt;";
  specials['>'] = "&gt;";
  specials['&'] = "&amp;";
  specials['"'] = "&#39;";
  specials['\''] = "&quot;";

  std::string result;
  std::size_t length = xml.length();

  for (size_t i = 0; i < length; ++i) {
    char character = xml[i];

    auto it = specials.find(character);

    if (it == specials.end()) {
      result.push_back(character);
    } else {
      result += it->second;
    }
  }
  return result;
}

struct Indent {
  Indent(int level);
  int level;
};

Indent::Indent(int _level) : level(_level) { NS_LOG_FUNCTION(this << _level); }

std::ostream &operator<<(std::ostream &os, const Indent &val) {
  for (int i = 0; i < val.level; i++) {
    os << "  ";
  }
  return os;
}

void TestRunnerImpl::PrintReport(TestCase *test, std::ostream *os, bool xml,
                                 int level) {
  NS_LOG_FUNCTION(this << test << os << xml << level);
  if (test->m_result == nullptr) {
    return;
  }
  const double MS_PER_SEC = 1000.;
  double real = test->m_result->clock.GetElapsedReal() / MS_PER_SEC;
  double user = test->m_result->clock.GetElapsedUser() / MS_PER_SEC;
  double system = test->m_result->clock.GetElapsedSystem() / MS_PER_SEC;

  std::streamsize oldPrecision = (*os).precision(3);
  *os << std::fixed;

  std::string statusString = test->IsFailed() ? "FAIL" : "PASS";
  if (xml) {
    *os << Indent(level) << "<Test>" << std::endl;
    *os << Indent(level + 1) << "<Name>"
        << ReplaceXmlSpecialCharacters(test->m_name) << "</Name>" << std::endl;
    *os << Indent(level + 1) << "<Result>" << statusString << "</Result>"
        << std::endl;
    *os << Indent(level + 1) << "<Time real=\"" << real << "\" user=\"" << user
        << "\" system=\"" << system << "\"/>" << std::endl;
    for (uint32_t i = 0; i < test->m_result->failure.size(); i++) {
      TestCaseFailure failure = test->m_result->failure[i];
      *os << Indent(level + 2) << "<FailureDetails>" << std::endl
          << Indent(level + 3) << "<Condition>"
          << ReplaceXmlSpecialCharacters(failure.cond) << "</Condition>"
          << std::endl
          << Indent(level + 3) << "<Actual>"
          << ReplaceXmlSpecialCharacters(failure.actual) << "</Actual>"
          << std::endl
          << Indent(level + 3) << "<Limit>"
          << ReplaceXmlSpecialCharacters(failure.limit) << "</Limit>"
          << std::endl
          << Indent(level + 3) << "<Message>"
          << ReplaceXmlSpecialCharacters(failure.message) << "</Message>"
          << std::endl
          << Indent(level + 3) << "<File>"
          << ReplaceXmlSpecialCharacters(failure.file) << "</File>" << std::endl
          << Indent(level + 3) << "<Line>" << failure.line << "</Line>"
          << std::endl
          << Indent(level + 2) << "</FailureDetails>" << std::endl;
    }
    for (uint32_t i = 0; i < test->m_children.size(); i++) {
      TestCase *child = test->m_children[i];
      PrintReport(child, os, xml, level + 1);
    }
    *os << Indent(level) << "</Test>" << std::endl;
  } else {
    *os << Indent(level) << statusString << " " << test->GetName() << " "
        << real << " s" << std::endl;
    if (m_verbose) {
      for (uint32_t i = 0; i < test->m_result->failure.size(); i++) {
        *os << Indent(level) << test->m_result->failure[i] << std::endl;
      }
      for (uint32_t i = 0; i < test->m_children.size(); i++) {
        TestCase *child = test->m_children[i];
        PrintReport(child, os, xml, level + 1);
      }
    }
  }

  (*os).unsetf(std::ios_base::floatfield);
  (*os).precision(oldPrecision);
}

void TestRunnerImpl::PrintHelp(const char *program_name) const {
  NS_LOG_FUNCTION(this << program_name);
  std::cout
      << "Usage: " << program_name << " [OPTIONS]" << std::endl
      << std::endl
      << "Options: " << std::endl
      << "  --help                 : print these options" << std::endl
      << "  --print-test-name-list : print the list of names of tests available"
      << std::endl
      << "  --list                 : an alias for --print-test-name-list"
      << std::endl
      << "  --print-test-types     : print the type of tests along with their "
         "names"
      << std::endl
      << "  --print-test-type-list : print the list of types of tests available"
      << std::endl
      << "  --print-temp-dir       : print name of temporary directory before "
         "running "
      << std::endl
      << "                           the tests" << std::endl
      << "  --test-type=TYPE       : process only tests of type TYPE"
      << std::endl
      << "  --test-name=NAME       : process only test whose name matches NAME"
      << std::endl
      << "  --suite=NAME           : an alias (here for compatibility reasons "
         "only) "
      << std::endl
      << "                           for --test-name=NAME" << std::endl
      << "  --assert-on-failure    : when a test fails, crash immediately "
         "(useful"
      << std::endl
      << "                           when running under a debugger" << std::endl
      << "  --stop-on-failure      : when a test fails, stop immediately"
      << std::endl
      << "  --fullness=FULLNESS    : choose the duration of tests to run: "
         "QUICK, "
      << std::endl
      << "                           EXTENSIVE, or TAKES_FOREVER, where "
         "EXTENSIVE "
      << std::endl
      << "                           includes QUICK and TAKES_FOREVER includes "
      << std::endl
      << "                           QUICK and EXTENSIVE (only QUICK tests are "
      << std::endl
      << "                           run by default)" << std::endl
      << "  --verbose              : print details of test execution"
      << std::endl
      << "  --xml                  : format test run output as xml" << std::endl
      << "  --tempdir=DIR          : set temp dir for tests to store output "
         "files"
      << std::endl
      << "  --datadir=DIR          : set data dir for tests to read reference "
         "files"
      << std::endl
      << "  --out=FILE             : send test result to FILE instead of "
         "standard "
      << "output" << std::endl
      << "  --append=FILE          : append test result to FILE instead of "
         "standard "
      << "output" << std::endl;
}

void TestRunnerImpl::PrintTestNameList(
    std::list<TestCase *>::const_iterator begin,
    std::list<TestCase *>::const_iterator end, bool printTestType) const {
  NS_LOG_FUNCTION(this << &begin << &end << printTestType);
  std::map<TestSuite::Type, std::string> label;

  label[TestSuite::ALL] = "all          ";
  label[TestSuite::UNIT] = "unit         ";
  label[TestSuite::SYSTEM] = "system       ";
  label[TestSuite::EXAMPLE] = "example      ";
  label[TestSuite::PERFORMANCE] = "performance  ";

  for (auto i = begin; i != end; ++i) {
    auto test = dynamic_cast<TestSuite *>(*i);
    NS_ASSERT(test != nullptr);
    if (printTestType) {
      std::cout << label[test->GetTestType()];
    }
    std::cout << test->GetName() << std::endl;
  }
}

void TestRunnerImpl::PrintTestTypeList() const {
  NS_LOG_FUNCTION(this);
  std::cout << "  core:        Run all TestSuite-based tests (exclude examples)"
            << std::endl;
  std::cout
      << "  example:     Examples (to see if example programs run successfully)"
      << std::endl;
  std::cout << "  performance: Performance Tests (check to see if the system "
               "is as fast as expected)"
            << std::endl;
  std::cout << "  system:      System Tests (spans modules to check "
               "integration of modules)"
            << std::endl;
  std::cout << "  unit:        Unit Tests (within modules to check basic "
               "functionality)"
            << std::endl;
}

std::list<TestCase *>
TestRunnerImpl::FilterTests(std::string testName, TestSuite::Type testType,
                            TestCase::TestDuration maximumTestDuration) {
  NS_LOG_FUNCTION(this << testName << testType);
  std::list<TestCase *> tests;
  for (uint32_t i = 0; i < m_suites.size(); ++i) {
    TestSuite *test = m_suites[i];
    if (testType != TestSuite::ALL && test->GetTestType() != testType) {
      continue;
    }
    if (!testName.empty() && test->GetName() != testName) {
      continue;
    }

    for (auto j = test->m_children.begin(); j != test->m_children.end();) {
      TestCase *testCase = *j;

      if (testCase->m_duration > maximumTestDuration) {
        delete *j;

        j = test->m_children.erase(j);
      } else {
        ++j;
      }
    }

    tests.push_back(test);
  }
  return tests;
}

int TestRunnerImpl::Run(int argc, char *argv[]) {
  NS_LOG_FUNCTION(this << argc << argv);
  std::string testName = "";
  std::string testTypeString = "";
  std::string out = "";
  std::string fullness = "";
  bool xml = false;
  bool append = false;
  bool printTempDir = false;
  bool printTestTypeList = false;
  bool printTestNameList = false;
  bool printTestTypeAndName = false;
  TestCase::TestDuration maximumTestDuration = TestCase::QUICK;
  char *progname = argv[0];

  char **argi = argv;
  ++argi;

  while (*argi != nullptr) {
    std::string arg = *argi;

    if (arg == "--assert-on-failure") {
      m_assertOnFailure = true;
    } else if (arg == "--stop-on-failure") {
      m_continueOnFailure = false;
    } else if (arg == "--verbose") {
      m_verbose = true;
    } else if (arg == "--print-temp-dir") {
      printTempDir = true;
    } else if (arg == "--update-data") {
      m_updateData = true;
    } else if (arg == "--help") {
      PrintHelp(progname);
      return 0;
    } else if (arg == "--print-test-name-list" || arg == "--list") {
      printTestNameList = true;
    } else if (arg == "--print-test-types") {
      printTestTypeAndName = true;
    } else if (arg == "--print-test-type-list") {
      printTestTypeList = true;
    } else if (arg == "--append") {
      append = true;
    } else if (arg == "--xml") {
      xml = true;
    } else if (arg.find("--test-type=") != std::string::npos) {
      testTypeString = arg.substr(arg.find_first_of('=') + 1);
    } else if (arg.find("--test-name=") != std::string::npos) {
      testName = arg.substr(arg.find_first_of('=') + 1);
    } else if (arg.find("--suite=") != std::string::npos) {
      testName = arg.substr(arg.find_first_of('=') + 1);
    } else if (arg.find("--tempdir=") != std::string::npos) {
      m_tempDir = arg.substr(arg.find_first_of('=') + 1);
    } else if (arg.find("--out=") != std::string::npos) {
      out = arg.substr(arg.find_first_of('=') + 1);
    } else if (arg.find("--fullness=") != std::string::npos) {
      fullness = arg.substr(arg.find_first_of('=') + 1);

      if (fullness == "QUICK") {
        maximumTestDuration = TestCase::QUICK;
      } else if (fullness == "EXTENSIVE") {
        maximumTestDuration = TestCase::EXTENSIVE;
      } else if (fullness == "TAKES_FOREVER") {
        maximumTestDuration = TestCase::TAKES_FOREVER;
      } else {
        PrintHelp(progname);
        return 3;
      }
    } else {
      PrintHelp(progname);
      return 0;
    }
    argi++;
  }
  TestSuite::Type testType;
  if (testTypeString.empty()) {
    testType = TestSuite::ALL;
  } else if (testTypeString == "core") {
    testType = TestSuite::ALL;
  } else if (testTypeString == "example") {
    testType = TestSuite::EXAMPLE;
  } else if (testTypeString == "unit") {
    testType = TestSuite::UNIT;
  } else if (testTypeString == "system") {
    testType = TestSuite::SYSTEM;
  } else if (testTypeString == "performance") {
    testType = TestSuite::PERFORMANCE;
  } else {
    std::cout << "Invalid test type specified: " << testTypeString << std::endl;
    PrintTestTypeList();
    return 1;
  }

  std::list<TestCase *> tests =
      FilterTests(testName, testType, maximumTestDuration);

  if (m_tempDir.empty()) {
    m_tempDir = SystemPath::MakeTemporaryDirectoryName();
  }
  if (printTempDir) {
    std::cout << m_tempDir << std::endl;
  }
  if (printTestNameList) {
    PrintTestNameList(tests.begin(), tests.end(), printTestTypeAndName);
    return 0;
  }
  if (printTestTypeList) {
    PrintTestTypeList();
    return 0;
  }

  std::ostream *os;
  if (!out.empty()) {
    std::ofstream *ofs;
    ofs = new std::ofstream();
    std::ios_base::openmode mode = std::ios_base::out;
    if (append) {
      mode |= std::ios_base::app;
    } else {
      mode |= std::ios_base::trunc;
    }
    ofs->open(out, mode);
    os = ofs;
  } else {
    os = &std::cout;
  }

  bool failed = false;
  if (tests.empty()) {
    std::cerr << "Error:  no tests match the requested string" << std::endl;
    return 1;
  } else if (tests.size() > 1) {
    std::cerr << "Error:  tests should be launched separately (one at a time)"
              << std::endl;
    return 1;
  }

  for (auto i = tests.begin(); i != tests.end(); ++i) {
    TestCase *test = *i;

#ifdef ENABLE_DES_METRICS
    {
      std::string testname = test->GetName();
      std::string runner = "[" + SystemPath::Split(argv[0]).back() + "]";

      std::vector<std::string> desargs;
      desargs.push_back(testname);
      desargs.push_back(runner);
      for (int i = 1; i < argc; ++i) {
        desargs.push_back(argv[i]);
      }

      DesMetrics::Get()->Initialize(desargs, m_tempDir);
    }
#endif

    test->Run(this);
    PrintReport(test, os, xml, 0);
    if (test->IsFailed()) {
      failed = true;
      if (!m_continueOnFailure) {
        return 1;
      }
    }
  }

  if (!out.empty()) {
    delete os;
  }

  return failed ? 1 : 0;
}

int TestRunner::Run(int argc, char *argv[]) {
  NS_LOG_FUNCTION(argc << argv);
  return TestRunnerImpl::Get()->Run(argc, argv);
}

} // namespace ns3
