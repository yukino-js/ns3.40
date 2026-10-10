
#include "ascii-file.h"

#include "assert.h"
#include "fatal-error.h"
#include "fatal-impl.h"

#include <iostream>
#include <string>

namespace ns3 {

AsciiFile::AsciiFile() : m_file() { FatalImpl::RegisterStream(&m_file); }

AsciiFile::~AsciiFile() {
  FatalImpl::UnregisterStream(&m_file);
  Close();
}

bool AsciiFile::Fail() const { return m_file.fail(); }

bool AsciiFile::Eof() const { return m_file.eof(); }

void AsciiFile::Close() { m_file.close(); }

void AsciiFile::Open(const std::string &filename, std::ios::openmode mode) {
  NS_ASSERT((mode & std::ios::app) == 0);
  NS_ASSERT(!m_file.fail());

  m_file.open(filename, mode);
}

void AsciiFile::Read(std::string &line) {
  NS_ASSERT(m_file.good());

  getline(m_file, line);
}

bool AsciiFile::Diff(const std::string &f1, const std::string &f2,
                     uint64_t &lineNumber) {
  AsciiFile ascii1;
  AsciiFile ascii2;
  ascii1.Open(f1, std::ios::in);
  ascii2.Open(f2, std::ios::in);
  bool bad = ascii1.Fail() || ascii2.Fail();
  if (bad) {
    return true;
  }

  std::string line1;
  std::string line2;
  lineNumber = 0;
  bool diff = false;

  while (!ascii1.Eof() && !ascii2.Eof()) {
    ascii1.Read(line1);
    ascii2.Read(line2);

    lineNumber++;

    bool same = ascii1.Fail() == ascii2.Fail();
    if (!same) {
      diff = true;
      break;
    }
    if (ascii1.Eof()) {
      break;
    }

    if (line1 != line2) {
      diff = true;
      break;
    }
  }

  return diff;
}

} // namespace ns3
