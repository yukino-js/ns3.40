

#include "ns3/log.h"
#include "ns3/pcap-file.h"
#include "ns3/test.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("pcap-file-test-suite");

static uint16_t Swap(uint16_t val) {
  return ((val >> 8) & 0x00ff) | ((val << 8) & 0xff00);
}

static uint32_t Swap(uint32_t val) {
  return ((val >> 24) & 0x000000ff) | ((val >> 8) & 0x0000ff00) |
         ((val << 8) & 0x00ff0000) | ((val << 24) & 0xff000000);
}

static bool CheckFileExists(std::string filename) {
  FILE *p = std::fopen(filename.c_str(), "rb");
  if (p == nullptr) {
    return false;
  }

  std::fclose(p);
  return true;
}

static bool CheckFileLength(std::string filename, long sizeExpected) {
  FILE *p = std::fopen(filename.c_str(), "rb");
  if (p == nullptr) {
    return false;
  }

  std::fseek(p, 0, SEEK_END);

  auto sizeActual = std::ftell(p);
  std::fclose(p);

  return sizeActual == sizeExpected;
}

class WriteModeCreateTestCase : public TestCase {
public:
  WriteModeCreateTestCase();
  ~WriteModeCreateTestCase() override;

private:
  void DoSetup() override;
  void DoRun() override;
  void DoTeardown() override;

  std::string m_testFilename;
};

WriteModeCreateTestCase::WriteModeCreateTestCase()
    : TestCase(
          "Check to see that PcapFile::Open with mode std::ios::out works") {}

WriteModeCreateTestCase::~WriteModeCreateTestCase() {}

void WriteModeCreateTestCase::DoSetup() {
  std::stringstream filename;
  uint32_t n = rand();
  filename << n;
  m_testFilename = CreateTempDirFilename(filename.str() + ".pcap");
}

void WriteModeCreateTestCase::DoTeardown() {
  if (remove(m_testFilename.c_str())) {
    NS_LOG_ERROR("Failed to delete file " << m_testFilename);
  }
}

void WriteModeCreateTestCase::DoRun() {
  PcapFile f;

  f.Open(m_testFilename, std::ios::out);

  NS_TEST_ASSERT_MSG_EQ(f.Fail(), false,
                        "Open (" << m_testFilename << ", \"w\") returns error");
  f.Close();

  NS_TEST_ASSERT_MSG_EQ(CheckFileExists(m_testFilename), true,
                        "Open ("
                            << m_testFilename
                            << ", \"std::ios::out\") does not create file");
  NS_TEST_ASSERT_MSG_EQ(
      CheckFileLength(m_testFilename, 0), true,
      "Open (" << m_testFilename
               << ", \"std::ios::out\") does not result in an empty file");

  f.Open(m_testFilename, std::ios::out);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), false,
                        "Open (" << m_testFilename
                                 << ", \"std::ios::out\") returns error");

  f.Init(1234, 5678, 7);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), false, "Init (1234, 5678, 7) returns error");

  f.Close();

  NS_TEST_ASSERT_MSG_EQ(
      CheckFileLength(m_testFilename, 24), true,
      "Init () does not result in a file with a pcap file header");

  f.Open(m_testFilename, std::ios::out);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), false,
                        "Open (" << m_testFilename
                                 << ", \"std::ios::out\") returns error");

  f.Close();

  NS_TEST_ASSERT_MSG_EQ(CheckFileLength(m_testFilename, 0), true,
                        "Open ("
                            << m_testFilename
                            << ", \"w\") does not result in an empty file");

  f.Open(m_testFilename, std::ios::out);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), false,
                        "Open (" << m_testFilename << ", \"w\") returns error");

  f.Init(1234, 5678, 7);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), false, "Init (1234, 5678, 7) returns error");

  uint8_t buffer[128];
  memset(buffer, 0, sizeof(buffer));
  f.Write(0, 0, buffer, 128);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), false,
                        "Write (write-only-file " << m_testFilename
                                                  << ") returns error");
}

class ReadModeCreateTestCase : public TestCase {
public:
  ReadModeCreateTestCase();
  ~ReadModeCreateTestCase() override;

private:
  void DoSetup() override;
  void DoRun() override;
  void DoTeardown() override;

  std::string m_testFilename;
};

ReadModeCreateTestCase::ReadModeCreateTestCase()
    : TestCase(
          "Check to see that PcapFile::Open with mode std::ios::in works") {}

