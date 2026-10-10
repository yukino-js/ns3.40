
#ifndef NS3_HISTOGRAM_H
#define NS3_HISTOGRAM_H

#include <ostream>
#include <stdint.h>
#include <vector>

namespace ns3 {

class Histogram {
public:
  Histogram(double binWidth);
  Histogram();

  uint32_t GetNBins() const;
  double GetBinStart(uint32_t index) const;
  double GetBinEnd(uint32_t index) const;
  double GetBinWidth(uint32_t index) const;
  void SetDefaultBinWidth(double binWidth);
  uint32_t GetBinCount(uint32_t index) const;

  void AddValue(double value);

  void Clear();

  void SerializeToXmlStream(std::ostream &os, uint16_t indent,
                            std::string elementName) const;

private:
  std::vector<uint32_t> m_histogram;
  double m_binWidth;
};

} // namespace ns3

#endif
