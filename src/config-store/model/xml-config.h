
#ifndef XML_CONFIG_STORE_H
#define XML_CONFIG_STORE_H

#include "file-config.h"

#include <string>

typedef struct _xmlTextWriter xmlTextWriter;
typedef xmlTextWriter *xmlTextWriterPtr;

namespace ns3 {

class XmlConfigSave : public FileConfig {
public:
  XmlConfigSave();
  ~XmlConfigSave() override;

  void SetFilename(std::string filename) override;
  void Default() override;
  void Global() override;
  void Attributes() override;

private:
  xmlTextWriterPtr m_writer;
};

class XmlConfigLoad : public FileConfig {
public:
  XmlConfigLoad();
  ~XmlConfigLoad() override;

  void SetFilename(std::string filename) override;
  void Default() override;
  void Global() override;
  void Attributes() override;

private:
  std::string m_filename;
};

} // namespace ns3

#endif
