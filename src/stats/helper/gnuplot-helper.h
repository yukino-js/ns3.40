
#ifndef GNUPLOT_HELPER_H
#define GNUPLOT_HELPER_H

#include "ns3/gnuplot-aggregator.h"
#include "ns3/object-factory.h"
#include "ns3/probe.h"
#include "ns3/ptr.h"
#include "ns3/time-series-adaptor.h"

#include <map>
#include <string>
#include <utility>

namespace ns3 {

class GnuplotHelper {
public:
  GnuplotHelper();

  GnuplotHelper(const std::string &outputFileNameWithoutExtension,
                const std::string &title, const std::string &xLegend,
                const std::string &yLegend,
                const std::string &terminalType = "png");

  virtual ~GnuplotHelper();

  void ConfigurePlot(const std::string &outputFileNameWithoutExtension,
                     const std::string &title, const std::string &xLegend,
                     const std::string &yLegend,
                     const std::string &terminalType = "png");

  void PlotProbe(const std::string &typeId, const std::string &path,
                 const std::string &probeTraceSource, const std::string &title,
                 GnuplotAggregator::KeyLocation keyLocation =
                     GnuplotAggregator::KEY_INSIDE);

  void AddTimeSeriesAdaptor(const std::string &adaptorName);

  Ptr<Probe> GetProbe(std::string probeName) const;

  Ptr<GnuplotAggregator> GetAggregator();

private:
  void AddProbe(const std::string &typeId, const std::string &probeName,
                const std::string &path);

  void ConstructAggregator();

  void ConnectProbeToAggregator(const std::string &typeId,
                                const std::string &matchIdentifier,
                                const std::string &path,
                                const std::string &probeTraceSource,
                                const std::string &title);

  ObjectFactory m_factory;

  Ptr<GnuplotAggregator> m_aggregator;

  std::map<std::string, std::pair<Ptr<Probe>, std::string>> m_probeMap;

  std::map<std::string, Ptr<TimeSeriesAdaptor>> m_timeSeriesAdaptorMap;

  uint32_t m_plotProbeCount;

  std::string m_outputFileNameWithoutExtension;

  std::string m_title;

  std::string m_xLegend;

  std::string m_yLegend;

  std::string m_terminalType;
};

} // namespace ns3

#endif