ReadModeCreateTestCase::~ReadModeCreateTestCase() {}

void ReadModeCreateTestCase::DoSetup() {
  std::stringstream filename;
  uint32_t n = rand();
  filename << n;
  m_testFilename = CreateTempDirFilename(filename.str() + ".pcap");
}

void ReadModeCreateTestCase::DoTeardown() {
  if (remove(m_testFilename.c_str())) {
    NS_LOG_ERROR("Failed to delete file " << m_testFilename);
  }
}

void ReadModeCreateTestCase::DoRun() {
  PcapFile f;

  f.Open(m_testFilename, std::ios::in);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), true,
                        "Open (non-existing-filename "
                            << m_testFilename
                            << ", \"std::ios::in\") does not return error");
  f.Close();
  f.Clear();
  NS_TEST_ASSERT_MSG_EQ(
      CheckFileExists(m_testFilename), false,
      "Open (" << m_testFilename
               << ", \"std::ios::in\") unexpectedly created a file");

  f.Open(m_testFilename, std::ios::out);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), false,
                        "Open (filename, \"std::ios::out\") returns error");
  f.Close();

  f.Open(m_testFilename, std::ios::in);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), true,
                        "Open (non-initialized-filename "
                            << m_testFilename
                            << ", \"std::ios::in\") does not return error");
  f.Close();
  f.Clear();

  f.Open(m_testFilename, std::ios::out);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), false,
                        "Open (" << m_testFilename
                                 << ", \"std::ios::out\") returns error");

  f.Init(1234, 5678, 7);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), false, "Init (1234, 5678, 7) returns error");
  f.Close();

  f.Open(m_testFilename, std::ios::in);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), false,
                        "Open (initialized-filename "
                            << m_testFilename
                            << ", \"std::ios::in\") returns error");

  uint8_t buffer[128];
  f.Write(0, 0, buffer, 128);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), true,
                        "Write (read-only-file " << m_testFilename
                                                 << ") does not return error");
  f.Close();
  f.Clear();
}

#if 0
class AppendModeCreateTestCase : public TestCase
{
public:
  AppendModeCreateTestCase ();
  virtual ~AppendModeCreateTestCase ();

private:
  virtual void DoSetup ();
  virtual void DoRun ();
  virtual void DoTeardown ();

  std::string m_testFilename;
};

AppendModeCreateTestCase::AppendModeCreateTestCase ()
  : TestCase ("Check to see that PcapFile::Open with mode std::ios::app works")
{
}

AppendModeCreateTestCase::~AppendModeCreateTestCase ()
{
}

void
AppendModeCreateTestCase::DoSetup ()
{
  std::stringstream filename;
  uint32_t n = rand ();
  filename << n;
  m_testFilename = CreateTempDirFilename (filename.str () + ".pcap");
}

void
AppendModeCreateTestCase::DoTeardown ()
{
  if (remove (m_testFilename.c_str ()))
    {
      NS_LOG_ERROR ("Failed to delete file " << m_testFilename);
    }
}

void
AppendModeCreateTestCase::DoRun ()
{
  PcapFile f;

  f.Open (m_testFilename, std::ios::out | std::ios::app);
  NS_TEST_ASSERT_MSG_EQ (f.Fail (), true, "Open (non-existing-filename " << m_testFilename <<
                         ", \"std::ios::app\") does not return error");
  f.Close ();
  f.Clear ();

  NS_TEST_ASSERT_MSG_EQ (CheckFileExists (m_testFilename), false,
                         "Open (" << m_testFilename << ", \"std::ios::app\") unexpectedly created a file");

  f.Open (m_testFilename, std::ios::out);
  NS_TEST_ASSERT_MSG_EQ (f.Fail (), false, "Open (" << m_testFilename <<
                         ", \"std::ios::out\") returns error");
  f.Close ();

  f.Open (m_testFilename, std::ios::out | std::ios::app);
  NS_TEST_ASSERT_MSG_EQ (f.Fail (), true, "Open (non-initialized-filename " << m_testFilename <<
                         ", \"std::ios::app\") does not return error");
  f.Close ();
  f.Clear ();

  f.Open (m_testFilename, std::ios::out);
  NS_TEST_ASSERT_MSG_EQ (f.Fail (), false, "Open (non-initialized-filename " << m_testFilename <<
                         ", \"std::ios::out\") returns error");

  f.Init (1234, 5678, 7);
  NS_TEST_ASSERT_MSG_EQ (f.Fail (), false, "Init (1234, 5678, 7) returns error");
  f.Close ();

  f.Open (m_testFilename, std::ios::out | std::ios::app);
  NS_TEST_ASSERT_MSG_EQ (f.Fail (), false, "Open (initialized-filename " << m_testFilename <<
                         ", \"std::ios::app\") returns error");

  uint8_t buffer[128];
  memset (buffer, 0, sizeof(buffer));
  f.Write (0, 0, buffer, 128);
  NS_TEST_ASSERT_MSG_EQ (f.Fail (), false, "Write (append-mode-file " << m_testFilename << ") returns error");

  f.Close ();
}
#endif

