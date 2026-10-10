#ifndef QUEUE_ITEM_H
#define QUEUE_ITEM_H

#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/simple-ref-count.h"
#include <ns3/address.h>

namespace ns3 {

class Packet;

class QueueItem : public SimpleRefCount<QueueItem> {
public:
  QueueItem(Ptr<Packet> p);

  virtual ~QueueItem();

  QueueItem() = delete;
  QueueItem(const QueueItem &) = delete;
  QueueItem &operator=(const QueueItem &) = delete;

  Ptr<Packet> GetPacket() const;

  virtual uint32_t GetSize() const;

  enum Uint8Values { IP_DSFIELD };

  virtual bool GetUint8Value(Uint8Values field, uint8_t &value) const;

  virtual void Print(std::ostream &os) const;

  typedef void (*TracedCallback)(Ptr<const QueueItem> item);

private:
  Ptr<Packet> m_packet;
};

std::ostream &operator<<(std::ostream &os, const QueueItem &item);

class QueueDiscItem : public QueueItem {
public:
  QueueDiscItem(Ptr<Packet> p, const Address &addr, uint16_t protocol);

  ~QueueDiscItem() override;

  QueueDiscItem() = delete;
  QueueDiscItem(const QueueDiscItem &) = delete;
  QueueDiscItem &operator=(const QueueDiscItem &) = delete;

  Address GetAddress() const;

  uint16_t GetProtocol() const;

  uint8_t GetTxQueueIndex() const;

  void SetTxQueueIndex(uint8_t txq);

  Time GetTimeStamp() const;

  void SetTimeStamp(Time t);

  virtual void AddHeader() = 0;

  void Print(std::ostream &os) const override;

  virtual bool Mark() = 0;

  virtual uint32_t Hash(uint32_t perturbation = 0) const;

private:
  Address m_address;
  uint16_t m_protocol;
  uint8_t m_txq;
  Time m_tstamp;
};

} // namespace ns3

#endif
