

#ifndef ERROR_MODEL_H
#define ERROR_MODEL_H

#include "ns3/object.h"
#include "ns3/random-variable-stream.h"

#include <list>

namespace ns3 {

class Packet;

class ErrorModel : public Object {
public:
  static TypeId GetTypeId();

  ErrorModel();
  ~ErrorModel() override;

  bool IsCorrupt(Ptr<Packet> pkt);
  void Reset();
  void Enable();
  void Disable();
  bool IsEnabled() const;

private:
  virtual bool DoCorrupt(Ptr<Packet> p) = 0;
  virtual void DoReset() = 0;

  bool m_enable;
};

class RateErrorModel : public ErrorModel {
public:
  static TypeId GetTypeId();

  RateErrorModel();
  ~RateErrorModel() override;

  enum ErrorUnit { ERROR_UNIT_BIT, ERROR_UNIT_BYTE, ERROR_UNIT_PACKET };

  RateErrorModel::ErrorUnit GetUnit() const;
  void SetUnit(ErrorUnit error_unit);

  double GetRate() const;
  void SetRate(double rate);

  void SetRandomVariable(Ptr<RandomVariableStream>);

  int64_t AssignStreams(int64_t stream);

private:
  bool DoCorrupt(Ptr<Packet> p) override;
  virtual bool DoCorruptPkt(Ptr<Packet> p);
  virtual bool DoCorruptByte(Ptr<Packet> p);
  virtual bool DoCorruptBit(Ptr<Packet> p);
  void DoReset() override;

  ErrorUnit m_unit;
  double m_rate;

  Ptr<RandomVariableStream> m_ranvar;
};

class BurstErrorModel : public ErrorModel {
public:
  static TypeId GetTypeId();

  BurstErrorModel();
  ~BurstErrorModel() override;

  double GetBurstRate() const;
  void SetBurstRate(double rate);

  void SetRandomVariable(Ptr<RandomVariableStream> ranVar);

  void SetRandomBurstSize(Ptr<RandomVariableStream> burstSz);

  int64_t AssignStreams(int64_t stream);

private:
  bool DoCorrupt(Ptr<Packet> p) override;
  void DoReset() override;

  double m_burstRate;
  Ptr<RandomVariableStream> m_burstStart;
  Ptr<RandomVariableStream> m_burstSize;

  uint32_t m_counter;
  uint32_t m_currentBurstSz;
};

class ListErrorModel : public ErrorModel {
public:
  static TypeId GetTypeId();
  ListErrorModel();
  ~ListErrorModel() override;

  std::list<uint64_t> GetList() const;
  void SetList(const std::list<uint64_t> &packetlist);

private:
  bool DoCorrupt(Ptr<Packet> p) override;
  void DoReset() override;

  typedef std::list<uint64_t> PacketList;
  typedef std::list<uint64_t>::const_iterator PacketListCI;

  PacketList m_packetList;
};

class ReceiveListErrorModel : public ErrorModel {
public:
  static TypeId GetTypeId();
  ReceiveListErrorModel();
  ~ReceiveListErrorModel() override;

  std::list<uint32_t> GetList() const;
  void SetList(const std::list<uint32_t> &packetlist);

private:
  bool DoCorrupt(Ptr<Packet> p) override;
  void DoReset() override;

  typedef std::list<uint32_t> PacketList;
  typedef std::list<uint32_t>::const_iterator PacketListCI;

  PacketList m_packetList;
  uint32_t m_timesInvoked;
};

class BinaryErrorModel : public ErrorModel {
public:
  static TypeId GetTypeId();

  BinaryErrorModel();
  ~BinaryErrorModel() override;

private:
  bool DoCorrupt(Ptr<Packet> p) override;
  void DoReset() override;

  uint8_t m_counter;
};

} // namespace ns3
#endif