class FileHeaderTestCase : public TestCase {
public:
  FileHeaderTestCase();
  ~FileHeaderTestCase() override;

private:
  void DoSetup() override;
  void DoRun() override;
  void DoTeardown() override;

  std::string m_testFilename;
};

FileHeaderTestCase::FileHeaderTestCase()
    : TestCase("Check to see that PcapFileHeader is managed correctly") {}

FileHeaderTestCase::~FileHeaderTestCase() {}

void FileHeaderTestCase::DoSetup() {
  std::stringstream filename;
  uint32_t n = rand();
  filename << n;
  m_testFilename = CreateTempDirFilename(filename.str() + ".pcap");
}

void FileHeaderTestCase::DoTeardown() {
  if (remove(m_testFilename.c_str())) {
    NS_LOG_ERROR("Failed to delete file " << m_testFilename);
  }
}

void FileHeaderTestCase::DoRun() {
  PcapFile f;

  f.Open(m_testFilename, std::ios::out);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), false,
                        "Open (" << m_testFilename
                                 << ", \"std::ios::out\") returns error");

  f.Init(1234, 5678, 7);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), false, "Init (1234, 5678, 7) returns error");
  f.Close();

  FILE *p = std::fopen(m_testFilename.c_str(), "r+b");
  NS_TEST_ASSERT_MSG_NE(
      p, nullptr,
      "fopen("
          << m_testFilename
          << ") should have been able to open a correctly created pcap file");

  uint32_t val32;
  uint16_t val16;

  union {
    uint32_t a;
    uint8_t b[4];
  } u;

  u.a = 1;
  bool bigEndian = u.b[3];

  size_t result = std::fread(&val32, sizeof(val32), 1, p);
  NS_TEST_ASSERT_MSG_EQ(result, 1, "Unable to fread() magic number");
  if (bigEndian) {
    val32 = Swap(val32);
  }
  NS_TEST_ASSERT_MSG_EQ(val32, 0xa1b2c3d4, "Magic number written incorrectly");

  result = std::fread(&val16, sizeof(val16), 1, p);
  NS_TEST_ASSERT_MSG_EQ(result, 1, "Unable to fread() version major");
  if (bigEndian) {
    val16 = Swap(val16);
  }
  NS_TEST_ASSERT_MSG_EQ(val16, 2, "Version major written incorrectly");

  result = std::fread(&val16, sizeof(val16), 1, p);
  NS_TEST_ASSERT_MSG_EQ(result, 1, "Unable to fread() version minor");
  if (bigEndian) {
    val16 = Swap(val16);
  }
  NS_TEST_ASSERT_MSG_EQ(val16, 4, "Version minor written incorrectly");

  result = std::fread(&val32, sizeof(val32), 1, p);
  NS_TEST_ASSERT_MSG_EQ(result, 1, "Unable to fread() time zone correction");
  if (bigEndian) {
    val32 = Swap(val32);
  }
  NS_TEST_ASSERT_MSG_EQ(val32, 7, "Version minor written incorrectly");

  result = std::fread(&val32, sizeof(val32), 1, p);
  NS_TEST_ASSERT_MSG_EQ(result, 1, "Unable to fread() sig figs");
  if (bigEndian) {
    val32 = Swap(val32);
  }
  NS_TEST_ASSERT_MSG_EQ(val32, 0, "Sig figs written incorrectly");

  result = std::fread(&val32, sizeof(val32), 1, p);
  NS_TEST_ASSERT_MSG_EQ(result, 1, "Unable to fread() snap length");
  if (bigEndian) {
    val32 = Swap(val32);
  }
  NS_TEST_ASSERT_MSG_EQ(val32, 5678, "Snap length written incorrectly");

  result = std::fread(&val32, sizeof(val32), 1, p);
  NS_TEST_ASSERT_MSG_EQ(result, 1, "Unable to fread() data link type");
  if (bigEndian) {
    val32 = Swap(val32);
  }
  NS_TEST_ASSERT_MSG_EQ(val32, 1234, "Data length type written incorrectly");

  std::fclose(p);
  p = nullptr;

  f.Open(m_testFilename, std::ios::in);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), false,
                        "Open (existing-initialized-file "
                            << m_testFilename
                            << ", \"std::ios::in\") returns error");

  NS_TEST_ASSERT_MSG_EQ(f.GetMagic(), 0xa1b2c3d4,
                        "Read back magic number incorrectly");
  NS_TEST_ASSERT_MSG_EQ(f.GetVersionMajor(), 2,
                        "Read back version major incorrectly");
  NS_TEST_ASSERT_MSG_EQ(f.GetVersionMinor(), 4,
                        "Read back version minor incorrectly");
  NS_TEST_ASSERT_MSG_EQ(f.GetTimeZoneOffset(), 7,
                        "Read back time zone offset incorrectly");
  NS_TEST_ASSERT_MSG_EQ(f.GetSigFigs(), 0, "Read back sig figs incorrectly");
  NS_TEST_ASSERT_MSG_EQ(f.GetSnapLen(), 5678, "Read back snap len incorrectly");
  NS_TEST_ASSERT_MSG_EQ(f.GetDataLinkType(), 1234,
                        "Read back data link type incorrectly");
  f.Close();

  f.Open(m_testFilename, std::ios::out);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), false,
                        "Open (" << m_testFilename
                                 << ", \"std::ios::out\") returns error");

  f.Init(1234, 5678, 7, true);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), false, "Init (1234, 5678, 7) returns error");
  f.Close();

  p = std::fopen(m_testFilename.c_str(), "r+b");
  NS_TEST_ASSERT_MSG_NE(
      p, nullptr,
      "fopen("
          << m_testFilename
          << ") should have been able to open a correctly created pcap file");

  result = std::fread(&val32, sizeof(val32), 1, p);
  NS_TEST_ASSERT_MSG_EQ(result, 1, "Unable to fread() magic number");
  NS_TEST_ASSERT_MSG_EQ(val32, Swap(uint32_t(0xa1b2c3d4)),
                        "Magic number written incorrectly");

  result = std::fread(&val16, sizeof(val16), 1, p);
  NS_TEST_ASSERT_MSG_EQ(result, 1, "Unable to fread() version major");
  NS_TEST_ASSERT_MSG_EQ(val16, Swap(uint16_t(2)),
                        "Version major written incorrectly");

  result = std::fread(&val16, sizeof(val16), 1, p);
  NS_TEST_ASSERT_MSG_EQ(result, 1, "Unable to fread() version minor");
  NS_TEST_ASSERT_MSG_EQ(val16, Swap(uint16_t(4)),
                        "Version minor written incorrectly");

  result = std::fread(&val32, sizeof(val32), 1, p);
  NS_TEST_ASSERT_MSG_EQ(result, 1, "Unable to fread() time zone correction");
  NS_TEST_ASSERT_MSG_EQ(val32, Swap(uint32_t(7)),
                        "Version minor written incorrectly");

  result = std::fread(&val32, sizeof(val32), 1, p);
  NS_TEST_ASSERT_MSG_EQ(result, 1, "Unable to fread() sig figs");
  NS_TEST_ASSERT_MSG_EQ(val32, 0, "Sig figs written incorrectly");

  result = std::fread(&val32, sizeof(val32), 1, p);
  NS_TEST_ASSERT_MSG_EQ(result, 1, "Unable to fread() snap length");
  NS_TEST_ASSERT_MSG_EQ(val32, Swap(uint32_t(5678)),
                        "Snap length written incorrectly");

  result = std::fread(&val32, sizeof(val32), 1, p);
  NS_TEST_ASSERT_MSG_EQ(result, 1, "Unable to fread() data link type");
  NS_TEST_ASSERT_MSG_EQ(val32, Swap(uint32_t(1234)),
                        "Data length type written incorrectly");

  std::fclose(p);
  p = nullptr;

  f.Open(m_testFilename, std::ios::in);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), false,
                        "Open (existing-initialized-file "
                            << m_testFilename
                            << ", \"std::ios::in\") returns error");

  NS_TEST_ASSERT_MSG_EQ(f.GetSwapMode(), true,
                        "Byte-swapped file not correctly indicated");

  NS_TEST_ASSERT_MSG_EQ(f.GetMagic(), 0xa1b2c3d4,
                        "Read back magic number incorrectly");
  NS_TEST_ASSERT_MSG_EQ(f.GetVersionMajor(), 2,
                        "Read back version major incorrectly");
  NS_TEST_ASSERT_MSG_EQ(f.GetVersionMinor(), 4,
                        "Read back version minor incorrectly");
  NS_TEST_ASSERT_MSG_EQ(f.GetTimeZoneOffset(), 7,
                        "Read back time zone offset incorrectly");
  NS_TEST_ASSERT_MSG_EQ(f.GetSigFigs(), 0, "Read back sig figs incorrectly");
  NS_TEST_ASSERT_MSG_EQ(f.GetSnapLen(), 5678, "Read back snap len incorrectly");
  NS_TEST_ASSERT_MSG_EQ(f.GetDataLinkType(), 1234,
                        "Read back data link type incorrectly");

  f.Close();
}

