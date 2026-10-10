

#ifndef OPENFLOW_SWITCH_NET_DEVICE_H
#define OPENFLOW_SWITCH_NET_DEVICE_H

#include "openflow-interface.h"

#include "ns3/arp-header.h"
#include "ns3/arp-l3-protocol.h"
#include "ns3/bridge-channel.h"
#include "ns3/enum.h"
#include "ns3/ethernet-header.h"
#include "ns3/integer.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/log.h"
#include "ns3/mac48-address.h"
#include "ns3/node.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/tcp-header.h"
#include "ns3/udp-header.h"
#include "ns3/uinteger.h"

#include <map>
#include <set>

namespace ns3 {

class OpenFlowSwitchNetDevice : public NetDevice {
public:
  static TypeId GetTypeId();

  static const char *GetManufacturerDescription();
  static const char *GetHardwareDescription();
  static const char *GetSoftwareDescription();
  static const char *GetSerialNumber();

  OpenFlowSwitchNetDevice();
  ~OpenFlowSwitchNetDevice() override;

  void SetController(Ptr<ofi::Controller> c);

  int AddSwitchPort(Ptr<NetDevice> switchPort);

  int AddVPort(const ofp_vport_mod *ovpm);

  int StatsDump(ofi::StatsDumpCallback *cb_);

  void StatsDone(ofi::StatsDumpCallback *cb_);

  void DoOutput(uint32_t packet_uid, int in_port, size_t max_len, int out_port,
                bool ignore_no_fwd);

  int ForwardControlInput(const void *msg, size_t length);

  sw_chain *GetChain();

  uint32_t GetNSwitchPorts() const;

  int GetSwitchPortIndex(ofi::Port p);

  ofi::Port GetSwitchPort(uint32_t n) const;

  vport_table_t GetVPortTable();

  void SetIfIndex(const uint32_t index) override;
  uint32_t GetIfIndex() const override;
  Ptr<Channel> GetChannel() const override;
  void SetAddress(Address address) override;
  Address GetAddress() const override;
  bool SetMtu(const uint16_t mtu) override;
  uint16_t GetMtu() const override;
  bool IsLinkUp() const override;
  void AddLinkChangeCallback(Callback<void> callback) override;
  bool IsBroadcast() const override;
  Address GetBroadcast() const override;
  bool IsMulticast() const override;
  Address GetMulticast(Ipv4Address multicastGroup) const override;
  bool IsPointToPoint() const override;
  bool IsBridge() const override;
  bool Send(Ptr<Packet> packet, const Address &dest,
            uint16_t protocolNumber) override;
  bool SendFrom(Ptr<Packet> packet, const Address &source, const Address &dest,
                uint16_t protocolNumber) override;
  Ptr<Node> GetNode() const override;
  void SetNode(Ptr<Node> node) override;
  bool NeedsArp() const override;
  void SetReceiveCallback(NetDevice::ReceiveCallback cb) override;
  void SetPromiscReceiveCallback(NetDevice::PromiscReceiveCallback cb) override;
  bool SupportsSendFrom() const override;
  Address GetMulticast(Ipv6Address addr) const override;

protected:
  void DoDispose() override;

  void ReceiveFromDevice(Ptr<NetDevice> netdev, Ptr<const Packet> packet,
                         uint16_t protocol, const Address &src,
                         const Address &dst, PacketType packetType);

  ofpbuf *BufferFromPacket(Ptr<const Packet> packet, Address src, Address dst,
                           int mtu, uint16_t protocol);

private:
  int AddFlow(const ofp_flow_mod *ofm);

  int ModFlow(const ofp_flow_mod *ofm);

  int OutputAll(uint32_t packet_uid, int in_port, bool flood);

  void OutputPacket(uint32_t packet_uid, int out_port);

  void OutputPort(uint32_t packet_uid, int in_port, int out_port,
                  bool ignore_no_fwd);

  void OutputControl(uint32_t packet_uid, int in_port, size_t max_len,
                     int reason);

  void SendErrorMsg(uint16_t type, uint16_t code, const void *data, size_t len);

  void SendFeaturesReply();

  void SendFlowExpired(sw_flow *flow, ofp_flow_expired_reason reason);

  void SendPortStatus(ofi::Port p, uint8_t status);

  void SendVPortTableFeatures();

  int SendOpenflowBuffer(ofpbuf *buffer);

  void RunThroughFlowTable(uint32_t packet_uid, int port,
                           bool send_to_controller = true);

  int RunThroughVPortTable(uint32_t packet_uid, int port, uint32_t vport);

  void FlowTableLookup(sw_flow_key key, ofpbuf *buffer, uint32_t packet_uid,
                       int port, bool send_to_controller);

  int UpdatePortStatus(ofi::Port &p);

  void FillPortDesc(ofi::Port p, ofp_phy_port *desc);

  void *MakeOpenflowReply(size_t openflow_len, uint8_t type, ofpbuf **bufferp);

  int ReceivePortMod(const void *msg);
  int ReceiveFeaturesRequest(const void *msg);
  int ReceiveGetConfigRequest(const void *msg);
  int ReceiveSetConfig(const void *msg);
  int ReceivePacketOut(const void *msg);
  int ReceiveFlow(const void *msg);
  int ReceiveStatsRequest(const void *msg);
  int ReceiveEchoRequest(const void *msg);
  int ReceiveEchoReply(const void *msg);
  int ReceiveVPortMod(const void *msg);
  int ReceiveVPortTableFeaturesRequest(const void *msg);

  NetDevice::ReceiveCallback m_rxCallback;
  NetDevice::PromiscReceiveCallback m_promiscRxCallback;

  Mac48Address m_address;
  Ptr<Node> m_node;
  Ptr<BridgeChannel> m_channel;
  uint32_t m_ifIndex;
  uint16_t m_mtu;

  typedef std::map<uint32_t, ofi::SwitchPacketMetadata> PacketData_t;
  PacketData_t m_packetData;

  typedef std::vector<ofi::Port> Ports_t;
  Ports_t m_ports;

  Ptr<ofi::Controller> m_controller;

  uint64_t m_id;
  Time m_lookupDelay;

  Time m_lastExecute;
  uint16_t m_flags;
  uint16_t m_missSendLen;

  sw_chain *m_chain;
  vport_table_t m_vportTable;
};

} // namespace ns3

#endif
