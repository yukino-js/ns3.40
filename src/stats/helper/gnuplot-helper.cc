
#include "gnuplot-helper.h"

#include "ns3/abort.h"
#include "ns3/assert.h"
#include "ns3/config.h"
#include "ns3/get-wildcard-matches.h"
#include "ns3/log.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("GnuplotHelper");

GnuplotHelper::GnuplotHelper()
    : m_aggregator(nullptr), m_plotProbeCount(0),
      m_outputFileNameWithoutExtension("gnuplot-helper"),
      m_title("Gnuplot Helper Plot"), m_xLegend("X Values"),
      m_yLegend("Y Values"), m_terminalType("png") {
  NS_LOG_FUNCTION(this);
}

GnuplotHelper::GnuplotHelper(const std::string &outputFileNameWithoutExtension,
                             const std::string &title,
                             const std::string &xLegend,
                             const std::string &yLegend,
                             const std::string &terminalType)
    : m_aggregator(nullptr), m_plotProbeCount(0),
      m_outputFileNameWithoutExtension(outputFileNameWithoutExtension),
      m_title(title), m_xLegend(xLegend), m_yLegend(yLegend),
      m_terminalType(terminalType) {
  NS_LOG_FUNCTION(this);

  ConstructAggregator();
}

GnuplotHelper::~GnuplotHelper() { NS_LOG_FUNCTION(this); }

void GnuplotHelper::ConfigurePlot(
    const std::string &outputFileNameWithoutExtension, const std::string &title,
    const std::string &xLegend, const std::string &yLegend,
    const std::string &terminalType) {
  NS_LOG_FUNCTION(this << outputFileNameWithoutExtension << title << xLegend
                       << yLegend << terminalType);

  if (m_aggregator) {
    NS_LOG_WARN("An existing aggregator object "
                << m_aggregator
                << " may be destroyed if no references remain.");
  }

  m_outputFileNameWithoutExtension = outputFileNameWithoutExtension;
  m_title = title;
  m_xLegend = xLegend;
  m_yLegend = yLegend;
  m_terminalType = terminalType;

  ConstructAggregator();
}

void GnuplotHelper::PlotProbe(const std::string &typeId,
                              const std::string &path,
                              const std::string &probeTraceSource,
                              const std::string &title,
                              GnuplotAggregator::KeyLocation keyLocation) {
  NS_LOG_FUNCTION(this << typeId << path << probeTraceSource << title
                       << keyLocation);

  Ptr<GnuplotAggregator> aggregator = GetAggregator();

  aggregator->SetTitle(m_title + " \\n\\nTrace Source Path: " + path);

  aggregator->Set2dDatasetDefaultStyle(Gnuplot2dDataset::LINES_POINTS);

  aggregator->SetKeyLocation(keyLocation);

  std::string pathWithoutLastToken;
  std::string lastToken;

  bool pathHasNoWildcards = path.find('*') == std::string::npos;

  size_t lastSlash = path.find_last_of('/');
  if (lastSlash == std::string::npos) {
    pathWithoutLastToken = path;
    lastToken = "";
  } else {
    pathWithoutLastToken = path.substr(0, lastSlash);

    lastToken = path.substr(lastSlash + 1, std::string::npos);
  }

  NS_LOG_DEBUG("Searching config database for trace source " << path);
  Config::MatchContainer matches = Config::LookupMatches(pathWithoutLastToken);
  uint32_t matchCount = matches.GetN();
  NS_LOG_DEBUG("Found " << matchCount << " matches for trace source " << path);

  std::string matchIdentifier;

  if (matchCount == 1 && pathHasNoWildcards) {
    matchIdentifier = "0";
    ConnectProbeToAggregator(typeId, matchIdentifier, path, probeTraceSource,
                             title);
  } else if (matchCount > 0) {
    for (uint32_t i = 0; i < matchCount; i++) {
      std::ostringstream matchIdentifierStream;
      matchIdentifierStream << i;
      matchIdentifier = matchIdentifierStream.str();

      std::string wildcardSeparator = " ";
      std::string matchedPath = matches.GetMatchedPath(i) + lastToken;
      std::string wildcardMatches =
          GetWildcardMatches(path, matchedPath, wildcardSeparator);

      ConnectProbeToAggregator(typeId, matchIdentifier, matchedPath,
                               probeTraceSource, title + "-" + wildcardMatches);
    }
  } else {
    NS_FATAL_ERROR("Lookup of " << path << " got no matches");
  }
}

void GnuplotHelper::AddProbe(const std::string &typeId,
                             const std::string &probeName,
                             const std::string &path) {
  NS_LOG_FUNCTION(this << typeId << probeName << path);

  if (m_probeMap.count(probeName) > 0) {
    NS_ABORT_MSG("That probe has already been added");
  }

  m_factory.SetTypeId(typeId);

  Ptr<Probe> probe = m_factory.Create()->GetObject<Probe>();
  if (!probe) {
    NS_ABORT_MSG("The requested type is not a probe");
  }

  probe->SetName(probeName);

  probe->ConnectByPath(path);

  probe->Enable();

  m_probeMap[probeName] = std::make_pair(probe, typeId);
}

void GnuplotHelper::AddTimeSeriesAdaptor(const std::string &adaptorName) {
  NS_LOG_FUNCTION(this << adaptorName);

  if (m_timeSeriesAdaptorMap.count(adaptorName) > 0) {
    NS_ABORT_MSG("That time series adaptor has already been added");
  }

  Ptr<TimeSeriesAdaptor> timeSeriesAdaptor = CreateObject<TimeSeriesAdaptor>();

  timeSeriesAdaptor->Enable();

  m_timeSeriesAdaptorMap[adaptorName] = timeSeriesAdaptor;
}

