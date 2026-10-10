
#ifndef FILE_AGGREGATOR_H
#define FILE_AGGREGATOR_H

#include "data-collection-object.h"

#include <fstream>
#include <map>
#include <string>

namespace ns3 {

class FileAggregator : public DataCollectionObject {
public:
  enum FileType { FORMATTED, SPACE_SEPARATED, COMMA_SEPARATED, TAB_SEPARATED };

  static TypeId GetTypeId();

  FileAggregator(const std::string &outputFileName,
                 FileType fileType = SPACE_SEPARATED);

  ~FileAggregator() override;

  void SetFileType(FileType fileType);

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

  void Write1d(std::string context, double v1);

  void Write2d(std::string context, double v1, double v2);

  void Write3d(std::string context, double v1, double v2, double v3);

  void Write4d(std::string context, double v1, double v2, double v3, double v4);

  void Write5d(std::string context, double v1, double v2, double v3, double v4,
               double v5);

  void Write6d(std::string context, double v1, double v2, double v3, double v4,
               double v5, double v6);

  void Write7d(std::string context, double v1, double v2, double v3, double v4,
               double v5, double v6, double v7);

  void Write8d(std::string context, double v1, double v2, double v3, double v4,
               double v5, double v6, double v7, double v8);

  void Write9d(std::string context, double v1, double v2, double v3, double v4,
               double v5, double v6, double v7, double v8, double v9);

  void Write10d(std::string context, double v1, double v2, double v3, double v4,
                double v5, double v6, double v7, double v8, double v9,
                double v10);

private:
  std::string m_outputFileName;

  std::ofstream m_file;

  FileType m_fileType;

  std::string m_separator;

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
