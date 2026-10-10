
#ifndef NS3_TEST_H
#define NS3_TEST_H

#include "system-wall-clock-ms.h"

#include <fstream>
#include <iostream>
#include <limits>
#include <list>
#include <sstream>
#include <stdint.h>
#include <string>
#include <vector>

namespace ns3 {

namespace tests {}

#define ASSERT_ON_FAILURE                                                      \
  do {                                                                         \
    if (MustAssertOnFailure()) {                                               \
      *(volatile int *)0 = 0;                                                  \
    }                                                                          \
  } while (false)

#define CONTINUE_ON_FAILURE                                                    \
  do {                                                                         \
    if (!MustContinueOnFailure()) {                                            \
      return;                                                                  \
    }                                                                          \
  } while (false)

#define CONTINUE_ON_FAILURE_RETURNS_BOOL                                       \
  do {                                                                         \
    if (!MustContinueOnFailure()) {                                            \
      return IsStatusFailure();                                                \
    }                                                                          \
  } while (false)

#define NS_TEST_ASSERT_MSG_EQ(actual, limit, msg)                              \
  do {                                                                         \
    if (!((actual) == (limit))) {                                              \
      ASSERT_ON_FAILURE;                                                       \
      std::ostringstream msgStream;                                            \
      msgStream << msg;                                                        \
      std::ostringstream actualStream;                                         \
      actualStream << actual;                                                  \
      std::ostringstream limitStream;                                          \
      limitStream << limit;                                                    \
      ReportTestFailure(std::string(#actual) + " (actual) == " +               \
                            std::string(#limit) + " (limit)",                  \
                        actualStream.str(), limitStream.str(),                 \
                        msgStream.str(), __FILE__, __LINE__);                  \
      CONTINUE_ON_FAILURE;                                                     \
    }                                                                          \
  } while (false)

#define NS_TEST_ASSERT_MSG_EQ_RETURNS_BOOL(actual, limit, msg)                 \
  do {                                                                         \
    if (!((actual) == (limit))) {                                              \
      ASSERT_ON_FAILURE;                                                       \
      std::ostringstream msgStream;                                            \
      msgStream << msg;                                                        \
      std::ostringstream actualStream;                                         \
      actualStream << actual;                                                  \
      std::ostringstream limitStream;                                          \
      limitStream << limit;                                                    \
      ReportTestFailure(std::string(#actual) + " (actual) == " +               \
                            std::string(#limit) + " (limit)",                  \
                        actualStream.str(), limitStream.str(),                 \
                        msgStream.str(), __FILE__, __LINE__);                  \
      CONTINUE_ON_FAILURE_RETURNS_BOOL;                                        \
    }                                                                          \
  } while (false)

#define NS_TEST_EXPECT_MSG_EQ(actual, limit, msg)                              \
  do {                                                                         \
    if (!((actual) == (limit))) {                                              \
      ASSERT_ON_FAILURE;                                                       \
      std::ostringstream msgStream;                                            \
      msgStream << msg;                                                        \
      std::ostringstream actualStream;                                         \
      actualStream << actual;                                                  \
      std::ostringstream limitStream;                                          \
      limitStream << limit;                                                    \
      ReportTestFailure(std::string(#actual) + " (actual) == " +               \
                            std::string(#limit) + " (limit)",                  \
                        actualStream.str(), limitStream.str(),                 \
                        msgStream.str(), __FILE__, __LINE__);                  \
    }                                                                          \
  } while (false)

#define NS_TEST_ASSERT_MSG_EQ_TOL(actual, limit, tol, msg)                     \
  do {                                                                         \
    if ((actual) > (limit) + (tol) || (actual) < (limit) - (tol)) {            \
      ASSERT_ON_FAILURE;                                                       \
      std::ostringstream msgStream;                                            \
      msgStream << msg;                                                        \
      std::ostringstream actualStream;                                         \
      actualStream << actual;                                                  \
      std::ostringstream limitStream;                                          \
      limitStream << limit << " +- " << tol;                                   \
      std::ostringstream condStream;                                           \
      condStream << #actual << " (actual) < " << #limit << " (limit) + "       \
                 << #tol << " (tol) && " << #actual << " (actual) > "          \
                 << #limit << " (limit) - " << #tol << " (tol)";               \
      ReportTestFailure(condStream.str(), actualStream.str(),                  \
                        limitStream.str(), msgStream.str(), __FILE__,          \
                        __LINE__);                                             \
      CONTINUE_ON_FAILURE;                                                     \
    }                                                                          \
  } while (false)

#define NS_TEST_ASSERT_MSG_EQ_TOL_RETURNS_BOOL(actual, limit, tol, msg)        \
  do {                                                                         \
    if ((actual) > (limit) + (tol) || (actual) < (limit) - (tol)) {            \
      ASSERT_ON_FAILURE;                                                       \
      std::ostringstream msgStream;                                            \
      msgStream << msg;                                                        \
      std::ostringstream actualStream;                                         \
      actualStream << actual;                                                  \
      std::ostringstream limitStream;                                          \
      limitStream << limit << " +- " << tol;                                   \
      std::ostringstream condStream;                                           \
      condStream << #actual << " (actual) < " << #limit << " (limit) + "       \
                 << #tol << " (tol) && " << #actual << " (actual) > "          \
                 << #limit << " (limit) - " << #tol << " (tol)";               \
      ReportTestFailure(condStream.str(), actualStream.str(),                  \
                        limitStream.str(), msgStream.str(), __FILE__,          \
                        __LINE__);                                             \
      CONTINUE_ON_FAILURE_RETURNS_BOOL;                                        \
    }                                                                          \
  } while (false)

#define NS_TEST_EXPECT_MSG_EQ_TOL(actual, limit, tol, msg)                     \
  do {                                                                         \
    if ((actual) > (limit) + (tol) || (actual) < (limit) - (tol)) {            \
      ASSERT_ON_FAILURE;                                                       \
      std::ostringstream msgStream;                                            \
      msgStream << msg;                                                        \
      std::ostringstream actualStream;                                         \
      actualStream << actual;                                                  \
      std::ostringstream limitStream;                                          \
      limitStream << limit << " +- " << tol;                                   \
      std::ostringstream condStream;                                           \
      condStream << #actual << " (actual) < " << #limit << " (limit) + "       \
                 << #tol << " (tol) && " << #actual << " (actual) > "          \
                 << #limit << " (limit) - " << #tol << " (tol)";               \
      ReportTestFailure(condStream.str(), actualStream.str(),                  \
                        limitStream.str(), msgStream.str(), __FILE__,          \
                        __LINE__);                                             \
    }                                                                          \
  } while (false)

#define NS_TEST_ASSERT_MSG_NE(actual, limit, msg)                              \
  do {                                                                         \
    if (!((actual) != (limit))) {                                              \
      ASSERT_ON_FAILURE;                                                       \
      std::ostringstream msgStream;                                            \
      msgStream << msg;                                                        \
      std::ostringstream actualStream;                                         \
      actualStream << actual;                                                  \
      std::ostringstream limitStream;                                          \
      limitStream << limit;                                                    \
      ReportTestFailure(std::string(#actual) + " (actual) != " +               \
                            std::string(#limit) + " (limit)",                  \
                        actualStream.str(), limitStream.str(),                 \
                        msgStream.str(), __FILE__, __LINE__);                  \
      CONTINUE_ON_FAILURE;                                                     \
    }                                                                          \
  } while (false)

#define NS_TEST_ASSERT_MSG_NE_RETURNS_BOOL(actual, limit, msg)                 \
  do {                                                                         \
    if (!((actual) != (limit))) {                                              \
      ASSERT_ON_FAILURE;                                                       \
      std::ostringstream msgStream;                                            \
      msgStream << msg;                                                        \
      std::ostringstream actualStream;                                         \
      actualStream << actual;                                                  \
      std::ostringstream limitStream;                                          \
      limitStream << limit;                                                    \
      ReportTestFailure(std::string(#actual) + " (actual) != " +               \
                            std::string(#limit) + " (limit)",                  \
                        actualStream.str(), limitStream.str(),                 \
                        msgStream.str(), __FILE__, __LINE__);                  \
      CONTINUE_ON_FAILURE_RETURNS_BOOL;                                        \
    }                                                                          \
  } while (false)

#define NS_TEST_EXPECT_MSG_NE(actual, limit, msg)                              \
  do {                                                                         \
    if (!((actual) != (limit))) {                                              \
      ASSERT_ON_FAILURE;                                                       \
      std::ostringstream msgStream;                                            \
      msgStream << msg;                                                        \
      std::ostringstream actualStream;                                         \
      actualStream << actual;                                                  \
      std::ostringstream limitStream;                                          \
      limitStream << limit;                                                    \
      ReportTestFailure(std::string(#actual) + " (actual) != " +               \
                            std::string(#limit) + " (limit)",                  \
                        actualStream.str(), limitStream.str(),                 \
                        msgStream.str(), __FILE__, __LINE__);                  \
    }                                                                          \
  } while (false)

#define NS_TEST_ASSERT_MSG_LT(actual, limit, msg)                              \
  do {                                                                         \
    if (!((actual) < (limit))) {                                               \
      ASSERT_ON_FAILURE;                                                       \
      std::ostringstream msgStream;                                            \
      msgStream << msg;                                                        \
      std::ostringstream actualStream;                                         \
      actualStream << actual;                                                  \
      std::ostringstream limitStream;                                          \
      limitStream << limit;                                                    \
      ReportTestFailure(std::string(#actual) + " (actual) < " +                \
                            std::string(#limit) + " (limit)",                  \
                        actualStream.str(), limitStream.str(),                 \
                        msgStream.str(), __FILE__, __LINE__);                  \
      CONTINUE_ON_FAILURE;                                                     \
    }                                                                          \
  } while (false)

#define NS_TEST_ASSERT_MSG_LT_OR_EQ(actual, limit, msg)                        \
  do {                                                                         \
    if (!((actual) <= (limit))) {                                              \
      ASSERT_ON_FAILURE;                                                       \
      std::ostringstream msgStream;                                            \
      msgStream << msg;                                                        \
      std::ostringstream actualStream;                                         \
      actualStream << actual;                                                  \
      std::ostringstream limitStream;                                          \
      limitStream << limit;                                                    \
      ReportTestFailure(std::string(#actual) + " (actual) < " +                \
                            std::string(#limit) + " (limit)",                  \
                        actualStream.str(), limitStream.str(),                 \
                        msgStream.str(), __FILE__, __LINE__);                  \
      CONTINUE_ON_FAILURE;                                                     \
    }                                                                          \
  } while (false)

#define NS_TEST_EXPECT_MSG_LT(actual, limit, msg)                              \
  do {                                                                         \
    if (!((actual) < (limit))) {                                               \
      ASSERT_ON_FAILURE;                                                       \
      std::ostringstream msgStream;                                            \
      msgStream << msg;                                                        \
      std::ostringstream actualStream;                                         \
      actualStream << actual;                                                  \
      std::ostringstream limitStream;                                          \
      limitStream << limit;                                                    \
      ReportTestFailure(std::string(#actual) + " (actual) < " +                \
                            std::string(#limit) + " (limit)",                  \
                        actualStream.str(), limitStream.str(),                 \
                        msgStream.str(), __FILE__, __LINE__);                  \
    }                                                                          \
  } while (false)

#define NS_TEST_EXPECT_MSG_LT_OR_EQ(actual, limit, msg)                        \
  do {                                                                         \
    if (!((actual) <= (limit))) {                                              \
      ASSERT_ON_FAILURE;                                                       \
      std::ostringstream msgStream;                                            \
      msgStream << msg;                                                        \
      std::ostringstream actualStream;                                         \
      actualStream << actual;                                                  \
      std::ostringstream limitStream;                                          \
      limitStream << limit;                                                    \
      ReportTestFailure(std::string(#actual) + " (actual) < " +                \
                            std::string(#limit) + " (limit)",                  \
                        actualStream.str(), limitStream.str(),                 \
                        msgStream.str(), __FILE__, __LINE__);                  \
    }                                                                          \
  } while (false)

#define NS_TEST_ASSERT_MSG_GT(actual, limit, msg)                              \
  do {                                                                         \
    if (!((actual) > (limit))) {                                               \
      ASSERT_ON_FAILURE;                                                       \
      std::ostringstream msgStream;                                            \
      msgStream << msg;                                                        \
      std::ostringstream actualStream;                                         \
      actualStream << actual;                                                  \
      std::ostringstream limitStream;                                          \
      limitStream << limit;                                                    \
      ReportTestFailure(std::string(#actual) + " (actual) > " +                \
                            std::string(#limit) + " (limit)",                  \
                        actualStream.str(), limitStream.str(),                 \
                        msgStream.str(), __FILE__, __LINE__);                  \
      CONTINUE_ON_FAILURE;                                                     \
    }                                                                          \
  } while (false)

#define NS_TEST_ASSERT_MSG_GT_OR_EQ(actual, limit, msg)                        \
  do {                                                                         \
    if (!((actual) >= (limit))) {                                              \
      ASSERT_ON_FAILURE;                                                       \
      std::ostringstream msgStream;                                            \
      msgStream << msg;                                                        \
      std::ostringstream actualStream;                                         \
      actualStream << actual;                                                  \
      std::ostringstream limitStream;                                          \
      limitStream << limit;                                                    \
      ReportTestFailure(std::string(#actual) + " (actual) > " +                \
                            std::string(#limit) + " (limit)",                  \
                        actualStream.str(), limitStream.str(),                 \
                        msgStream.str(), __FILE__, __LINE__);                  \
      CONTINUE_ON_FAILURE;                                                     \
    }                                                                          \
  } while (false)

#define NS_TEST_EXPECT_MSG_GT(actual, limit, msg)                              \
  do {                                                                         \
    if (!((actual) > (limit))) {                                               \
      ASSERT_ON_FAILURE;                                                       \
      std::ostringstream msgStream;                                            \
      msgStream << msg;                                                        \
      std::ostringstream actualStream;                                         \
      actualStream << actual;                                                  \
      std::ostringstream limitStream;                                          \
      limitStream << limit;                                                    \
      ReportTestFailure(std::string(#actual) + " (actual) > " +                \
                            std::string(#limit) + " (limit)",                  \
                        actualStream.str(), limitStream.str(),                 \
                        msgStream.str(), __FILE__, __LINE__);                  \
    }                                                                          \
  } while (false)

#define NS_TEST_EXPECT_MSG_GT_OR_EQ(actual, limit, msg)                        \
  do {                                                                         \
    if (!((actual) >= (limit))) {                                              \
      ASSERT_ON_FAILURE;                                                       \
      std::ostringstream msgStream;                                            \
      msgStream << msg;                                                        \
      std::ostringstream actualStream;                                         \
      actualStream << actual;                                                  \
      std::ostringstream limitStream;                                          \
      limitStream << limit;                                                    \
      ReportTestFailure(std::string(#actual) + " (actual) > " +                \
                            std::string(#limit) + " (limit)",                  \
                        actualStream.str(), limitStream.str(),                 \
                        msgStream.str(), __FILE__, __LINE__);                  \
    }                                                                          \
  } while (false)

bool TestDoubleIsEqual(
    const double a, const double b,
    const double epsilon = std::numeric_limits<double>::epsilon());

class TestRunnerImpl;

class TestCase {
public:
  enum TestDuration { QUICK = 1, EXTENSIVE = 2, TAKES_FOREVER = 3 };

  virtual ~TestCase();

  TestCase(const TestCase &) = delete;
  TestCase &operator=(const TestCase &) = delete;

  std::string GetName() const;

protected:
  TestCase(std::string name);

  void AddTestCase(TestCase *testCase, TestDuration duration = QUICK);

  void SetDataDir(std::string directory);

  bool IsStatusFailure() const;
  bool IsStatusSuccess() const;

  TestCase *GetParent() const;

  void ReportTestFailure(std::string cond, std::string actual,
                         std::string limit, std::string message,
                         std::string file, int32_t line);
  bool MustAssertOnFailure() const;
  bool MustContinueOnFailure() const;
  std::string CreateDataDirFilename(std::string filename);
  std::string CreateTempDirFilename(std::string filename);

private:
  friend class TestRunnerImpl;

  virtual void DoSetup();

  virtual void DoRun() = 0;

  virtual void DoTeardown();

  void Run(TestRunnerImpl *runner);
  bool IsFailed() const;

  struct Result;

  TestCase *m_parent;
  std::vector<TestCase *> m_children;
  std::string m_dataDir;
  TestRunnerImpl *m_runner;
  Result *m_result;
  std::string m_name;
  TestDuration m_duration;
};

class TestSuite : public TestCase {
public:
  enum Type { ALL = 0, UNIT, SYSTEM, EXAMPLE, PERFORMANCE };

  TestSuite(std::string name, Type type = UNIT);

  TestSuite::Type GetTestType();

private:
  void DoRun() override;

  TestSuite::Type m_type;
};

class TestRunner {
public:
  static int Run(int argc, char *argv[]);
};

template <typename T> class TestVectors {
public:
  TestVectors();
  virtual ~TestVectors();

  TestVectors(const TestVectors &) = delete;
  TestVectors &operator=(const TestVectors &) = delete;

  void Reserve(uint32_t reserve);

  std::size_t Add(T vector);

  std::size_t GetN() const;
  T Get(std::size_t i) const;

private:
  typedef std::vector<T> TestVector;
  TestVector m_vectors;
};

template <typename T> TestVectors<T>::TestVectors() : m_vectors() {}

template <typename T> void TestVectors<T>::Reserve(uint32_t reserve) {
  m_vectors.reserve(reserve);
}

template <typename T> TestVectors<T>::~TestVectors() {}

template <typename T> std::size_t TestVectors<T>::Add(T vector) {
  std::size_t index = m_vectors.size();
  m_vectors.push_back(vector);
  return index;
}

template <typename T> std::size_t TestVectors<T>::GetN() const {
  return m_vectors.size();
}

template <typename T> T TestVectors<T>::Get(std::size_t i) const {
  NS_ABORT_MSG_UNLESS(m_vectors.size() > i, "TestVectors::Get(): Bad index");
  return m_vectors[i];
}

} // namespace ns3

#endif
