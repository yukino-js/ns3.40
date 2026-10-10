
#ifndef MPI_TEST_FIXTURES_H
#define MPI_TEST_FIXTURES_H

#include <iomanip>
#include <ios>
#include <sstream>

namespace ns3 {

template <typename T> class Ptr;
class Address;
class Packet;

#define RANK0COUT(x)                                                           \
  do                                                                           \
    if (SinkTracer::GetWorldRank() == 0) {                                     \
      std::cout << "TEST : ";                                                  \
      std::ios_base::fmtflags f(std::cout.flags());                            \
      std::cout << std::setfill('0') << std::setw(5)                           \
                << SinkTracer::GetLineCount();                                 \
      std::cout.flags(f);                                                      \
      std::cout << " : " << x;                                                 \
    }                                                                          \
  while (false)

#define RANK0COUTAPPEND(x)                                                     \
  do                                                                           \
    if (SinkTracer::GetWorldRank() == 0) {                                     \
      std::cout << x;                                                          \
    }                                                                          \
  while (false)

class SinkTracer {
public:
  static void Init();

  static void SinkTrace(const ns3::Ptr<const ns3::Packet> packet,
                        const ns3::Address &srcAddress,
                        const ns3::Address &destAddress);

  static void Verify(unsigned long expectedCount);

  static std::string FormatAddress(const ns3::Address address);

  static int GetWorldRank() { return m_worldRank; }

  static int GetWorldSize() { return m_worldSize; }

  static int GetLineCount() { return m_line++; }

private:
  static unsigned long m_sinkCount;
  static unsigned long m_line;
  static int m_worldRank;
  static int m_worldSize;
};

} // namespace ns3

#endif
