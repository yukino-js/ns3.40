
#ifndef ASCII_FILE_H
#define ASCII_FILE_H

#include <fstream>
#include <stdint.h>
#include <string>

namespace ns3 {

class AsciiFile {
public:
  AsciiFile();
  ~AsciiFile();

  bool Fail() const;
  bool Eof() const;

  void Open(const std::string &filename, std::ios::openmode mode);

  void Close();

  void Read(std::string &line);

  static bool Diff(const std::string &f1, const std::string &f2,
                   uint64_t &lineNumber);

private:
  std::string m_filename;
  std::fstream m_file;
};

} // namespace ns3

#endif
