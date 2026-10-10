// @ts-check


import { readFileSync, readdirSync, statSync } from "node:fs";
import path from "node:path";
import { XMLParser } from "fast-xml-parser";
import { mean, sum } from "./stats.js";

export const RUN_NAME_RE = /^(.+)_(Tcp[A-Za-z0-9]+)(?:_s(\d+))?$/;


export function parseRunName(basename) {
  const match = RUN_NAME_RE.exec(basename);
  if (!match) return null;
  return {
    scenario: match[1],
    protocol: match[2],
    seed: match[3] ? Number.parseInt(match[3], 10) : null,
  };
}

export class FlowData {
  flowId;
  srcAddr;
  dstAddr;
  protocol;
  txBytes;
  rxBytes;
  txPackets;
  rxPackets;
  lostPackets;
  delaySumNs;
  jitterSumNs;
  timeFirstTxNs;
  timeLastRxNs;
  durationNs;

  constructor(init) {
    this.flowId = init.flowId;
    this.srcAddr = init.srcAddr;
    this.dstAddr = init.dstAddr;
    this.protocol = init.protocol;
    this.txBytes = init.txBytes;
    this.rxBytes = init.rxBytes;
    this.txPackets = init.txPackets;
    this.rxPackets = init.rxPackets;
    this.lostPackets = init.lostPackets;
    this.delaySumNs = init.delaySumNs;
    this.jitterSumNs = init.jitterSumNs;
    this.timeFirstTxNs = init.timeFirstTxNs;
    this.timeLastRxNs = init.timeLastRxNs;
    this.durationNs = init.durationNs;
  }

  get throughputMbps() {
    if (this.durationNs > 0) {
      return ((this.rxBytes * 8) / (this.durationNs / 1e9)) / 1e6;
    }
    return 0;
  }

  get avgDelayMs() {
    if (this.rxPackets > 0) {
      return this.delaySumNs / this.rxPackets / 1e6;
    }
    return 0;
  }

  get avgJitterMs() {
    if (this.rxPackets > 1) {
      return this.jitterSumNs / (this.rxPackets - 1) / 1e6;
    }
    return 0;
  }

  get lossRate() {
    if (this.txPackets > 0) {
      return (this.lostPackets / this.txPackets) * 100;
    }
    return 0;
  }
}

export class ScenarioResult {
  scenario;
  protocol;
  seed;
  sourcePath;
  flows;

  constructor(init) {
    this.scenario = init.scenario;
    this.protocol = init.protocol;
    this.seed = init.seed ?? null;
    this.sourcePath = init.sourcePath ?? "";
    this.flows = init.flows ?? [];
  }

  get forwardFlows() {
    return this.flows.filter(
      (flow) =>
        flow.protocol === 6 &&
        flow.srcAddr.startsWith("10.1.") &&
        flow.dstAddr.startsWith("10.2."),
    );
  }

  get totalThroughputMbps() {
    const forward = this.forwardFlows;
    if (forward.length === 0) return 0;
    const firstTx = Math.min(...forward.map((flow) => flow.timeFirstTxNs));
    const lastRx = Math.max(...forward.map((flow) => flow.timeLastRxNs));
    const durationNs = lastRx - firstTx;
    if (durationNs <= 0) return 0;
    return (sum(forward.map((flow) => flow.rxBytes)) * 8) / (durationNs / 1e9) / 1e6;
  }

  get avgDelayMs() {
    const receiving = this.forwardFlows.filter((flow) => flow.rxPackets > 0);
    return receiving.length > 0
      ? mean(receiving.map((flow) => flow.avgDelayMs))
      : 0;
  }

  get avgJitterMs() {
    const receiving = this.forwardFlows.filter((flow) => flow.rxPackets > 1);
    return receiving.length > 0
      ? mean(receiving.map((flow) => flow.avgJitterMs))
      : 0;
  }

  get totalLossRate() {
    const forward = this.forwardFlows;
    const txPackets = sum(forward.map((flow) => flow.txPackets));
    const lostPackets = sum(forward.map((flow) => flow.lostPackets));
    return txPackets > 0 ? (lostPackets / txPackets) * 100 : 0;
  }

  get jainFairness() {
    const throughputs = this.forwardFlows.map((flow) => flow.throughputMbps);
    const denominator =
      throughputs.length * sum(throughputs.map((value) => value * value));
    if (throughputs.length === 0 || denominator === 0) return 0;
    return sum(throughputs) ** 2 / denominator;
  }
}

export class AggregatedResult {
  scenario;
  protocol;
  totalThroughputMbps;
  avgDelayMs;
  avgJitterMs;
  totalLossRate;
  seedCount;

