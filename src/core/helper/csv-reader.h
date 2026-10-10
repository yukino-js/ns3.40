
#ifndef NS3_CSV_READER_H_
#define NS3_CSV_READER_H_

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <istream>
#include <string>
#include <vector>

namespace ns3 {

class CsvReader {
public:
  CsvReader(const std::string &filepath, char delimiter = ',');

  CsvReader(std::istream &stream, char delimiter = ',');

  virtual ~CsvReader();

  std::size_t ColumnCount() const;

  std::size_t RowNumber() const;

  char Delimiter() const;

  bool FetchNextRow();

  template <class T> bool GetValue(std::size_t columnIndex, T &value) const;

  bool IsBlankRow() const;

private:
  bool GetValueAs(std::string input, double &value) const;

  bool GetValueAs(std::string input, float &value) const;

  bool GetValueAs(std::string input, signed char &value) const;

  bool GetValueAs(std::string input, short &value) const;

  bool GetValueAs(std::string input, int &value) const;

  bool GetValueAs(std::string input, long &value) const;

  bool GetValueAs(std::string input, long long &value) const;

  bool GetValueAs(std::string input, std::string &value) const;

  bool GetValueAs(std::string input, unsigned char &value) const;

  bool GetValueAs(std::string input, unsigned short &value) const;

  bool GetValueAs(std::string input, unsigned int &value) const;

  bool GetValueAs(std::string input, unsigned long &value) const;

  bool GetValueAs(std::string input, unsigned long long &value) const;

  bool IsDelimiter(char c) const;

  void ParseLine(const std::string &line);

  std::tuple<std::string, std::string::const_iterator>
  ParseColumn(std::string::const_iterator begin,
              std::string::const_iterator end);

  typedef std::vector<std::string> Columns;

  char m_delimiter;
  std::size_t m_rowsRead;
  Columns m_columns;
  bool m_blankRow;
  std::ifstream m_fileStream;

  std::istream *m_stream;
};

template <class T>
bool CsvReader::GetValue(std::size_t columnIndex, T &value) const {
  if (columnIndex >= ColumnCount()) {
    return false;
  }

  std::string cell = m_columns[columnIndex];

  return GetValueAs(std::move(cell), value);
}

} // namespace ns3

#endif
