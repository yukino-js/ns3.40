
#ifndef UAN_HEADER_COMMON_H
#define UAN_HEADER_COMMON_H

#include "ns3/header.h"
#include "ns3/mac8-address.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"

namespace ns3 {

struct UanProtocolBits {
  uint8_t m_type : 4;
  uint8_t m_protocolNumber : 4;
};

class UanHeaderCommon : public Header {
public:
  UanHeaderCommon();
  UanHeaderCommon(const Mac8Address src, const Mac8Address dest, uint8_t type,
                  uint8_t protocolNumber);
  ~UanHeaderCommon() override;

  static TypeId GetTypeId();

  void SetDest(Mac8Address dest);
  void SetSrc(Mac8Address src);
  void SetType(uint8_t type);
  void SetProtocolNumber(uint16_t protocolNumber);

  Mac8Address GetDest() const;
  Mac8Address GetSrc() const;
  uint8_t GetType() const;
  uint16_t GetProtocolNumber() const;

  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;
  TypeId GetInstanceTypeId() const override;

private:
  Mac8Address m_dest;
  Mac8Address m_src;
  UanProtocolBits m_uanProtocolBits{0};
};

} // namespace ns3

#endif
