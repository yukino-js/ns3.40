
#include "ns3/example-as-test.h"

#include <sstream>

using namespace ns3;

class MpiTestCase : public ExampleAsTestCase {
public:
  MpiTestCase(const std::string name, const std::string program,
              const std::string dataDir, const int ranks,
              const std::string args = "", const bool shouldNotErr = true);

  ~MpiTestCase() override {}

  std::string GetCommandTemplate() const override;

  std::string GetPostProcessingCommand() const override;

private:
  int m_ranks;
};

MpiTestCase::MpiTestCase(const std::string name, const std::string program,
                         const std::string dataDir, const int ranks,
                         const std::string args, const bool shouldNotErr)
    : ExampleAsTestCase(name, program, dataDir, args, shouldNotErr),
      m_ranks(ranks) {}

std::string MpiTestCase::GetCommandTemplate() const {
  std::stringstream ss;
  ss << "mpiexec -n " << m_ranks << " %s --test " << m_args;
  return ss.str();
}

std::string MpiTestCase::GetPostProcessingCommand() const {
  std::string command("| grep TEST | sort ");
  return command;
}

class MpiTestSuite : public TestSuite {
public:
  MpiTestSuite(const std::string name, const std::string program,
               const std::string dataDir, const int ranks,
               const std::string args = "", const TestDuration duration = QUICK,
               const bool shouldNotErr = true)
      : TestSuite(name, EXAMPLE) {
    AddTestCase(
        new MpiTestCase(name, program, dataDir, ranks, args, shouldNotErr),
        duration);
  }
};

static MpiTestSuite g_mpiNms2("mpi-example-nms-2", "nms-p2p-nix-distributed",
                              NS_TEST_SOURCEDIR, 2);
static MpiTestSuite g_mpiComm2("mpi-example-comm-2",
                               "simple-distributed-mpi-comm", NS_TEST_SOURCEDIR,
                               2);
static MpiTestSuite g_mpiComm2comm("mpi-example-comm-2-init",
                                   "simple-distributed-mpi-comm",
                                   NS_TEST_SOURCEDIR, 2, "--init");
static MpiTestSuite g_mpiComm3comm("mpi-example-comm-3-init",
                                   "simple-distributed-mpi-comm",
                                   NS_TEST_SOURCEDIR, 3, "--init");
static MpiTestSuite g_mpiEmpty2("mpi-example-empty-2",
                                "simple-distributed-empty-node",
                                NS_TEST_SOURCEDIR, 2);
static MpiTestSuite g_mpiEmpty3("mpi-example-empty-3",
                                "simple-distributed-empty-node",
                                NS_TEST_SOURCEDIR, 3);
static MpiTestSuite g_mpiSimple2("mpi-example-simple-2", "simple-distributed",
                                 NS_TEST_SOURCEDIR, 2);
static MpiTestSuite g_mpiThird2("mpi-example-third-2", "third-distributed",
                                NS_TEST_SOURCEDIR, 2);

static MpiTestSuite g_mpiSimple2NullMsg("mpi-example-simple-2-nullmsg",
                                        "simple-distributed", NS_TEST_SOURCEDIR,
                                        2, "--nullmsg");
static MpiTestSuite g_mpiEmpty2NullMsg("mpi-example-empty-2-nullmsg",
                                       "simple-distributed-empty-node",
                                       NS_TEST_SOURCEDIR, 2, "-nullmsg");
static MpiTestSuite g_mpiEmpty3NullMsg("mpi-example-empty-3-nullmsg",
                                       "simple-distributed-empty-node",
                                       NS_TEST_SOURCEDIR, 3, "-nullmsg");
