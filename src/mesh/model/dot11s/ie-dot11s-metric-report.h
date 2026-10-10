
#ifndef METRIC_REPORT_H
#define METRIC_REPORT_H

#include "ns3/buffer.h"
#include "ns3/mesh-information-element-vector.h"

#include <stdint.h>

namespace ns3 {
namespace dot11s {
class IeLinkMetricReport : public WifiInformationElement {
public:
  IeLinkMetricReport();
  IeLinkMetricReport(uint32_t metric);
  void SetMetric(uint32_t metric);
  uint32_t GetMetric() const;

  WifiInformationElementId ElementId() const override;
  void SerializeInformationField(Buffer::Iterator i) const override;
  uint16_t DeserializeInformationField(Buffer::Iterator start,
                                       uint16_t length) override;
  void Print(std::ostream &os) const override;
  uint16_t GetInformationFieldSize() const override;

private:
  uint32_t m_metric;
  friend bool operator==(const IeLinkMetricReport &a,
                         const IeLinkMetricReport &b);
  friend bool operator>(const IeLinkMetricReport &a,
                        const IeLinkMetricReport &b);
  friend bool operator<(const IeLinkMetricReport &a,
                        const IeLinkMetricReport &b);
};

bool operator==(const IeLinkMetricReport &a, const IeLinkMetricReport &b);
bool operator>(const IeLinkMetricReport &a, const IeLinkMetricReport &b);
bool operator<(const IeLinkMetricReport &a, const IeLinkMetricReport &b);
std::ostream &operator<<(std::ostream &os,
                         const IeLinkMetricReport &linkMetricReport);
} // namespace dot11s
} // namespace ns3
#endif
