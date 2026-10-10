
#ifndef NETMAP_NET_DEVICE_H
#define NETMAP_NET_DEVICE_H

#include "fd-net-device.h"

#include "ns3/net-device-queue-interface.h"

#include <atomic>
#include <mutex>
#include <net/netmap_user.h>
#include <thread>

namespace ns3 {

class NetDeviceQueueLock : public NetDeviceQueue {
public:
  static TypeId GetTypeId();

  NetDeviceQueueLock();
  virtual ~NetDeviceQueueLock();

  virtual void Start();

  virtual void Stop();

  virtual void Wake();

  virtual bool IsStopped() const;

  virtual void NotifyQueuedBytes(uint32_t bytes);

  virtual void NotifyTransmittedBytes(uint32_t bytes);

private:
  mutable std::mutex m_mutex;
};

class NetmapNetDeviceFdReader : public FdReader {
public:
  NetmapNetDeviceFdReader();

  void SetBufferSize(uint32_t bufferSize);

  void SetNetmapIfp(struct netmap_if *nifp);

private:
  FdReader::Data DoRead();

  uint32_t m_bufferSize;
  struct netmap_if *m_nifp;
};

class NetmapNetDevice : public FdNetDevice {
public:
  static TypeId GetTypeId();

  NetmapNetDevice();
  virtual ~NetmapNetDevice();

  uint32_t GetBytesInNetmapTxRing();

  int GetSpaceInNetmapTxRing() const;

  void SetNetDeviceQueue(Ptr<NetDeviceQueue> queue);

  void SetNetmapInterfaceRepresentation(struct netmap_if *nifp);

  void SetTxRingsInfo(uint32_t nTxRings, uint32_t nTxRingsSlots);

  void SetRxRingsInfo(uint32_t nRxRings, uint32_t nRxRingsSlots);

  virtual ssize_t Write(uint8_t *buffer, size_t length);

private:
  Ptr<FdReader> DoCreateFdReader();
  void DoFinishStartingDevice();
  void DoFinishStoppingDevice();

  virtual void SyncAndNotifyQueue();

  struct netmap_if *m_nifp;
  uint32_t m_nTxRings;
  uint32_t m_nTxRingsSlots;
  uint32_t m_nRxRings;
  uint32_t m_nRxRingsSlots;
  Ptr<NetDeviceQueue> m_queue;
  uint32_t m_totalQueuedBytes;
  std::thread m_syncAndNotifyQueueThread;
  std::atomic<bool> m_syncAndNotifyQueueThreadRun;
  uint8_t m_syncAndNotifyQueuePeriod;
};

} // namespace ns3

#endif
