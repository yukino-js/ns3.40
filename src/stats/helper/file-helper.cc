
#include "file-helper.h"

#include "ns3/abort.h"
#include "ns3/config.h"
#include "ns3/get-wildcard-matches.h"
#include "ns3/log.h"

#include <fstream>
#include <iostream>
#include <string>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("FileHelper");

FileHelper::FileHelper()
    : m_aggregator(nullptr), m_fileProbeCount(0),
      m_fileType(FileAggregator::SPACE_SEPARATED),
      m_outputFileNameWithoutExtension("file-helper"),
      m_hasHeadingBeenSet(false) {
  NS_LOG_FUNCTION(this);
}

FileHelper::FileHelper(const std::string &outputFileNameWithoutExtension,
                       FileAggregator::FileType fileType)
    : m_aggregator(nullptr), m_fileProbeCount(0), m_fileType(fileType),
      m_outputFileNameWithoutExtension(outputFileNameWithoutExtension),
      m_hasHeadingBeenSet(false) {
  NS_LOG_FUNCTION(this);
}

FileHelper::~FileHelper() { NS_LOG_FUNCTION(this); }

void FileHelper::ConfigureFile(
    const std::string &outputFileNameWithoutExtension,
    FileAggregator::FileType fileType) {
  NS_LOG_FUNCTION(this << outputFileNameWithoutExtension << fileType);

  if (m_aggregator) {
    NS_LOG_WARN("An existing aggregator object "
                << m_aggregator
                << " may be destroyed if no references remain.");
  }

  m_fileType = fileType;
  m_outputFileNameWithoutExtension = outputFileNameWithoutExtension;
  m_hasHeadingBeenSet = false;
}

void FileHelper::WriteProbe(const std::string &typeId, const std::string &path,
                            const std::string &probeTraceSource) {
  NS_LOG_FUNCTION(this << typeId << path << probeTraceSource);

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

  Config::MatchContainer matches = Config::LookupMatches(pathWithoutLastToken);
  uint32_t matchCount = matches.GetN();

  std::string matchIdentifier;

  bool onlyOneAggregator;

  if (matchCount == 1 && pathHasNoWildcards) {
    matchIdentifier = "0";
    onlyOneAggregator = true;
    ConnectProbeToAggregator(typeId, matchIdentifier, path, probeTraceSource,
                             m_outputFileNameWithoutExtension,
                             onlyOneAggregator);
  } else if (matchCount > 0) {
    for (uint32_t i = 0; i < matchCount; i++) {
      std::ostringstream matchIdentifierStream;
      matchIdentifierStream << i;
      matchIdentifier = matchIdentifierStream.str();
      onlyOneAggregator = false;

      std::string wildcardSeparator = "-";
      std::string matchedPath = matches.GetMatchedPath(i) + lastToken;
      std::string wildcardMatches =
          GetWildcardMatches(path, matchedPath, wildcardSeparator);

      ConnectProbeToAggregator(
          typeId, matchIdentifier, matchedPath, probeTraceSource,
          m_outputFileNameWithoutExtension + "-" + wildcardMatches,
          onlyOneAggregator);
    }
  } else {
    NS_FATAL_ERROR("Lookup of " << path << " got no matches");
  }
}