  constructor(init) {
    this.scenario = init.scenario;
    this.protocol = init.protocol;
    this.totalThroughputMbps = init.totalThroughputMbps;
    this.avgDelayMs = init.avgDelayMs;
    this.avgJitterMs = init.avgJitterMs;
    this.totalLossRate = init.totalLossRate;
    this.seedCount = init.seedCount;
  }
}


const xmlParser = new XMLParser({
  ignoreAttributes: false,
  attributeNamePrefix: "",
  parseAttributeValue: false,
  parseTagValue: false,
  trimValues: true,
  isArray: (name) => name === "Flow",
});

function parseNsTime(text) {
  if (!text) return 0;
  const cleaned = text.replace(/^\+/, "").replace(/\+$/, "").replace(/ns/g, "");
  const value = Number.parseFloat(cleaned);
  return Number.isFinite(value) ? value : 0;
}

function intAttribute(attributes, name) {
  const raw = attributes[name];
  const value = Number.parseInt(String(raw ?? "0"), 10);
  return Number.isFinite(value) ? value : 0;
}

function stringAttribute(attributes, name) {
  const raw = attributes[name];
  return typeof raw === "string" ? raw : "";
}

function child(parent, key) {
  if (parent === null || typeof parent !== "object") return undefined;
  return (parent)[key];
}

function asAttributes(value) {
  return value !== null && typeof value === "object"
    ? (value)
    : {};
}

function asArray(value) {
  if (Array.isArray(value)) return value;
  return value === undefined ? [] : [value];
}

export function parseFlowMonitor(filepath) {
  const document = xmlParser.parse(readFileSync(filepath, "utf8"));
  const monitor = asAttributes(child(document, "FlowMonitor"));

  const flowInfo = new Map();
  const classifier = child(monitor, "Ipv4FlowClassifier");
  for (const element of asArray(child(classifier, "Flow"))) {
    const attributes = asAttributes(element);
    flowInfo.set(intAttribute(attributes, "flowId"), {
      srcAddr: stringAttribute(attributes, "sourceAddress"),
      dstAddr: stringAttribute(attributes, "destinationAddress"),
      protocol: intAttribute(attributes, "protocol"),
    });
  }

  const stats = child(monitor, "FlowStats");
  const flows = [];
  for (const element of asArray(child(stats, "Flow"))) {
    const attributes = asAttributes(element);
    const flowId = intAttribute(attributes, "flowId");
    const info = flowInfo.get(flowId) ?? {
      srcAddr: "",
      dstAddr: "",
      protocol: 0,
    };
    const firstTx = parseNsTime(
      stringAttribute(attributes, "timeFirstTxPacket") || "0ns",
    );
    const lastRx = parseNsTime(
      stringAttribute(attributes, "timeLastRxPacket") || "0ns",
    );

    flows.push(
      new FlowData({
        flowId,
        srcAddr: info.srcAddr,
        dstAddr: info.dstAddr,
        protocol: info.protocol,
        txBytes: intAttribute(attributes, "txBytes"),
        rxBytes: intAttribute(attributes, "rxBytes"),
        txPackets: intAttribute(attributes, "txPackets"),
        rxPackets: intAttribute(attributes, "rxPackets"),
        lostPackets: intAttribute(attributes, "lostPackets"),
        delaySumNs: parseNsTime(stringAttribute(attributes, "delaySum") || "0ns"),
        jitterSumNs: parseNsTime(
          stringAttribute(attributes, "jitterSum") || "0ns",
        ),
        timeFirstTxNs: firstTx,
        timeLastRxNs: lastRx,
        durationNs: lastRx > firstTx ? lastRx - firstTx : 0,
      }),
    );
  }
  return flows;
}

function optionalAttribute(attributes, name) {
  const raw = attributes[name];
  return typeof raw === "string" ? raw : "";
}

