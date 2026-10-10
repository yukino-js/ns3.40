
#ifndef TOPOLOGY_READER_HELPER_H
#define TOPOLOGY_READER_HELPER_H

#include "ns3/topology-reader.h"

#include <string>

namespace ns3 {

class TopologyReaderHelper {
public:
  TopologyReaderHelper();

  void SetFileName(const std::string fileName);

  void SetFileType(const std::string fileType);

  Ptr<TopologyReader> GetTopologyReader();

private:
  Ptr<TopologyReader> m_inputModel;
  std::string m_fileName;
  std::string m_fileType;
};

} // namespace ns3

#endif
