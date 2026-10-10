
#ifndef CONFIG_STORE_H
#define CONFIG_STORE_H

#include "file-config.h"

#include "ns3/object-base.h"

namespace ns3 {

class ConfigStore : public ObjectBase {
public:
  enum Mode { LOAD, SAVE, NONE };

  enum FileFormat { XML, RAW_TEXT };

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;

  ConfigStore();
  ~ConfigStore() override;

  void SetMode(Mode mode);
  void SetFileFormat(FileFormat format);
  void SetFilename(std::string filename);
  void SetSaveDeprecated(bool saveDeprecated);

  void ConfigureDefaults();
  void ConfigureAttributes();

private:
  Mode m_mode;
  FileFormat m_fileFormat;
  bool m_saveDeprecated;
  std::string m_filename;
  FileConfig *m_file;
};

std::ostream &operator<<(std::ostream &os, ConfigStore::Mode &mode);
std::ostream &operator<<(std::ostream &os, ConfigStore::FileFormat &format);

} // namespace ns3

#endif
