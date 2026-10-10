
#ifndef DESMETRICS_H
#define DESMETRICS_H

#include "nstime.h"
#include "singleton.h"

#include <fstream>
#include <mutex>
#include <stdint.h>
#include <string>
#include <vector>

namespace ns3 {

class DesMetrics : public Singleton<DesMetrics> {
public:
  void Initialize(std::vector<std::string> args, std::string outDir = "");

  void Trace(const Time &now, const Time &delay);

  void TraceWithContext(uint32_t context, const Time &now, const Time &delay);

  ~DesMetrics() override;

private:
  void Close();

  static std::string m_outputDir;

  bool m_initialized;
  std::ofstream m_os;
  char m_separator;

  std::mutex m_mutex;
};

} // namespace ns3

#endif
