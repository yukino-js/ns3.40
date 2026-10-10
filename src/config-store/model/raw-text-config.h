
#ifndef RAW_TEXT_CONFIG_H
#define RAW_TEXT_CONFIG_H

#include "file-config.h"

#include <fstream>
#include <string>

namespace ns3 {

class RawTextConfigSave : public FileConfig {
public:
  RawTextConfigSave();
  ~RawTextConfigSave() override;
  void SetFilename(std::string filename) override;
  void Default() override;
  void Global() override;
  void Attributes() override;

private:
  std::ofstream *m_os;
};

class RawTextConfigLoad : public FileConfig {
public:
  RawTextConfigLoad();
  ~RawTextConfigLoad() override;
  void SetFilename(std::string filename) override;
  void Default() override;
  void Global() override;
  void Attributes() override;

private:
  virtual bool ParseLine(const std::string &line, std::string &type,
                         std::string &name, std::string &value);

  std::string Strip(std::string value);
  std::ifstream *m_is;
};

} // namespace ns3

#endif
