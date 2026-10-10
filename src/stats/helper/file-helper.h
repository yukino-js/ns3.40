
#ifndef FILE_HELPER_H
#define FILE_HELPER_H

#include "ns3/file-aggregator.h"
#include "ns3/object-factory.h"
#include "ns3/probe.h"
#include "ns3/ptr.h"
#include "ns3/time-series-adaptor.h"

#include <map>
#include <string>

namespace ns3 {

class FileHelper {
public:
  FileHelper();

  FileHelper(
      const std::string &outputFileNameWithoutExtension,
      FileAggregator::FileType fileType = FileAggregator::SPACE_SEPARATED);

  virtual ~FileHelper();

  void ConfigureFile(
      const std::string &outputFileNameWithoutExtension,
      FileAggregator::FileType fileType = FileAggregator::SPACE_SEPARATED);

  void WriteProbe(const std::string &typeId, const std::string &path,
                  const std::string &probeTraceSource);

  void AddTimeSeriesAdaptor(const std::string &adaptorName);

  void AddAggregator(const std::string &aggregatorName,
                     const std::string &outputFileName, bool onlyOneAggregator);

  Ptr<Probe> GetProbe(std::string probeName) const;

  Ptr<FileAggregator> GetAggregatorSingle();

  Ptr<FileAggregator> GetAggregatorMultiple(const std::string &aggregatorName,
                                            const std::string &outputFileName);

  void SetHeading(const std::string &heading);

  void Set1dFormat(const std::string &format);

  void Set2dFormat(const std::string &format);

  void Set3dFormat(const std::string &format);

  void Set4dFormat(const std::string &format);

  void Set5dFormat(const std::string &format);

  void Set6dFormat(const std::string &format);

  void Set7dFormat(const std::string &format);

  void Set8dFormat(const std::string &format);

  void Set9dFormat(const std::string &format);

  void Set10dFormat(const std::string &format);

private:
  void AddProbe(const std::string &typeId, const std::string &probeName,
                const std::string &path);

  void ConnectProbeToAggregator(
      const std::string &typeId, const std::string &matchIdentifier,
      const std::string &path, const std::string &probeTraceSource,
      const std::string &outputFileNameWithoutExtension,
      bool onlyOneAggregator);

  ObjectFactory m_factory;

  Ptr<FileAggregator> m_aggregator;

  std::map<std::string, Ptr<FileAggregator>> m_aggregatorMap;

  std::map<std::string, std::pair<Ptr<Probe>, std::string>> m_probeMap;

  std::map<std::string, Ptr<TimeSeriesAdaptor>> m_timeSeriesAdaptorMap;

  uint32_t m_fileProbeCount;

  FileAggregator::FileType m_fileType;

  std::string m_outputFileNameWithoutExtension;

  bool m_hasHeadingBeenSet;

  std::string m_heading;

  std::string m_1dFormat;
  std::string m_2dFormat;
  std::string m_3dFormat;
  std::string m_4dFormat;
  std::string m_5dFormat;
  std::string m_6dFormat;
  std::string m_7dFormat;
  std::string m_8dFormat;
  std::string m_9dFormat;
  std::string m_10dFormat;
};

} // namespace ns3

#endif
