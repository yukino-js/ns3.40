
#ifndef PCAP_FILE_H
#define PCAP_FILE_H

#include "ns3/ptr.h"

#include <fstream>
#include <stdint.h>
#include <string>

namespace ns3 {

class Packet;
class Header;

class PcapFile {
public:
  static const int32_t ZONE_DEFAULT = 0;
  static const uint32_t SNAPLEN_DEFAULT = 65535;

public:
  PcapFile();
  ~PcapFile();

  bool Fail() const;
  bool Eof() const;
  void Clear();

  void Open(const std::string &filename, std::ios::openmode mode);

  void Close();

  void Init(uint32_t dataLinkType, uint32_t snapLen = SNAPLEN_DEFAULT,
            int32_t timeZoneCorrection = ZONE_DEFAULT, bool swapMode = false,
            bool nanosecMode = false);

  void Write(uint32_t tsSec, uint32_t tsUsec, const uint8_t *const data,
             uint32_t totalLen);

  void Write(uint32_t tsSec, uint32_t tsUsec, Ptr<const Packet> p);
  void Write(uint32_t tsSec, uint32_t tsUsec, const Header &header,
             Ptr<const Packet> p);

  void Read(uint8_t *const data, uint32_t maxBytes, uint32_t &tsSec,
            uint32_t &tsUsec, uint32_t &inclLen, uint32_t &origLen,
            uint32_t &readLen);

  bool GetSwapMode();

  bool IsNanoSecMode();

  uint32_t GetMagic();

  uint16_t GetVersionMajor();

  uint16_t GetVersionMinor();

  int32_t GetTimeZoneOffset();

  uint32_t GetSigFigs();

  uint32_t GetSnapLen();

  uint32_t GetDataLinkType();

  static bool Diff(const std::string &f1, const std::string &f2, uint32_t &sec,
                   uint32_t &usec, uint32_t &packets,
                   uint32_t snapLen = SNAPLEN_DEFAULT);

private:
  struct PcapFileHeader {
    uint32_t m_magicNumber;
    uint16_t m_versionMajor;
    uint16_t m_versionMinor;
    int32_t m_zone;
    uint32_t m_sigFigs;
    uint32_t m_snapLen;
    uint32_t m_type;
  };

  struct PcapRecordHeader {
    uint32_t m_tsSec;
    uint32_t m_tsUsec;
    uint32_t m_inclLen;
    uint32_t m_origLen;
  };

  uint8_t Swap(uint8_t val);
  uint16_t Swap(uint16_t val);
  uint32_t Swap(uint32_t val);
  void Swap(PcapFileHeader *from, PcapFileHeader *to);
  void Swap(PcapRecordHeader *from, PcapRecordHeader *to);

  void WriteFileHeader();
  uint32_t WritePacketHeader(uint32_t tsSec, uint32_t tsUsec,
                             uint32_t totalLen);

  void ReadAndVerifyFileHeader();

  std::string m_filename;
  std::fstream m_file;
  PcapFileHeader m_fileHeader;
  bool m_swapMode;
  bool m_nanosecMode;
};

} // namespace ns3

#endif
