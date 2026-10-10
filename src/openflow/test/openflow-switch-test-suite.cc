
#include "ns3/openflow-interface.h"
#include "ns3/openflow-switch-net-device.h"
#include "ns3/test.h"

using namespace ns3;

class SwitchFlowTableTestCase : public TestCase {
public:
  SwitchFlowTableTestCase() : TestCase("Switch test case") {
    m_chain = chain_create();
  }

  ~SwitchFlowTableTestCase() override { chain_destroy(m_chain); }

private:
  void DoRun() override;

  sw_chain *m_chain;
};

void SwitchFlowTableTestCase::DoRun() {

  time_init();

  size_t actions_len = 0;
  int output_port = 0;

  Mac48Address dl_src("00:00:00:00:00:00");
  Mac48Address dl_dst("00:00:00:00:00:01");
  Ipv4Address nw_src("192.168.1.1");
  Ipv4Address nw_dst("192.168.1.2");
  int tp_src = 5000;
  int tp_dst = 80;

  sw_flow_key key;
  key.wildcards = 0;

  key.flow.in_port = htons(0);

  key.flow.dl_vlan = htons(OFP_VLAN_NONE);
  key.flow.dl_type = htons(ETH_TYPE_IP);
  key.flow.nw_proto = htons(IP_TYPE_UDP);

  key.flow.reserved = 0;
  key.flow.mpls_label1 = htonl(MPLS_INVALID_LABEL);
  key.flow.mpls_label2 = htonl(MPLS_INVALID_LABEL);

  dl_src.CopyTo(key.flow.dl_src);
  dl_dst.CopyTo(key.flow.dl_dst);

  key.flow.nw_src = htonl(nw_src.Get());
  key.flow.nw_dst = htonl(nw_dst.Get());

  key.flow.tp_src = htonl(tp_src);
  key.flow.tp_dst = htonl(tp_dst);

  ofp_flow_mod ofm;
  ofm.header.version = OFP_VERSION;
  ofm.header.type = OFPT_FLOW_MOD;
  ofm.header.length = htons(sizeof(ofp_flow_mod) + actions_len);
  ofm.command = htons(OFPFC_ADD);
  ofm.idle_timeout = htons(OFP_FLOW_PERMANENT);
  ofm.hard_timeout = htons(OFP_FLOW_PERMANENT);
  ofm.buffer_id = htonl(-1);
  ofm.priority = OFP_DEFAULT_PRIORITY;

  ofm.match.wildcards = key.wildcards;
  ofm.match.in_port = key.flow.in_port;
  memcpy(ofm.match.dl_src, key.flow.dl_src, sizeof ofm.match.dl_src);
  memcpy(ofm.match.dl_dst, key.flow.dl_dst, sizeof ofm.match.dl_dst);
  ofm.match.dl_vlan = key.flow.dl_vlan;
  ofm.match.dl_type = key.flow.dl_type;
  ofm.match.nw_proto = key.flow.nw_proto;
  ofm.match.nw_src = key.flow.nw_src;
  ofm.match.nw_dst = key.flow.nw_dst;
  ofm.match.tp_src = key.flow.tp_src;
  ofm.match.tp_dst = key.flow.tp_dst;
  ofm.match.mpls_label1 = key.flow.mpls_label1;
  ofm.match.mpls_label2 = key.flow.mpls_label1;

  sw_flow *flow = flow_alloc(actions_len);
  NS_TEST_ASSERT_MSG_NE(flow, 0, "Cannot allocate memory for the flow.");

  flow_extract_match(&flow->key, &ofm.match);

  flow->priority = flow->key.wildcards ? ntohs(ofm.priority) : -1;
  flow->idle_timeout = ntohs(ofm.idle_timeout);
  flow->hard_timeout = ntohs(ofm.hard_timeout);
  flow->used = flow->created = time_now();
  flow->sf_acts->actions_len = actions_len;
  flow->byte_count = 0;
  flow->packet_count = 0;
  memcpy(flow->sf_acts->actions, ofm.actions, actions_len);

  NS_TEST_ASSERT_MSG_EQ(chain_insert(m_chain, flow), 0,
                        "Flow table failed to insert Flow.");

  NS_TEST_ASSERT_MSG_NE(
      chain_lookup(m_chain, &key), 0,
      "Key provided doesn't match to the flow that was created from it.");

  dl_dst.CopyTo(key.flow.dl_src);
  dl_src.CopyTo(key.flow.dl_dst);
  key.flow.nw_src = htonl(nw_dst.Get());
  key.flow.nw_dst = htonl(nw_src.Get());
  key.flow.tp_src = htonl(tp_dst);
  key.flow.tp_dst = htonl(tp_src);

  NS_TEST_ASSERT_MSG_EQ(chain_lookup(m_chain, &key), 0,
                        "Key provided shouldn't match the flow but it does.");

  dl_dst.CopyTo(key.flow.dl_dst);
  dl_src.CopyTo(key.flow.dl_src);
  key.flow.nw_src = htonl(nw_src.Get());
  key.flow.nw_dst = htonl(nw_dst.Get());
  key.flow.tp_src = htonl(tp_src);
  key.flow.tp_dst = htonl(tp_dst);

  ofp_action_output acts[1];
  acts[0].type = htons(OFPAT_OUTPUT);
  acts[0].len = htons(sizeof(ofp_action_output));
  acts[0].port = output_port;

  uint16_t priority = key.wildcards ? ntohs(ofm.priority) : -1;
  NS_TEST_ASSERT_MSG_EQ(chain_modify(m_chain, &key, priority, false,
                                     (const ofp_action_header *)acts,
                                     sizeof(acts)),
                        1, "Flow table failed to modify Flow.");

  NS_TEST_ASSERT_MSG_EQ(chain_delete(m_chain, &key, output_port, 0, 0), 1,
                        "Flow table failed to delete Flow.");
  NS_TEST_ASSERT_MSG_EQ(chain_lookup(m_chain, &key), 0,
                        "Key provided shouldn't match the flow but it does.");
}

class SwitchTestSuite : public TestSuite {
public:
  SwitchTestSuite();
};

SwitchTestSuite::SwitchTestSuite() : TestSuite("openflow", UNIT) {
  AddTestCase(new SwitchFlowTableTestCase, TestCase::QUICK);
}

static SwitchTestSuite switchTestSuite;