void FileHelper::AddProbe(const std::string &typeId,
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

void FileHelper::AddTimeSeriesAdaptor(const std::string &adaptorName) {
  NS_LOG_FUNCTION(this << adaptorName);

  if (m_timeSeriesAdaptorMap.count(adaptorName) > 0) {
    NS_ABORT_MSG("That time series adaptor has already been added");
  }

  Ptr<TimeSeriesAdaptor> timeSeriesAdaptor = CreateObject<TimeSeriesAdaptor>();

  timeSeriesAdaptor->Enable();

  m_timeSeriesAdaptorMap[adaptorName] = timeSeriesAdaptor;
}

void FileHelper::AddAggregator(const std::string &aggregatorName,
                               const std::string &outputFileName,
                               bool onlyOneAggregator) {
  NS_LOG_FUNCTION(this << aggregatorName << outputFileName
                       << onlyOneAggregator);

  if (m_aggregatorMap.count(aggregatorName) > 0) {
    NS_ABORT_MSG("That file aggregator has already been added");
  }

  if (onlyOneAggregator) {
    Ptr<FileAggregator> singleAggregator = GetAggregatorSingle();

    m_aggregatorMap[aggregatorName] = singleAggregator;
    return;
  }

  Ptr<FileAggregator> multipleAggregator =
      CreateObject<FileAggregator>(outputFileName, m_fileType);

  multipleAggregator->Set1dFormat(m_1dFormat);
  multipleAggregator->Set2dFormat(m_2dFormat);
  multipleAggregator->Set3dFormat(m_3dFormat);
  multipleAggregator->Set4dFormat(m_4dFormat);
  multipleAggregator->Set5dFormat(m_5dFormat);
  multipleAggregator->Set6dFormat(m_6dFormat);
  multipleAggregator->Set7dFormat(m_7dFormat);
  multipleAggregator->Set8dFormat(m_8dFormat);
  multipleAggregator->Set9dFormat(m_9dFormat);
  multipleAggregator->Set10dFormat(m_10dFormat);

  multipleAggregator->SetHeading(m_heading);

  multipleAggregator->Enable();

  m_aggregatorMap[aggregatorName] = multipleAggregator;
}

Ptr<Probe> FileHelper::GetProbe(std::string probeName) const {
  NS_LOG_FUNCTION(this << probeName);

  auto mapIterator = m_probeMap.find(probeName);

  if (mapIterator != m_probeMap.end()) {
    return mapIterator->second.first;
  } else {
    NS_ABORT_MSG("That probe has not been added");
  }
}

Ptr<FileAggregator> FileHelper::GetAggregatorSingle() {
  NS_LOG_FUNCTION(this);

  if (!m_aggregator) {
    std::string outputFileName = m_outputFileNameWithoutExtension + ".txt";
    m_aggregator = CreateObject<FileAggregator>(outputFileName, m_fileType);

    m_aggregator->Set1dFormat(m_1dFormat);
    m_aggregator->Set2dFormat(m_2dFormat);
    m_aggregator->Set3dFormat(m_3dFormat);
    m_aggregator->Set4dFormat(m_4dFormat);
    m_aggregator->Set5dFormat(m_5dFormat);
    m_aggregator->Set6dFormat(m_6dFormat);
    m_aggregator->Set7dFormat(m_7dFormat);
    m_aggregator->Set8dFormat(m_8dFormat);
    m_aggregator->Set9dFormat(m_9dFormat);
    m_aggregator->Set10dFormat(m_10dFormat);

    m_aggregator->SetHeading(m_heading);

    m_aggregator->Enable();
  }
  return m_aggregator;
}

Ptr<FileAggregator>
FileHelper::GetAggregatorMultiple(const std::string &aggregatorName,
                                  const std::string &outputFileName) {
  NS_LOG_FUNCTION(this);

  if (m_aggregatorMap.count(aggregatorName) > 0) {
    return m_aggregatorMap[aggregatorName];
  }

  bool onlyOneAggregator = false;
  AddAggregator(aggregatorName, outputFileName, onlyOneAggregator);

  return m_aggregatorMap[aggregatorName];
}

void FileHelper::SetHeading(const std::string &heading) {
  NS_LOG_FUNCTION(this << heading);

  m_hasHeadingBeenSet = true;
  m_heading = heading;
}

void FileHelper::Set1dFormat(const std::string &format) {
  NS_LOG_FUNCTION(this << format);

  m_1dFormat = format;
}

void FileHelper::Set2dFormat(const std::string &format) {
  NS_LOG_FUNCTION(this << format);

  m_2dFormat = format;
}

void FileHelper::Set3dFormat(const std::string &format) {
  NS_LOG_FUNCTION(this << format);

  m_3dFormat = format;
}

void FileHelper::Set4dFormat(const std::string &format) {
  NS_LOG_FUNCTION(this << format);

  m_4dFormat = format;
}

void FileHelper::Set5dFormat(const std::string &format) {
  NS_LOG_FUNCTION(this << format);

  m_5dFormat = format;
}

void FileHelper::Set6dFormat(const std::string &format) {
  NS_LOG_FUNCTION(this << format);

  m_6dFormat = format;
}

void FileHelper::Set7dFormat(const std::string &format) {
  NS_LOG_FUNCTION(this << format);

  m_7dFormat = format;
}

void FileHelper::Set8dFormat(const std::string &format) {
  NS_LOG_FUNCTION(this << format);

  m_8dFormat = format;
}

void FileHelper::Set9dFormat(const std::string &format) {
  NS_LOG_FUNCTION(this << format);

  m_9dFormat = format;
}

void FileHelper::Set10dFormat(const std::string &format) {
  NS_LOG_FUNCTION(this << format);

  m_10dFormat = format;
}

void FileHelper::ConnectProbeToAggregator(
    const std::string &typeId, const std::string &matchIdentifier,
    const std::string &path, const std::string &probeTraceSource,
    const std::string &outputFileNameWithoutExtension, bool onlyOneAggregator) {
  NS_LOG_FUNCTION(this << typeId << matchIdentifier << path << probeTraceSource
                       << outputFileNameWithoutExtension << onlyOneAggregator);

  m_fileProbeCount++;

  std::ostringstream probeNameStream;
  probeNameStream << "FileProbe-" << m_fileProbeCount;
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

  std::string outputFileName = outputFileNameWithoutExtension + ".txt";
  AddAggregator(probeContext, outputFileName, onlyOneAggregator);

  std::string adaptorTraceSource = "Output";
  m_timeSeriesAdaptorMap[probeContext]->TraceConnect(
      adaptorTraceSource, probeContext,
      MakeCallback(&FileAggregator::Write2d, m_aggregatorMap[probeContext]));
}

} // namespace ns3