class RecordHeaderTestCase : public TestCase {
public:
  RecordHeaderTestCase();
  ~RecordHeaderTestCase() override;

private:
  void DoSetup() override;
  void DoRun() override;
  void DoTeardown() override;

  std::string m_testFilename;
};

RecordHeaderTestCase::RecordHeaderTestCase()
    : TestCase("Check to see that PcapRecordHeader is managed correctly") {}

RecordHeaderTestCase::~RecordHeaderTestCase() {}

void RecordHeaderTestCase::DoSetup() {
  std::stringstream filename;
  uint32_t n = rand();
  filename << n;
  m_testFilename = CreateTempDirFilename(filename.str() + ".pcap");
}

void RecordHeaderTestCase::DoTeardown() {
  if (remove(m_testFilename.c_str())) {
    NS_LOG_ERROR("Failed to delete file " << m_testFilename);
  }
}

void RecordHeaderTestCase::DoRun() {
  PcapFile f;

  f.Open(m_testFilename, std::ios::out);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), false,
                        "Open (" << m_testFilename
                                 << ", \"std::ios::out\") returns error");

  f.Init(37, 43, -7);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), false, "Init (37, 43, -7) returns error");

  uint8_t bufferOut[128];
  for (uint32_t i = 0; i < 128; ++i) {
    bufferOut[i] = i;
  }

  f.Write(1234, 5678, bufferOut, 128);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), false,
                        "Write (write-only-file " << m_testFilename
                                                  << ") returns error");
  f.Close();

  FILE *p = std::fopen(m_testFilename.c_str(), "r+b");
  NS_TEST_ASSERT_MSG_NE(
      p, nullptr,
      "fopen() should have been able to open a correctly created pcap file");

  std::fseek(p, 0, SEEK_END);
  auto size = std::ftell(p);
  NS_TEST_ASSERT_MSG_EQ(size, 83,
                        "Pcap file with one 43 byte packet is incorrect size");

  std::fseek(p, 24, SEEK_SET);

  uint32_t val32;

  union {
    uint32_t a;
    uint8_t b[4];
  } u;

  u.a = 1;
  bool bigEndian = u.b[3];

  size_t result = std::fread(&val32, sizeof(val32), 1, p);
  NS_TEST_ASSERT_MSG_EQ(result, 1, "Unable to fread() seconds timestamp");
  if (bigEndian) {
    val32 = Swap(val32);
  }
  NS_TEST_ASSERT_MSG_EQ(val32, 1234, "Seconds timestamp written incorrectly");

  result = std::fread(&val32, sizeof(val32), 1, p);
  NS_TEST_ASSERT_MSG_EQ(result, 1, "Unable to fread() microseconds timestamp");
  if (bigEndian) {
    val32 = Swap(val32);
  }
  NS_TEST_ASSERT_MSG_EQ(val32, 5678,
                        "Microseconds timestamp written incorrectly");

  result = std::fread(&val32, sizeof(val32), 1, p);
  NS_TEST_ASSERT_MSG_EQ(result, 1, "Unable to fread() included length");
  if (bigEndian) {
    val32 = Swap(val32);
  }
  NS_TEST_ASSERT_MSG_EQ(val32, 43, "Included length written incorrectly");

  result = std::fread(&val32, sizeof(val32), 1, p);
  NS_TEST_ASSERT_MSG_EQ(result, 1, "Unable to fread() actual length");
  if (bigEndian) {
    val32 = Swap(val32);
  }
  NS_TEST_ASSERT_MSG_EQ(val32, 128, "Actual length written incorrectly");

  uint8_t bufferIn[128];

  result = std::fread(bufferIn, 1, 43, p);
  NS_TEST_ASSERT_MSG_EQ(result, 43,
                        "Unable to fread() packet data of expected length");

  for (uint32_t i = 0; i < 43; ++i) {
    NS_TEST_ASSERT_MSG_EQ(bufferIn[i], bufferOut[i],
                          "Incorrect packet data written");
  }

  std::fclose(p);
  p = nullptr;

  f.Open(m_testFilename, std::ios::in);
  NS_TEST_ASSERT_MSG_EQ(
      f.Fail(), false,
      "Open (" << m_testFilename
               << ", \"std::ios::in\") of existing good file returns error");

  uint32_t tsSec;
  uint32_t tsUsec;
  uint32_t inclLen;
  uint32_t origLen;
  uint32_t readLen;

  f.Read(bufferIn, sizeof(bufferIn), tsSec, tsUsec, inclLen, origLen, readLen);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), false,
                        "Read() of known good packet returns error");
  NS_TEST_ASSERT_MSG_EQ(
      tsSec, 1234, "Incorrectly read seconds timestamp from known good packet");
  NS_TEST_ASSERT_MSG_EQ(
      tsUsec, 5678,
      "Incorrectly read microseconds timestamp from known good packet");
  NS_TEST_ASSERT_MSG_EQ(
      inclLen, 43, "Incorrectly read included length from known good packet");
  NS_TEST_ASSERT_MSG_EQ(
      origLen, 128, "Incorrectly read original length from known good packet");
  NS_TEST_ASSERT_MSG_EQ(readLen, 43,
                        "Incorrectly constructed actual read length from known "
                        "good packet given buffer size");
  f.Close();

  for (uint32_t i = 0; i < 43; ++i) {
    NS_TEST_ASSERT_MSG_EQ(bufferIn[i], bufferOut[i],
                          "Incorrect packet data read from known good packet");
  }

  f.Open(m_testFilename, std::ios::out);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), false,
                        "Open (" << m_testFilename
                                 << ", \"std::ios::out\") returns error");

  f.Init(37, 43, -7, true);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), false, "Init (37, 43, -7) returns error");

  f.Write(1234, 5678, bufferOut, 128);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), false,
                        "Write (write-only-file " << m_testFilename
                                                  << ") returns error");
  f.Close();

  p = std::fopen(m_testFilename.c_str(), "r+b");
  NS_TEST_ASSERT_MSG_NE(
      p, nullptr,
      "fopen() should have been able to open a correctly created pcap file");

  std::fseek(p, 0, SEEK_END);
  size = std::ftell(p);
  NS_TEST_ASSERT_MSG_EQ(size, 83,
                        "Pcap file with one 43 byte packet is incorrect size");

  result = std::fseek(p, 24, SEEK_SET);
  NS_TEST_ASSERT_MSG_EQ(result, 0, "Failed seeking past pcap header");

  result = std::fread(&val32, sizeof(val32), 1, p);
  NS_TEST_ASSERT_MSG_EQ(result, 1, "Unable to fread() seconds timestamp");
  NS_TEST_ASSERT_MSG_EQ(val32, Swap(uint32_t(1234)),
                        "Swapped seconds timestamp written incorrectly");

  result = std::fread(&val32, sizeof(val32), 1, p);
  NS_TEST_ASSERT_MSG_EQ(result, 1, "Unable to fread() microseconds timestamp");
  NS_TEST_ASSERT_MSG_EQ(val32, Swap(uint32_t(5678)),
                        "Swapped microseconds timestamp written incorrectly");

  result = std::fread(&val32, sizeof(val32), 1, p);
  NS_TEST_ASSERT_MSG_EQ(result, 1, "Unable to fread() included length");
  NS_TEST_ASSERT_MSG_EQ(val32, Swap(uint32_t(43)),
                        "Swapped included length written incorrectly");

  result = std::fread(&val32, sizeof(val32), 1, p);
  NS_TEST_ASSERT_MSG_EQ(result, 1, "Unable to fread() actual length");
  NS_TEST_ASSERT_MSG_EQ(val32, Swap(uint32_t(128)),
                        "Swapped Actual length written incorrectly");

  result = std::fread(bufferIn, 1, 43, p);
  NS_TEST_ASSERT_MSG_EQ(result, 43,
                        "Unable to fread() packet data of expected length");

  for (uint32_t i = 0; i < 43; ++i) {
    NS_TEST_ASSERT_MSG_EQ(bufferIn[i], bufferOut[i],
                          "Incorrect packet data written");
  }

  std::fclose(p);
  p = nullptr;

  f.Open(m_testFilename, std::ios::in);
  NS_TEST_ASSERT_MSG_EQ(
      f.Fail(), false,
      "Open (" << m_testFilename
               << ", \"std::ios::in\") of existing good file returns error");

  f.Read(bufferIn, sizeof(bufferIn), tsSec, tsUsec, inclLen, origLen, readLen);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), false,
                        "Read() of known good packet returns error");
  NS_TEST_ASSERT_MSG_EQ(
      tsSec, 1234, "Incorrectly read seconds timestamp from known good packet");
  NS_TEST_ASSERT_MSG_EQ(
      tsUsec, 5678,
      "Incorrectly read microseconds timestamp from known good packet");
  NS_TEST_ASSERT_MSG_EQ(
      inclLen, 43, "Incorrectly read included length from known good packet");
  NS_TEST_ASSERT_MSG_EQ(
      origLen, 128, "Incorrectly read original length from known good packet");
  NS_TEST_ASSERT_MSG_EQ(readLen, 43,
                        "Incorrectly constructed actual read length from known "
                        "good packet given buffer size");

  for (uint32_t i = 0; i < 43; ++i) {
    NS_TEST_ASSERT_MSG_EQ(bufferIn[i], bufferOut[i],
                          "Incorrect packet data read from known good packet");
  }

  f.Close();
}

