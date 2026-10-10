// @ts-check

export const DEFAULT_DURATION = 20;

export const DEFAULT_N_LEAF = 3;

export const DEFAULT_SIM_SEED = 42;

export const DEFAULT_PROTOCOLS = [
  "TcpSwift",
  "TcpNewReno",
  "TcpCubic",
  "TcpBbr",
];

export const LOG_ROOT = "./logs";

export const SCENARIOS = [
  ["intra_rack_10g", "25Gbps", "10Gbps", "1us", "2us"],
  ["intra_rack_25g", "25Gbps", "25Gbps", "1us", "2us"],
  ["leaf_spine_20g", "50Gbps", "20Gbps", "2us", "5us"],
  ["leaf_spine_50g", "50Gbps", "50Gbps", "2us", "5us"],
  ["oversub_4to1_10g", "10Gbps", "2.5Gbps", "2us", "5us"],
  ["oversub_4to1_40g", "40Gbps", "10Gbps", "2us", "5us"],
  ["oversub_2to1_25g", "25Gbps", "12.5Gbps", "2us", "5us"],
  ["oversub_2to1_50g", "50Gbps", "25Gbps", "2us", "5us"],
  ["congested_light", "10Gbps", "5Gbps", "2us", "5us"],
  ["congested_medium", "10Gbps", "2Gbps", "2us", "5us"],
  ["congested_heavy", "10Gbps", "500Mbps", "2us", "5us"],
  ["cross_pod_10g", "25Gbps", "10Gbps", "5us", "50us"],
  ["cross_pod_20g", "50Gbps", "20Gbps", "5us", "50us"],
  ["cross_dc_wan", "10Gbps", "1Gbps", "10us", "5ms"],
  ["rdma_like_25g", "25Gbps", "25Gbps", "500ns", "1us"],
  ["rdma_like_50g", "50Gbps", "50Gbps", "500ns", "1us"],
  ["mixed_small_flow", "10Gbps", "2Gbps", "2us", "10us"],
  ["mixed_large_flow", "50Gbps", "12.5Gbps", "2us", "10us"],
  ["asymmetric_high", "50Gbps", "1Gbps", "1us", "10us"],
  ["symmetric_low", "1Gbps", "1Gbps", "5us", "20us"],
  ["dc_100m", "1Gbps", "100Mbps", "2us", "5us"],
  ["dc_500m", "1Gbps", "500Mbps", "2us", "5us"],
  ["dc_100g", "100Gbps", "100Gbps", "1us", "2us"],
  ["dc_oversub_10to1", "10Gbps", "1Gbps", "2us", "5us"],
  ["wifi_ac", "1Gbps", "400Mbps", "1ms", "5ms"],
  ["wifi_ax", "1Gbps", "600Mbps", "1ms", "3ms"],
  ["wifi_n", "100Mbps", "50Mbps", "2ms", "10ms"],
  ["wifi_legacy", "100Mbps", "10Mbps", "5ms", "20ms"],
  ["lte_good", "100Mbps", "50Mbps", "5ms", "20ms"],
  ["lte_poor", "50Mbps", "10Mbps", "10ms", "50ms"],
  ["nr_5g_embb", "1Gbps", "500Mbps", "1ms", "5ms"],
  ["nr_5g_edge", "500Mbps", "100Mbps", "2ms", "10ms"],
  ["wan_metro", "10Gbps", "1Gbps", "100us", "2ms"],
  ["wan_longhaul", "10Gbps", "1Gbps", "500us", "25ms"],
  ["satellite_leo", "500Mbps", "150Mbps", "2ms", "25ms"],
  ["satellite_geo", "50Mbps", "10Mbps", "10ms", "300ms"],
];

export const SCENARIO_NAMES = SCENARIOS.map(([name]) => name);

export const SCENARIO_LINKS = new Map(
  SCENARIOS.map(([name, access, bottleneck, accessDelay, bottleneckDelay]) => [
    name,
    [access, bottleneck, accessDelay, bottleneckDelay],
  ]),
);

export const S_ORDER = [
  ["S1", "intra_rack_10g"],
  ["S2", "intra_rack_25g"],
  ["S3", "leaf_spine_20g"],
  ["S4", "asymmetric_high"],
  ["S5", "congested_heavy"],
  ["S6", "symmetric_low"],
  ["S7", "dc_500m"],
  ["S8", "dc_100m"],
  ["S9", "cross_dc_wan"],
  ["S10", "wan_metro"],
  ["S11", "wifi_ac"],
  ["S12", "wifi_ax"],
  ["S13", "wifi_n"],
  ["S14", "wifi_legacy"],
  ["S15", "nr_5g_embb"],
  ["S16", "nr_5g_edge"],
  ["S17", "lte_good"],
  ["S18", "lte_poor"],
  ["S19", "satellite_geo"],
];

export const SCENARIO_BY_SID = new Map(S_ORDER);

export const SID_BY_SCENARIO = new Map(
  S_ORDER.map(([sid, scenario]) => [scenario, sid]),
);

export const UDP_PAIRED_SIDS = [
  "S1",
  "S2",
  "S3",
  "S4",
  "S5",
  "S6",
  "S7",
  "S8",
  "S10",
  "S13",
  "S14",
  "S16",
  "S17",
  "S18",
  "S19",
];

export const PROTOCOL_ORDER = ["TcpSwift", "TcpNewReno", "TcpCubic", "TcpBbr"];

export const PROTOCOL_COLORS_BAR = ["#dd0031", "#61dafb", "#42b883", "#61dafb"];

export const PROTOCOL_COLORS_MAP = {
  TcpSwift: "#dd0031",
  TcpNewReno: "#61dafb",
  TcpCubic: "#42b883",
  TcpBbr: "#61dafb",
};

export const FLOW_COLORS = {
  TcpNewReno: "#61dafb",
  TcpCubic: "#673ab8",
  TcpBbr: "#42b883",
  TcpSwift: "#dd0031",
};

export const PROTOCOL_LABEL = {
  TcpSwift: "Swift",
  TcpNewReno: "NewReno",
  TcpCubic: "CUBIC",
  TcpBbr: "BBR",
};

export const DOCS_PLOT_COLORS = {
  TcpSwift: "#61DAFB",
  TcpNewReno: "#673AB8",
  TcpCubic: "#42B883",
  TcpBbr: "#DD0031",
};
