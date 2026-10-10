
#include "file-config.h"

namespace ns3 {

FileConfig::~FileConfig() {}

void FileConfig::SetSaveDeprecated(bool saveDeprecated) {
  m_saveDeprecated = saveDeprecated;
}

NoneFileConfig::NoneFileConfig() {}

NoneFileConfig::~NoneFileConfig() {}

void NoneFileConfig::SetFilename(std::string filename) {}

void NoneFileConfig::Default() {}

void NoneFileConfig::Global() {}

void NoneFileConfig::Attributes() {}

} // namespace ns3