export function forwardKpi(filepath) {
  const document = xmlParser.parse(readFileSync(filepath, "utf8"));
  const monitor = asAttributes(child(document, "FlowMonitor"));

  const forwardIds = new Set();
  const ports = new Set();
  const classifier = child(monitor, "Ipv4FlowClassifier");
  for (const element of asArray(child(classifier, "Flow"))) {
    const attributes = asAttributes(element);
    if (intAttribute(attributes, "protocol") !== 6) continue;
    const destinationPort = optionalAttribute(attributes, "destinationPort");
    if (destinationPort) ports.add(destinationPort);
    if (
      optionalAttribute(attributes, "sourceAddress").startsWith("10.1.") &&
      optionalAttribute(attributes, "destinationAddress").startsWith("10.2.")
    ) {
      forwardIds.add(intAttribute(attributes, "flowId"));
    }
  }

  const goodputs = [];
  const delays = [];
  const jitters = [];
  const firstTxTimes = [];
  const lastRxTimes = [];
  let totalRxBytes = 0;
  let txPackets = 0;
  let lostPackets = 0;

  const stats = child(monitor, "FlowStats");
  for (const element of asArray(child(stats, "Flow"))) {
    const attributes = asAttributes(element);
    if (!forwardIds.has(intAttribute(attributes, "flowId"))) continue;
    const rxPackets = intAttribute(attributes, "rxPackets");
    const rxBytes = intAttribute(attributes, "rxBytes");
    const firstTx = parseNsTime(stringAttribute(attributes, "timeFirstTxPacket"));
    const lastRx = parseNsTime(stringAttribute(attributes, "timeLastRxPacket"));
    const durationS = (lastRx - firstTx) / 1e9;
    goodputs.push(durationS > 0 ? (rxBytes * 8) / durationS / 1e6 : 0);
    firstTxTimes.push(firstTx);
    lastRxTimes.push(lastRx);
    totalRxBytes += rxBytes;
    if (rxPackets > 0) {
      delays.push(parseNsTime(stringAttribute(attributes, "delaySum")) / rxPackets / 1e6);
    }
    if (rxPackets > 1) {
      jitters.push(
        parseNsTime(stringAttribute(attributes, "jitterSum")) /
          (rxPackets - 1) /
          1e6,
      );
    }
    txPackets += intAttribute(attributes, "txPackets");
    lostPackets += intAttribute(attributes, "lostPackets");
  }

  const nflow = goodputs.length;
  const individualTotal = sum(goodputs);
  const squares = sum(goodputs.map((value) => value * value));
  const jain =
    nflow > 0 && squares > 0
      ? (individualTotal * individualTotal) / (nflow * squares)
      : 0;
  const sharedDurationNs =
    nflow > 0 ? Math.max(...lastRxTimes) - Math.min(...firstTxTimes) : 0;
  const aggregateGoodput =
    sharedDurationNs > 0
      ? (totalRxBytes * 8) / (sharedDurationNs / 1e9) / 1e6
      : 0;

  return {
    nflow,
    ports: [...ports].sort().join("/"),
    goodput: aggregateGoodput,
    delay: delays.length > 0 ? mean(delays) : 0,
    jitter: jitters.length > 0 ? mean(jitters) : 0,
    loss: txPackets > 0 ? (100 * lostPackets) / txPackets : 0,
    jain,
  };
}

export function listFlowMonitorFiles(directory, options = {}) {
  const recursive = options.recursive === true;
  const found = [];
  const walk = (current) => {
    let entries;
    try {
      entries = readdirSync(current, { withFileTypes: true });
    } catch {
      return;
    }
    for (const entry of entries) {
      const full = path.join(current, entry.name);
      if (entry.isDirectory()) {
        if (recursive) walk(full);
        continue;
      }
      if (entry.isFile() && entry.name.endsWith(".flowmonitor")) found.push(full);
    }
  };
  walk(directory);
  found.sort();
  return found;
}

export function loadAllResults(logsDir) {
  const results = [];
  for (const filepath of listFlowMonitorFiles(logsDir, { recursive: true })) {
    const basename = path.basename(filepath, ".flowmonitor");
    const parsed = parseRunName(basename);
    if (!parsed) continue;
    results.push(
      new ScenarioResult({
        scenario: parsed.scenario,
        protocol: parsed.protocol,
        seed: parsed.seed,
        sourcePath: filepath,
        flows: parseFlowMonitor(filepath),
      }),
    );
  }
  return results;
}

export function aggregateResults(results) {
  const grouped = new Map();
  for (const result of results) {
    const key = `${result.scenario}\u0000${result.protocol}`;
    const bucket = grouped.get(key);
    if (bucket) bucket.push(result);
    else grouped.set(key, [result]);
  }

  const aggregated = [];
  for (const key of [...grouped.keys()].sort()) {
    const runs = grouped.get(key) ?? [];
    const [scenario, protocol] = key.split("\u0000");
    aggregated.push(
      new AggregatedResult({
        scenario,
        protocol,
        totalThroughputMbps: mean(
          runs.map((run) => run.totalThroughputMbps),
        ),
        avgDelayMs: mean(runs.map((run) => run.avgDelayMs)),
        avgJitterMs: mean(runs.map((run) => run.avgJitterMs)),
        totalLossRate: mean(runs.map((run) => run.totalLossRate)),
        seedCount: runs.length,
      }),
    );
  }
  return aggregated;
}

export function isFile(filepath) {
  try {
    return statSync(filepath).isFile();
  } catch {
    return false;
  }
}

export function isDirectory(filepath) {
  try {
    return statSync(filepath).isDirectory();
  } catch {
    return false;
  }
}