class ReadFileTestCase : public TestCase {
public:
  ReadFileTestCase();
  ~ReadFileTestCase() override;

private:
  void DoSetup() override;
  void DoRun() override;
  void DoTeardown() override;

  std::string m_testFilename;
};

ReadFileTestCase::ReadFileTestCase()
    : TestCase(
          "Check to see that PcapFile can read out a known good pcap file") {}

ReadFileTestCase::~ReadFileTestCase() {}

void ReadFileTestCase::DoSetup() {}

void ReadFileTestCase::DoTeardown() {}

static const uint32_t N_KNOWN_PACKETS = 6;
static const uint32_t N_PACKET_BYTES = 16;

struct PacketEntry {
  uint32_t tsSec;
  uint32_t tsUsec;
  uint32_t inclLen;
  uint32_t origLen;
  uint16_t data[N_PACKET_BYTES];
};

static const PacketEntry knownPackets[] = {
    {2,
     3696,
     46,
     46,
     {0x0001, 0x0800, 0x0604, 0x0001, 0x0000, 0x0000, 0x0003, 0x0a01, 0x0201,
      0xffff, 0xffff, 0xffff, 0x0a01, 0x0204, 0x0000, 0x0000}},
    {2,
     3707,
     46,
     46,
     {0x0001, 0x0800, 0x0604, 0x0002, 0x0000, 0x0000, 0x0006, 0x0a01, 0x0204,
      0x0000, 0x0000, 0x0003, 0x0a01, 0x0201, 0x0000, 0x0000}},
    {2,
     3801,
     1070,
     1070,
     {0x4500, 0x041c, 0x0000, 0x0000, 0x3f11, 0x0000, 0x0a01, 0x0101, 0x0a01,
      0x0204, 0xc001, 0x0009, 0x0408, 0x0000, 0x0000, 0x0000}},
    {2,
     3811,
     46,
     46,
     {0x0001, 0x0800, 0x0604, 0x0001, 0x0000, 0x0000, 0x0006, 0x0a01, 0x0204,
      0xffff, 0xffff, 0xffff, 0x0a01, 0x0201, 0x0000, 0x0000}},
    {2,
     3822,
     46,
     46,
     {0x0001, 0x0800, 0x0604, 0x0002, 0x0000, 0x0000, 0x0003, 0x0a01, 0x0201,
      0x0000, 0x0000, 0x0006, 0x0a01, 0x0204, 0x0000, 0x0000}},
    {2,
     3915,
     1070,
     1070,
     {0x4500, 0x041c, 0x0000, 0x0000, 0x4011, 0x0000, 0x0a01, 0x0204, 0x0a01,
      0x0101, 0x0009, 0xc001, 0x0408, 0x0000, 0x0000, 0x0000}},
};

