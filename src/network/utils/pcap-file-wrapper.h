
#ifndef PCAP_FILE_WRAPPER_H
#define PCAP_FILE_WRAPPER_H

#include "pcap-file.h"

#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"

#include <cstring>
#include <fstream>
#include <limits>

namespace ns3 {

class PcapFileWrapper : public Object {
public:
  static TypeId GetTypeId();

  PcapFileWrapper();
  ~PcapFileWrapper() override;

  bool Fail() const;
  bool Eof() const;
  void Clear();

  void Open(const std::string &filename, std::ios::openmode mode);

  void Close();

  void Init(uint32_t dataLinkType,
            uint32_t snapLen = std::numeric_limits<uint32_t>::max(),
            int32_t tzCorrection = PcapFile::ZONE_DEFAULT);

  void Write(Time t, Ptr<const Packet> p);

  void Write(Time t, const Header &header, Ptr<const Packet> p);

  void Write(Time t, const uint8_t *buffer, uint32_t length);

  Ptr<Packet> Read(Time &t);

  uint32_t GetMagic();

  uint16_t GetVersionMajor();

  uint16_t GetVersionMinor();

  int32_t GetTimeZoneOffset();

  uint32_t GetSigFigs();

  uint32_t GetSnapLen();

  uint32_t GetDataLinkType();

private:
  PcapFile m_file;
  uint32_t m_snapLen;
  bool m_nanosecMode;
};

} // namespace ns3

#endif
