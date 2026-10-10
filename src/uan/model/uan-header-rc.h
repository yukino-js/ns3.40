
#ifndef UAN_HEADER_RC_H
#define UAN_HEADER_RC_H

#include "ns3/header.h"
#include "ns3/mac8-address.h"
#include "ns3/nstime.h"

#include <set>

namespace ns3 {

class UanHeaderRcData : public Header {
public:
  UanHeaderRcData();
  UanHeaderRcData(uint8_t frameNum, Time propDelay);
  ~UanHeaderRcData() override;

  static TypeId GetTypeId();

  void SetFrameNo(uint8_t frameNum);
  void SetPropDelay(Time propDelay);
  uint8_t GetFrameNo() const;
  Time GetPropDelay() const;
  void Print(std::ostream &os, Time::Unit unit) const;

  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;
  TypeId GetInstanceTypeId() const override;

private:
  uint8_t m_frameNo;
  Time m_propDelay;
};

class UanHeaderRcRts : public Header {
public:
  UanHeaderRcRts();
  UanHeaderRcRts(uint8_t frameNo, uint8_t retryNo, uint8_t noFrames,
                 uint16_t length, Time ts);
  ~UanHeaderRcRts() override;

  static TypeId GetTypeId();

  void SetFrameNo(uint8_t fno);
  void SetNoFrames(uint8_t no);
  void SetTimeStamp(Time timeStamp);
  void SetLength(uint16_t length);
  void SetRetryNo(uint8_t no);

  uint8_t GetFrameNo() const;
  uint8_t GetNoFrames() const;
  Time GetTimeStamp() const;
  uint16_t GetLength() const;
  uint8_t GetRetryNo() const;
  void Print(std::ostream &os, Time::Unit unit) const;

  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;
  TypeId GetInstanceTypeId() const override;

private:
  uint8_t m_frameNo;
  uint8_t m_noFrames;
  uint16_t m_length;
  Time m_timeStamp;
  uint8_t m_retryNo;
};

class UanHeaderRcCtsGlobal : public Header {
public:
  UanHeaderRcCtsGlobal();
  UanHeaderRcCtsGlobal(Time wt, Time ts, uint16_t rate, uint16_t retryRate);
  ~UanHeaderRcCtsGlobal() override;

  static TypeId GetTypeId();

  void SetRateNum(uint16_t rate);
  void SetRetryRate(uint16_t rate);
  void SetWindowTime(Time t);

  void SetTxTimeStamp(Time timeStamp);

  uint16_t GetRateNum() const;
  uint16_t GetRetryRate() const;
  Time GetWindowTime() const;
  Time GetTxTimeStamp() const;
  void Print(std::ostream &os, Time::Unit unit) const;

  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;
  TypeId GetInstanceTypeId() const override;

private:
  Time m_timeStampTx;
  Time m_winTime;
  uint16_t m_retryRate;
  uint16_t m_rateNum;
};

class UanHeaderRcCts : public Header {
public:
  UanHeaderRcCts();
  UanHeaderRcCts(uint8_t frameNo, uint8_t retryNo, Time rtsTs, Time delay,
                 Mac8Address addr);
  ~UanHeaderRcCts() override;

  static TypeId GetTypeId();

  void SetFrameNo(uint8_t frameNo);
  void SetRtsTimeStamp(Time timeStamp);
  void SetDelayToTx(Time delay);
  void SetRetryNo(uint8_t no);
  void SetAddress(Mac8Address addr);

  uint8_t GetFrameNo() const;
  Time GetRtsTimeStamp() const;
  Time GetDelayToTx() const;
  uint8_t GetRetryNo() const;
  Mac8Address GetAddress() const;
  void Print(std::ostream &os, Time::Unit unit) const;

  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;
  TypeId GetInstanceTypeId() const override;

private:
  uint8_t m_frameNo;
  Time m_timeStampRts;
  uint8_t m_retryNo;
  Time m_delay;
  Mac8Address m_address;
};

class UanHeaderRcAck : public Header {
public:
  UanHeaderRcAck();
  ~UanHeaderRcAck() override;

  static TypeId GetTypeId();

  void SetFrameNo(uint8_t frameNo);
  void AddNackedFrame(uint8_t frame);

  const std::set<uint8_t> &GetNackedFrames() const;
  uint8_t GetFrameNo() const;
  uint8_t GetNoNacks() const;

  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;
  void Print(std::ostream &os) const override;
  TypeId GetInstanceTypeId() const override;

private:
  uint8_t m_frameNo;
  std::set<uint8_t> m_nackedFrames;
};

} // namespace ns3

#endif