void ReadFileTestCase::DoRun() {
  PcapFile f;

  std::string filename = CreateDataDirFilename("known.pcap");
  f.Open(filename, std::ios::in);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), false,
                        "Open (" << filename
                                 << ", \"std::ios::in\") returns error");

  uint8_t data[N_PACKET_BYTES];
  uint32_t tsSec;
  uint32_t tsUsec;
  uint32_t inclLen;
  uint32_t origLen;
  uint32_t readLen;

  for (uint32_t i = 0; i < N_KNOWN_PACKETS; ++i) {
    const PacketEntry &p = knownPackets[i];

    f.Read(data, sizeof(data), tsSec, tsUsec, inclLen, origLen, readLen);
    NS_TEST_ASSERT_MSG_EQ(f.Fail(), false,
                          "Read() of known good pcap file returns error");
    NS_TEST_ASSERT_MSG_EQ(
        tsSec, p.tsSec,
        "Incorrectly read seconds timestamp from known good pcap file");
    NS_TEST_ASSERT_MSG_EQ(
        tsUsec, p.tsUsec,
        "Incorrectly read microseconds timestamp from known good pcap file");
    NS_TEST_ASSERT_MSG_EQ(
        inclLen, p.inclLen,
        "Incorrectly read included length from known good packet");
    NS_TEST_ASSERT_MSG_EQ(
        origLen, p.origLen,
        "Incorrectly read original length from known good packet");
    NS_TEST_ASSERT_MSG_EQ(readLen, N_PACKET_BYTES,
                          "Incorrect actual read length from known good packet "
                          "given buffer size");
  }

  f.Read(data, 1, tsSec, tsUsec, inclLen, origLen, readLen);
  NS_TEST_ASSERT_MSG_EQ(
      f.Eof(), true,
      "Read() of known good pcap file at EOF does not return error");

  f.Close();
}

