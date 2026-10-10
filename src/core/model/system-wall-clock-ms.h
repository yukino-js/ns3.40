
#ifndef SYSTEM_WALL_CLOCK_MS_H
#define SYSTEM_WALL_CLOCK_MS_H

#include <stdint.h>

namespace ns3 {

class SystemWallClockMs {
public:
  SystemWallClockMs();
  ~SystemWallClockMs();

  void Start();
  int64_t End();

  int64_t GetElapsedReal() const;
  int64_t GetElapsedUser() const;
  int64_t GetElapsedSystem() const;

private:
  class SystemWallClockMsPrivate *m_priv;
};

} // namespace ns3

#endif
