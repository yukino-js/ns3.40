
#ifndef DPDK_NET_DEVICE_H
#define DPDK_NET_DEVICE_H

#include "fd-net-device.h"

#include <rte_cycles.h>
#include <rte_mempool.h>
#include <rte_ring.h>

struct rte_eth_dev_tx_buffer;
struct rte_mbuf;

namespace ns3 {

class DpdkNetDevice : public FdNetDevice {
public:
  static TypeId GetTypeId();

  DpdkNetDevice();

  ~DpdkNetDevice() override;

  void CheckAllPortsLinkStatus();

  void InitDpdk(int argc, char **argv, std::string dpdkDriver);

  void SetDeviceName(std::string deviceName);

  static void SignalHandler(int signum);

  static int LaunchCore(void *arg);

  void HandleTx();

  void HandleRx();

  bool IsLinkUp() const override;

  void FreeBuffer(uint8_t *buf) override;

  uint8_t *AllocateBuffer(size_t len) override;

protected:
  ssize_t Write(uint8_t *buffer, size_t length) override;

  uint16_t m_portId;

  std::string m_deviceName;

private:
  void DoFinishStoppingDevice() override;
  static volatile bool m_forceQuit;

  struct rte_mempool *m_mempool;

  struct rte_eth_dev_tx_buffer *m_txBuffer;

  struct rte_eth_dev_tx_buffer *m_rxBuffer;

  EventId m_txEvent;

  Time m_txTimeout;

  uint32_t m_maxRxPktBurst;

  uint32_t m_maxTxPktBurst;

  uint32_t m_mempoolCacheSize;

  uint16_t m_nbRxDesc;

  uint16_t m_nbTxDesc;
};

} // namespace ns3

#endif