class DiffTestCase : public TestCase {
public:
  DiffTestCase();

private:
  void DoRun() override;
};

DiffTestCase::DiffTestCase()
    : TestCase("Check that PcapFile::Diff works as expected") {}

void DiffTestCase::DoRun() {
  std::string filename = CreateDataDirFilename("known.pcap");
  uint32_t sec(0);
  uint32_t usec(0);
  uint32_t packets(0);
  bool diff = PcapFile::Diff(filename, filename, sec, usec, packets);
  NS_TEST_EXPECT_MSG_EQ(diff, false,
                        "PcapDiff(file, file) must always be false");

  std::string filename2 = CreateTempDirFilename("different.pcap");
  PcapFile f;

  f.Open(filename2, std::ios::out);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), false,
                        "Open (" << filename2
                                 << ", \"std::ios::out\") returns error");
  f.Init(1, N_PACKET_BYTES);
  NS_TEST_ASSERT_MSG_EQ(f.Fail(), false,
                        "Init (1, " << N_PACKET_BYTES << ") returns error");

  for (uint32_t i = 0; i < N_KNOWN_PACKETS; ++i) {
    const PacketEntry &p = knownPackets[i];

    f.Write(p.tsSec, p.tsUsec, (const uint8_t *)p.data, p.origLen);
    NS_TEST_EXPECT_MSG_EQ(f.Fail(), false, "Write must not fail");
  }
  f.Close();

  packets = 0;
  diff = PcapFile::Diff(filename, filename2, sec, usec, packets);
  NS_TEST_EXPECT_MSG_EQ(diff, true, "PcapDiff(file, file2) must be true");
  NS_TEST_EXPECT_MSG_EQ(sec, 2, "Files are different from 2.3696 seconds");
  NS_TEST_EXPECT_MSG_EQ(usec, 3696, "Files are different from 2.3696 seconds");
}

class PcapFileTestSuite : public TestSuite {
public:
  PcapFileTestSuite();
};

PcapFileTestSuite::PcapFileTestSuite() : TestSuite("pcap-file", UNIT) {
  SetDataDir(NS_TEST_SOURCEDIR);
  AddTestCase(new WriteModeCreateTestCase, TestCase::QUICK);
  AddTestCase(new ReadModeCreateTestCase, TestCase::QUICK);
  AddTestCase(new FileHeaderTestCase, TestCase::QUICK);
  AddTestCase(new RecordHeaderTestCase, TestCase::QUICK);
  AddTestCase(new ReadFileTestCase, TestCase::QUICK);
  AddTestCase(new DiffTestCase, TestCase::QUICK);
}

static PcapFileTestSuite pcapFileTestSuite;