Ptr<Probe> GnuplotHelper::GetProbe(std::string probeName) const {
  auto mapIterator = m_probeMap.find(probeName);

  if (mapIterator != m_probeMap.end()) {
    return mapIterator->second.first;
  } else {
    NS_ABORT_MSG("That probe has not been added");
  }
}

Ptr<GnuplotAggregator> GnuplotHelper::GetAggregator() {
  NS_LOG_FUNCTION(this);

  if (!m_aggregator) {
    ConstructAggregator();
  }
  return m_aggregator;
}

void GnuplotHelper::ConstructAggregator() {
  NS_LOG_FUNCTION(this);

  m_aggregator =
      CreateObject<GnuplotAggregator>(m_outputFileNameWithoutExtension);

  m_aggregator->SetTerminal(m_terminalType);
  m_aggregator->SetTitle(m_title);
  m_aggregator->SetLegend(m_xLegend, m_yLegend);

  m_aggregator->Enable();
}

void GnuplotHelper::ConnectProbeToAggregator(
    const std::string &typeId, const std::string &matchIdentifier,
    const std::string &path, const std::string &probeTraceSource,
    const std::string &title) {
  NS_LOG_FUNCTION(this << typeId << matchIdentifier << path << probeTraceSource
                       << title);

  Ptr<GnuplotAggregator> aggregator = GetAggregator();

  m_plotProbeCount++;

  std::ostringstream probeNameStream;
  probeNameStream << "PlotProbe-" << m_plotProbeCount;
  std::string probeName = probeNameStream.str();

  std::string probeContext =
      probeName + "/" + matchIdentifier + "/" + probeTraceSource;

  AddProbe(typeId, probeName, path);

  AddTimeSeriesAdaptor(probeContext);

  if (m_probeMap[probeName].second == "ns3::DoubleProbe") {
    m_probeMap[probeName].first->TraceConnectWithoutContext(
        probeTraceSource, MakeCallback(&TimeSeriesAdaptor::TraceSinkDouble,
                                       m_timeSeriesAdaptorMap[probeContext]));
  } else if (m_probeMap[probeName].second == "ns3::BooleanProbe") {
    m_probeMap[probeName].first->TraceConnectWithoutContext(
        probeTraceSource, MakeCallback(&TimeSeriesAdaptor::TraceSinkBoolean,
                                       m_timeSeriesAdaptorMap[probeContext]));
  } else if (m_probeMap[probeName].second == "ns3::PacketProbe") {
    m_probeMap[probeName].first->TraceConnectWithoutContext(
        probeTraceSource, MakeCallback(&TimeSeriesAdaptor::TraceSinkUinteger32,
                                       m_timeSeriesAdaptorMap[probeContext]));
  } else if (m_probeMap[probeName].second == "ns3::ApplicationPacketProbe") {
    m_probeMap[probeName].first->TraceConnectWithoutContext(
        probeTraceSource, MakeCallback(&TimeSeriesAdaptor::TraceSinkUinteger32,
                                       m_timeSeriesAdaptorMap[probeContext]));
  } else if (m_probeMap[probeName].second == "ns3::Ipv4PacketProbe") {
    m_probeMap[probeName].first->TraceConnectWithoutContext(
        probeTraceSource, MakeCallback(&TimeSeriesAdaptor::TraceSinkUinteger32,
                                       m_timeSeriesAdaptorMap[probeContext]));
  } else if (m_probeMap[probeName].second == "ns3::Ipv6PacketProbe") {
    m_probeMap[probeName].first->TraceConnectWithoutContext(
        probeTraceSource, MakeCallback(&TimeSeriesAdaptor::TraceSinkUinteger32,
                                       m_timeSeriesAdaptorMap[probeContext]));
  } else if (m_probeMap[probeName].second == "ns3::Uinteger8Probe") {
    m_probeMap[probeName].first->TraceConnectWithoutContext(
        probeTraceSource, MakeCallback(&TimeSeriesAdaptor::TraceSinkUinteger8,
                                       m_timeSeriesAdaptorMap[probeContext]));
  } else if (m_probeMap[probeName].second == "ns3::Uinteger16Probe") {
    m_probeMap[probeName].first->TraceConnectWithoutContext(
        probeTraceSource, MakeCallback(&TimeSeriesAdaptor::TraceSinkUinteger16,
                                       m_timeSeriesAdaptorMap[probeContext]));
  } else if (m_probeMap[probeName].second == "ns3::Uinteger32Probe") {
    m_probeMap[probeName].first->TraceConnectWithoutContext(
        probeTraceSource, MakeCallback(&TimeSeriesAdaptor::TraceSinkUinteger32,
                                       m_timeSeriesAdaptorMap[probeContext]));
  } else if (m_probeMap[probeName].second == "ns3::TimeProbe") {
    m_probeMap[probeName].first->TraceConnectWithoutContext(
        probeTraceSource, MakeCallback(&TimeSeriesAdaptor::TraceSinkDouble,
                                       m_timeSeriesAdaptorMap[probeContext]));
  } else {
    NS_FATAL_ERROR("Unknown probe type "
                   << m_probeMap[probeName].second
                   << "; need to add support in the helper for this");
  }

  std::string adaptorTraceSource = "Output";
  m_timeSeriesAdaptorMap[probeContext]->TraceConnect(
      adaptorTraceSource, probeContext,
      MakeCallback(&GnuplotAggregator::Write2d, aggregator));

  aggregator->Add2dDataset(probeContext, title);
}

} // namespace ns3
