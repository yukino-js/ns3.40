
#ifndef OPENFLOW_INTERFACE_H
#define OPENFLOW_INTERFACE_H

#include <assert.h>
#include <errno.h>

#include "ns3/address.h"
#include "ns3/log.h"
#include "ns3/mac48-address.h"
#include "ns3/net-device.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

#include <limits>
#include <map>
#include <set>

#include "openflow/ericsson-ext.h"
#include "openflow/nicira-ext.h"
#include "openflow/openflow.h"

extern "C" {
#define private _private
#define delete _delete
#define list List

#include "openflow/private/csum.h"
#include "openflow/private/poll-loop.h"
#include "openflow/private/rconn.h"
#include "openflow/private/stp.h"
#include "openflow/private/vconn.h"
#include "openflow/private/xtoxll.h"

#include "openflow/private/chain.h"
#include "openflow/private/datapath.h"
#include "openflow/private/table.h"
uint32_t save_buffer(ofpbuf *);
ofpbuf *retrieve_buffer(uint32_t id);
void discard_buffer(uint32_t id);
#include "openflow/private/dp_act.h"
void set_vlan_vid(ofpbuf *buffer, sw_flow_key *key,
                  const ofp_action_header *ah);
void set_vlan_pcp(ofpbuf *buffer, sw_flow_key *key,
                  const ofp_action_header *ah);
void strip_vlan(ofpbuf *buffer, sw_flow_key *key, const ofp_action_header *ah);
void set_dl_addr(ofpbuf *buffer, sw_flow_key *key, const ofp_action_header *ah);
void set_nw_addr(ofpbuf *buffer, sw_flow_key *key, const ofp_action_header *ah);
void set_tp_port(ofpbuf *buffer, sw_flow_key *key, const ofp_action_header *ah);
void set_mpls_label(ofpbuf *buffer, sw_flow_key *key,
                    const ofp_action_header *ah);
void set_mpls_exp(ofpbuf *buffer, sw_flow_key *key,
                  const ofp_action_header *ah);
#include "openflow/private/pt_act.h"

#undef list
#undef private
#undef delete
}

#ifndef OFP_SUPPORTED_CAPABILITIES
#define OFP_SUPPORTED_CAPABILITIES                                             \
  (OFPC_FLOW_STATS | OFPC_TABLE_STATS | OFPC_PORT_STATS | OFPC_MULTI_PHY_TX |  \
   OFPC_VPORT_TABLE)
#endif

#ifndef OFP_SUPPORTED_ACTIONS
#define OFP_SUPPORTED_ACTIONS                                                  \
  ((1 << OFPAT_OUTPUT) | (1 << OFPAT_SET_VLAN_VID) |                           \
   (1 << OFPAT_SET_VLAN_PCP) | (1 << OFPAT_STRIP_VLAN) |                       \
   (1 << OFPAT_SET_DL_SRC) | (1 << OFPAT_SET_DL_DST) |                         \
   (1 << OFPAT_SET_NW_SRC) | (1 << OFPAT_SET_NW_DST) |                         \
   (1 << OFPAT_SET_TP_SRC) | (1 << OFPAT_SET_TP_DST) |                         \
   (1 << OFPAT_SET_MPLS_LABEL) | (1 << OFPAT_SET_MPLS_EXP))
#endif

#ifndef OFP_SUPPORTED_VPORT_TABLE_ACTIONS
#define OFP_SUPPORTED_VPORT_TABLE_ACTIONS                                      \
  ((1 << OFPPAT_OUTPUT) | (1 << OFPPAT_POP_MPLS) | (1 << OFPPAT_PUSH_MPLS) |   \
   (1 << OFPPAT_SET_MPLS_LABEL) | (1 << OFPPAT_SET_MPLS_EXP))
#endif

namespace ns3 {

class OpenFlowSwitchNetDevice;

namespace ofi {

struct Port {
  Port()
      : config(0), state(0), netdev(nullptr), rx_packets(0), tx_packets(0),
        rx_bytes(0), tx_bytes(0), tx_dropped(0), mpls_ttl0_dropped(0) {}

  uint32_t config;
  uint32_t state;
  Ptr<NetDevice> netdev;
  unsigned long long int rx_packets;
  unsigned long long int tx_packets;
  unsigned long long int rx_bytes;
  unsigned long long int tx_bytes;
  unsigned long long int tx_dropped;
  unsigned long long int mpls_ttl0_dropped;
};

class Stats {
public:
  Stats(ofp_stats_types _type, size_t body_len);

  int DoInit(const void *body, int body_len, void **state);

  int DoDump(Ptr<OpenFlowSwitchNetDevice> swtch, void *state, ofpbuf *buffer);

  void DoCleanup(void *state);

  struct FlowStatsState {
    int table_idx;
    sw_table_position position;
    ofp_flow_stats_request rq;
    time_t now;

    ofpbuf *buffer;
  };

  struct PortStatsState {
    uint32_t num_ports;
    uint32_t *ports;
  };

  ofp_stats_types type;

private:
  int DescStatsDump(void *state, ofpbuf *buffer);

  int FlowStatsInit(const void *body, int body_len, void **state);
  int AggregateStatsInit(const void *body, int body_len, void **state);
  int PortStatsInit(const void *body, int body_len, void **state);

