
#ifndef SYSTEM_WALL_CLOCK_TIMESTAMP_H
#define SYSTEM_WALL_CLOCK_TIMESTAMP_H

#include <ctime>
#include <string>

namespace ns3 {

class SystemWallClockTimestamp {
public:
  SystemWallClockTimestamp();

  void Stamp();

  std::string ToString() const;

  std::time_t GetLast() const;

  std::time_t GetInterval() const;

private:
  std::time_t m_last;

  std::time_t m_diff;
};

} // namespace ns3

#endif
