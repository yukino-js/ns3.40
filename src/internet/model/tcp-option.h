
#ifndef TCP_OPTION_H
#define TCP_OPTION_H

#include "ns3/buffer.h"
#include "ns3/object-factory.h"
#include "ns3/object.h"

#include <stdint.h>

namespace ns3 {

class TcpOption : public Object {
public:
  TcpOption();
  ~TcpOption() override;

  static TypeId GetTypeId();

  TypeId GetInstanceTypeId() const override;

  enum Kind {
    END = 0,
    NOP = 1,
    MSS = 2,
    WINSCALE = 3,
    SACKPERMITTED = 4,
    SACK = 5,
    TS = 8,
    UNKNOWN = 255
  };

  virtual void Print(std::ostream &os) const = 0;
  virtual void Serialize(Buffer::Iterator start) const = 0;

  virtual uint32_t Deserialize(Buffer::Iterator start) = 0;

  virtual uint8_t GetKind() const = 0;
  virtual uint32_t GetSerializedSize() const = 0;

  static Ptr<TcpOption> CreateOption(uint8_t kind);

  static bool IsKindKnown(uint8_t kind);
};

class TcpOptionUnknown : public TcpOption {
public:
  TcpOptionUnknown();
  ~TcpOptionUnknown() override;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;

  void Print(std::ostream &os) const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

  uint8_t GetKind() const override;
  uint32_t GetSerializedSize() const override;

private:
  uint8_t m_kind;
  uint32_t m_size;
  uint8_t m_content[40];
};

} // namespace ns3

#endif
