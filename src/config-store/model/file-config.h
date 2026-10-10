
#ifndef FILE_CONFIG_H
#define FILE_CONFIG_H

#include <string>

namespace ns3 {

class FileConfig {
public:
  virtual ~FileConfig();
  virtual void SetFilename(std::string filename) = 0;
  void SetSaveDeprecated(bool saveDeprecated);
  virtual void Default() = 0;
  virtual void Global() = 0;
  virtual void Attributes() = 0;

protected:
  bool m_saveDeprecated;
};

class NoneFileConfig : public FileConfig {
public:
  NoneFileConfig();
  ~NoneFileConfig() override;
  void SetFilename(std::string filename) override;
  void Default() override;
  void Global() override;
  void Attributes() override;
};

} // namespace ns3

#endif