  int (*FlowDumpCallback)(sw_flow *flow, void *state);
  int (*AggregateDumpCallback)(sw_flow *flow, void *state);

  int FlowStatsDump(Ptr<OpenFlowSwitchNetDevice> dp, FlowStatsState *state,
                    ofpbuf *buffer);
  int AggregateStatsDump(Ptr<OpenFlowSwitchNetDevice> dp,
                         ofp_aggregate_stats_request *state, ofpbuf *buffer);
  int TableStatsDump(Ptr<OpenFlowSwitchNetDevice> dp, void *state,
                     ofpbuf *buffer);
  int PortStatsDump(Ptr<OpenFlowSwitchNetDevice> dp, PortStatsState *state,
                    ofpbuf *buffer);
  int PortTableStatsDump(Ptr<OpenFlowSwitchNetDevice> dp, void *state,
                         ofpbuf *buffer);
};

struct Action {
  static bool IsValidType(ofp_action_type type);

  static uint16_t Validate(ofp_action_type type, size_t len,
                           const sw_flow_key *key, const ofp_action_header *ah);

  static void Execute(ofp_action_type type, ofpbuf *buffer, sw_flow_key *key,
                      const ofp_action_header *ah);
};

struct VPortAction {
  static bool IsValidType(ofp_vport_action_type type);

  static uint16_t Validate(ofp_vport_action_type type, size_t len,
                           const ofp_action_header *ah);

  static void Execute(ofp_vport_action_type type, ofpbuf *buffer,
                      const sw_flow_key *key, const ofp_action_header *ah);
};

struct EricssonAction {
  static bool IsValidType(er_action_type type);

  static uint16_t Validate(er_action_type type, size_t len);

  static void Execute(er_action_type type, ofpbuf *buffer,
                      const sw_flow_key *key, const er_action_header *ah);
};

struct StatsDumpCallback {
  bool done;
  ofp_stats_request *rq;
  Stats *s;
  void *state;
  Ptr<OpenFlowSwitchNetDevice> swtch;
};

struct SwitchPacketMetadata {
  Ptr<Packet> packet;
  ofpbuf *buffer;
  uint16_t protocolNumber;
  Address src;
  Address dst;
};

class Controller : public Object {
public:
  static TypeId GetTypeId();
  ~Controller() override;

  virtual void AddSwitch(Ptr<OpenFlowSwitchNetDevice> swtch);

  virtual void ReceiveFromSwitch(Ptr<OpenFlowSwitchNetDevice> swtch,
                                 ofpbuf *buffer) {}

  void StartDump(StatsDumpCallback *cb);

protected:
  virtual void SendToSwitch(Ptr<OpenFlowSwitchNetDevice> swtch, void *msg,
                            size_t length);

  ofp_flow_mod *BuildFlow(sw_flow_key key, uint32_t buffer_id, uint16_t command,
                          void *acts, size_t actions_len, int idle_timeout,
                          int hard_timeout);

  uint8_t GetPacketType(ofpbuf *buffer);

  typedef std::set<Ptr<OpenFlowSwitchNetDevice>> Switches_t;
  Switches_t m_switches;
};

class DropController : public Controller {
public:
  static TypeId GetTypeId();

  void ReceiveFromSwitch(Ptr<OpenFlowSwitchNetDevice> swtch,
                         ofpbuf *buffer) override;
};

class LearningController : public Controller {
public:
  static TypeId GetTypeId();

  ~LearningController() override { m_learnState.clear(); }

  void ReceiveFromSwitch(Ptr<OpenFlowSwitchNetDevice> swtch,
                         ofpbuf *buffer) override;

protected:
  struct LearnedState {
    uint32_t port;
  };

  Time m_expirationTime;
  typedef std::map<Mac48Address, LearnedState> LearnState_t;
  LearnState_t m_learnState;
};

void ExecuteActions(Ptr<OpenFlowSwitchNetDevice> swtch, uint64_t packet_uid,
                    ofpbuf *buffer, sw_flow_key *key,
                    const ofp_action_header *actions, size_t actions_len,
                    int ignore_no_fwd);

uint16_t ValidateActions(const sw_flow_key *key,
                         const ofp_action_header *actions, size_t actions_len);

void ExecuteVPortActions(Ptr<OpenFlowSwitchNetDevice> swtch,
                         uint64_t packet_uid, ofpbuf *buffer, sw_flow_key *key,
                         const ofp_action_header *actions, size_t actions_len);

uint16_t ValidateVPortActions(const ofp_action_header *actions,
                              size_t actions_len);

void ExecuteVendor(ofpbuf *buffer, const sw_flow_key *key,
                   const ofp_action_header *ah);

uint16_t ValidateVendor(const sw_flow_key *key, const ofp_action_header *ah,
                        uint16_t len);

#define PKT_BUFFER_BITS 8
#define N_PKT_BUFFERS (1 << PKT_BUFFER_BITS)
#define PKT_BUFFER_MASK (N_PKT_BUFFERS - 1)
#define PKT_COOKIE_BITS (32 - PKT_BUFFER_BITS)

} // namespace ofi

} // namespace ns3

#endif
