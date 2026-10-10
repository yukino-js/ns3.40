#!/usr/bin/env node
// @ts-check


import { mkdir, readdir, readFile, rm, writeFile } from "node:fs/promises";
import path from "node:path";
import process from "node:process";
import { pathToFileURL } from "node:url";
import { buildCsv, parseCsv } from "../../lib/csv.js";
import { forwardKpi, listFlowMonitorFiles } from "../../lib/flowmonitor.js";
import { FigureRenderer } from "../../lib/plotly.js";
import {
  DOCS_PLOT_COLORS,
  PROTOCOL_LABEL,
  PROTOCOL_ORDER,
  SCENARIOS,
  SCENARIO_BY_SID,
  S_ORDER,
  UDP_PAIRED_SIDS,
} from "../../lib/scenarios.js";
import { floatToString, mean, round } from "../../lib/stats.js";
import { GRID_COLOR, axisStyle, baseLayout, inches } from "../../lib/theme.js";


const REPO_ROOT = path.resolve(import.meta.dirname, "..", "..");

const LOGS_DIR = path.join(REPO_ROOT, "logs");

const PLOTS_DIR = path.join(REPO_ROOT, "docs", "plots");

const KPI_CSV = path.join(LOGS_DIR, "summary", "kpi_forward.csv");

const EXPECTED_ARTIFACTS = 864;

const MANIFEST_NAME = "figure_manifest.json";

const FIGURE_STEMS = new Set([
  "fig01_goodput_clean",
  "fig02_delay_clean",
  "fig03_tradeoff_clean",
  "fig04_udp_burst_clean",
  "fig06_architecture_zh",
  "fig07_workflow_zh",
]);

const INTEGER_COLUMNS = new Set(["Flows"]);




export function rateMbps(text) {
  const match = /^([\d.]+)([GMK]?)bps/.exec(text);
  if (!match) throw new Error(`unparsable rate: ${text}`);
  const scale = { G: 1000, M: 1, K: 1e-3, "": 1e-6 }[match[2]];
  return Number.parseFloat(match[1]) * (scale ?? 1);
}

export function delayMs(text) {
  const match = /^([\d.]+)(ns|us|ms|s)/.exec(text);
  if (!match) throw new Error(`unparsable delay: ${text}`);
  const scale = { ns: 1e-6, us: 1e-3, ms: 1, s: 1e3 }[match[2]];
  return Number.parseFloat(match[1]) * (scale ?? 1);
}


const SCENARIO_LINKS = new Map(
  SCENARIOS.map(([name, access, bottleneck, accessDelay, bottleneckDelay]) => [
    name,
    { access, bottleneck, accessDelay, bottleneckDelay },
  ]),
);

export async function deriveRows() {
  const settings = [
    ["tcp_only", path.join(LOGS_DIR, "comparison")],
    ["udp_burst", path.join(LOGS_DIR, "comparison-udp")],
  ];

  const rows = [];
  for (const [setting, directory] of settings) {
    for (const filepath of listFlowMonitorFiles(directory)) {
      const base = path.basename(filepath, ".flowmonitor");
      const match = /^(.+)_(Tcp[A-Za-z0-9]+?)(?:_s(\d+))?$/.exec(base);
      if (!match) {
        throw new Error(`unrecognised artifact name: ${base}`);
      }
      const scenario = match[1];
      const protocol = match[2];
      const seed = match[3] ? Number.parseInt(match[3], 10) : "";
      const link = SCENARIO_LINKS.get(scenario);
      if (!link) continue;

      const kpi = forwardKpi(filepath);
      const bottleneckMbps = rateMbps(link.bottleneck);
      rows.push({
        Setting: setting,
        Scenario: scenario,
        Protocol: protocol,
        Seed: seed,
        BottleneckMbps: bottleneckMbps,
        BaseOwdMs: round(
          2 * delayMs(link.accessDelay) + delayMs(link.bottleneckDelay),
          4,
        ),
        Flows: kpi.nflow,
        SinkPort: kpi.ports,
        Goodput_Mbps: round(kpi.goodput, 2),
        Util: round(kpi.goodput / bottleneckMbps, 4),
        Delay_ms: round(kpi.delay, 4),
        Jitter_ms: round(kpi.jitter, 4),
        Loss_pct: round(kpi.loss, 4),
        Jain: round(kpi.jain, 4),
        Source: path.relative(LOGS_DIR, filepath).split(path.sep).join("/"),
      });
    }
  }

  if (rows.length === 0) {
    throw new Error(
      `No FlowMonitor files found under ${LOGS_DIR}; ` +
        "run the simulations first",
    );
  }
  return rows;
}

export async function writeAndVerifyCsv(rows) {
  const first = rows[0];
  if (!first) throw new Error("cannot write an empty KPI table");
  const fieldnames = Object.keys(first);

  let previous = null;
  try {
    previous = parseCsv(await readFile(KPI_CSV, "utf8"));
  } catch {
    previous = null;
  }

  const current = rows.map((row) => {
    const converted = {};
    for (const key of fieldnames) {
      const value = row[key];
      converted[key] =
        typeof value === "string" || INTEGER_COLUMNS.has(key)
          ? String(value)
          : floatToString(value);
    }
    return converted;
  });

  await mkdir(path.dirname(KPI_CSV), { recursive: true });
  await writeFile(
    KPI_CSV,
    buildCsv(fieldnames, current, { lineterminator: "\n" }),
    "utf8",
  );

  if (previous === null) return "kpi_forward.csv created";

  if (previous.length !== current.length) {
    return `kpi_forward.csv ROW COUNT CHANGED: ${previous.length} -> ${current.length}`;
  }

  let changed = 0;
  for (const [index, before] of previous.entries()) {
    const after = current[index];
    const differs = fieldnames.some((key) => before[key] !== after[key]);
    if (differs) changed += 1;
  }
  return changed === 0
    ? "kpi_forward.csv regenerated: identical to previous version"
    : `kpi_forward.csv regenerated: ${changed} rows CHANGED vs previous version`;
}


export function buildPlotView(rows) {
  const groups = new Map();
  for (const row of rows) {
    const setting = String(row.Setting);
    const scenario = String(row.Scenario);
    const protocol = String(row.Protocol);
    let perScenario = groups.get(setting);
    if (!perScenario) {
      perScenario = new Map();
      groups.set(setting, perScenario);
    }
    let perProtocol = perScenario.get(scenario);
    if (!perProtocol) {
      perProtocol = {};
      perScenario.set(scenario, perProtocol);
    }
    const bucket = perProtocol[protocol];
    if (bucket) bucket.push(row);
    else perProtocol[protocol] = [row];
  }

  const selected = {
    tcp_only: S_ORDER.map(([, scenario]) => scenario),
    udp_burst: S_ORDER.filter(([sid]) => UDP_PAIRED_SIDS.includes(sid)).map(
      ([, scenario]) => scenario,
    ),
  };

  const view = { tcp_only: {}, udp_burst: {} };
  for (const [setting, scenarios] of Object.entries(selected)) {
    for (const scenario of scenarios) {
      const protocols = groups.get(setting)?.get(scenario) ?? {};
      const found = Object.keys(protocols).sort();
      if (
        found.length !== PROTOCOL_ORDER.length ||
        !PROTOCOL_ORDER.every((protocol) => protocol in protocols)
      ) {
        throw new Error(
          `Incomplete group ${setting}/${scenario}: ` +
            `found [${found.join(", ")}], expected [${PROTOCOL_ORDER.join(", ")}]`,
        );
      }
      const averaged = {};
      for (const protocol of PROTOCOL_ORDER) {
        averaged[protocol] = averageRows(protocols[protocol]);
      }
      view[setting][scenario] = averaged;
    }
  }
  return view;
}

const AVERAGED_FIELDS = [
  "BottleneckMbps",
  "BaseOwdMs",
  "Flows",
  "Goodput_Mbps",
  "Util",
  "Delay_ms",
  "Jitter_ms",
  "Loss_pct",
  "Jain",
];

function averageRows(rows) {
  const base = rows[0];
  if (rows.length === 1) return base;
  const averaged = { ...base };
  for (const key of AVERAGED_FIELDS) {
    if (typeof base[key] !== "number") continue;
    averaged[key] = round(mean(rows.map((row) => Number(row[key]))), 4);
  }
  return averaged;
}


function protocolColor(protocol) {
  return DOCS_PLOT_COLORS[protocol] ?? "#333333";
}

function protocolLabel(protocol) {
  return PROTOCOL_LABEL[ (protocol)] ?? protocol;
}

function titleCase(text) {
  return text
    .replace(/_/g, " ")
    .replace(/\b\w/g, (character) => character.toUpperCase());
}

function groupedBarTraces(view, positions, sids, valueKey, width = 0.19) {
  const categories = sids.map((sid) => SCENARIO_BY_SID.get(sid) ?? sid);
  const traces = [];

  PROTOCOL_ORDER.forEach((protocol, index) => {
    const xs = [];
    const ys = [];
    categories.forEach((scenario, position) => {
      const row = view[scenario]?.[protocol];
      if (!row) return;
      xs.push(positions[position] + (index - 1.5) * width);
      ys.push(Number(row[valueKey]));
    });
    traces.push({
      type: "bar",
      x: xs,
      y: ys,
      width,
      name: protocolLabel(protocol),
      marker: { color: protocolColor(protocol), line: { color: "white", width: 0.5 } },
      legendgroup: protocol,
    });
  });

  return { traces, categories };
}

function sidAxis(sids, positions, tickSize = 8) {
  return {
    ...axisStyle({ grid: false, tickSize }),
    tickmode: "array",
    tickvals: positions,
    ticktext: sids,
    range: [-0.7, sids.length - 0.3],
  };
}

function topLegend() {
  return {
    orientation: "h",
    x: 0.5,
    xanchor: "center",
    y: 1.02,
    yanchor: "bottom",
    font: { size: 8 },
    tracegroupgap: 4,
  };
}

export async function saveFigure(renderer, spec) {
  const rendered = await renderer.render(
    spec,
    { png: true, pdf: true, svg: true },
    300 / 72,
  );

  const files = [];
  for (const extension of ["png", "pdf", "svg"]) {
    const filename = `${spec.name}.${extension}`;
    const target = path.join(PLOTS_DIR, filename);
    const payload =
      extension === "png"
        ? rendered.png
        : extension === "pdf"
          ? rendered.pdf
          : rendered.svg;
    if (payload === undefined) continue;
    await writeFile(target, payload);
    files.push(filename);
  }
  return { stem: spec.name, files };
}


export async function plotGoodput(view, renderer) {
  const sids = S_ORDER.map(([sid]) => sid);
  const positions = sids.map((_, index) => index);
  const width = inches(9.6);
  const height = inches(3.6);
  const { traces } = groupedBarTraces(
    view.tcp_only,
    positions,
    sids,
    "Goodput_Mbps",
  );

  const layout = {
    ...baseLayout({
      width,
      height,
      baseFontSize: 8,
      margin: { top: 70, right: 25, bottom: 45, left: 80 },
    }),
    title: {
      text: "Forward goodput across 19 selected scenarios (TCP-only)",
      font: { size: 10 },
      x: 0.5,
      xanchor: "center",
    },
    bargap: 0.25,
    bargroupgap: 0.02,
    xaxis: sidAxis(sids, positions),
    yaxis: axisStyle({ title: "Aggregate forward goodput (Mbps)", log: true }),
    legend: topLegend(),
  };

  return saveFigure(renderer, {
    name: "fig01_goodput_clean",
    width,
    height,
    data: traces,
    layout,
  });
}

export async function plotDelay(view, renderer) {
  const sids = S_ORDER.map(([sid]) => sid);
  const positions = sids.map((_, index) => index);
  const width = inches(9.6);
  const height = inches(3.6);
  const { traces, categories } = groupedBarTraces(
    view.tcp_only,
    positions,
    sids,
    "Delay_ms",
  );

  const shapes = [];
  categories.forEach((scenario, position) => {
    const protocols = view.tcp_only[scenario];
    const first = protocols ? Object.values(protocols)[0] : undefined;
    if (!first) return;
    const baseOwd = Number(first.BaseOwdMs);
    shapes.push({
      type: "line",
      x0: position - 0.42,
      x1: position + 0.42,
      y0: Math.max(baseOwd, 1e-3),
      y1: Math.max(baseOwd, 1e-3),
      line: { color: "#222222", width: 1, dash: "dash" },
      layer: "above",
    });
  });

  const lineHandle = {
    type: "scatter",
    mode: "lines",
    x: [],
    y: [],
    name: "Base OWD",
    line: { color: "#222222", width: 1, dash: "dash" },
    showlegend: true,
  };

  const layout = {
    ...baseLayout({
      width,
      height,
      baseFontSize: 8,
      margin: { top: 70, right: 25, bottom: 45, left: 80 },
    }),
    title: {
      text: "Forward one-way delay; dashes mark the base propagation OWD",
      font: { size: 10 },
      x: 0.5,
      xanchor: "center",
    },
    bargap: 0.25,
    bargroupgap: 0.02,
    xaxis: sidAxis(sids, positions),
    yaxis: axisStyle({ title: "Mean one-way delay (ms)", log: true }),
    legend: topLegend(),
    shapes,
  };

  return saveFigure(renderer, {
    name: "fig02_delay_clean",
    width,
    height,
    data: [...traces, lineHandle],
    layout,
  });
}

export async function plotTradeoff(view, renderer) {
  const width = inches(6.4);
  const height = inches(4.2);

  const data = [];
  for (const protocol of PROTOCOL_ORDER) {
    const xs = [];
    const ys = [];
    for (const scenario of Object.keys(view.tcp_only).sort()) {
      const row = view.tcp_only[scenario]?.[protocol];
      if (!row) continue;
      xs.push(Math.max(Number(row.Delay_ms), 1e-2));
      ys.push(Number(row.Util) * 100);
    }
    data.push({
      type: "scatter",
      mode: "markers",
      x: xs,
      y: ys,
      name: protocolLabel(protocol),
      marker: {
        color: protocolColor(protocol),
        size: 9,
        opacity: 0.8,
        line: { color: "white", width: 0.6 },
      },
    });
  }

  const labelX = [];
  const labelY = [];
  const labelText = [];
  for (const sid of ["S8", "S9", "S19"]) {
    const scenario = SCENARIO_BY_SID.get(sid);
    const row = scenario ? view.tcp_only[scenario]?.TcpSwift : undefined;
    if (!row) continue;
    labelX.push(Math.max(Number(row.Delay_ms), 1e-2));
    labelY.push(Number(row.Util) * 100);
    labelText.push(sid);
  }
  if (labelText.length > 0) {
    data.push({
      type: "scatter",
      mode: "text",
      x: labelX,
      y: labelY,
      text: labelText,
      textposition: "top right",
      textfont: { size: 8, color: protocolColor("TcpSwift") },
      showlegend: false,
      hoverinfo: "skip",
    });
  }

  const layout = {
    ...baseLayout({
      width,
      height,
      baseFontSize: 8,
      margin: { top: 55, right: 25, bottom: 55, left: 70 },
    }),
    title: {
      text: "Utilization-delay trade-off (TCP-only, 19 scenarios)",
      font: { size: 10 },
      x: 0.5,
      xanchor: "center",
    },
    xaxis: axisStyle({ title: "Mean one-way delay (ms, log scale)", log: true }),
    yaxis: { ...axisStyle({ title: "Bottleneck utilization (%)" }), range: [0, 108] },
    legend: { x: 1, y: 0, xanchor: "right", yanchor: "bottom", font: { size: 8 } },
  };

  return saveFigure(renderer, {
    name: "fig03_tradeoff_clean",
    width,
    height,
    data,
    layout,
  });
}

export async function plotUdpBurst(view, renderer) {
  const sids = UDP_PAIRED_SIDS;
  const positions = sids.map((_, index) => index);
  const width = inches(9.6);
  const height = inches(5.6);
  const barWidth = 0.19;

  const data = [];
  PROTOCOL_ORDER.forEach((protocol, index) => {
    const dropX = [];
    const drops = [];
    const lossX = [];
    const losses = [];

    sids.forEach((sid, position) => {
      const scenario = SCENARIO_BY_SID.get(sid);
      if (!scenario) return;
      const tcpRow = view.tcp_only[scenario]?.[protocol];
      const udpRow = view.udp_burst[scenario]?.[protocol];
      if (!tcpRow || !udpRow) return;
      const tcpGoodput = Number(tcpRow.Goodput_Mbps);
      const offset = positions[position] + (index - 1.5) * barWidth;
      dropX.push(offset);
      drops.push((100 * (Number(udpRow.Goodput_Mbps) - tcpGoodput)) / tcpGoodput);
      lossX.push(offset);
      losses.push(
        Math.max(Number(udpRow.Loss_pct) - Number(tcpRow.Loss_pct), 0),
      );
    });

    data.push({
      type: "bar",
      x: dropX,
      y: drops,
      width: barWidth,
      name: protocolLabel(protocol),
      marker: { color: protocolColor(protocol), line: { color: "white", width: 0.5 } },
      xaxis: "x",
      yaxis: "y",
      legendgroup: protocol,
    });
    data.push({
      type: "bar",
      x: lossX,
      y: losses,
      width: barWidth,
      name: protocolLabel(protocol),
      marker: { color: protocolColor(protocol), line: { color: "white", width: 0.5 } },
      xaxis: "x2",
      yaxis: "y2",
      legendgroup: protocol,
      showlegend: false,
    });
  });

  const annotations = [
    {
      text: "Cross-traffic robustness on the 15 paired scenarios",
      x: 0.5,
      y: 1,
      xref: "x domain",
      yref: "y domain",
      xanchor: "center",
      yanchor: "bottom",
      showarrow: false,
      font: { size: 10 },
    },
    {
      text: "Goodput change under burst (%)",
      x: -0.07,
      y: 0.5,
      xref: "x domain",
      yref: "y domain",
      xanchor: "center",
      yanchor: "middle",
      textangle: -90,
      showarrow: false,
      font: { size: 8 },
    },
    {
      text: "Added loss under burst (pp)",
      x: -0.07,
      y: 0.5,
      xref: "x2 domain",
      yref: "y2 domain",
      xanchor: "center",
      yanchor: "middle",
      textangle: -90,
      showarrow: false,
      font: { size: 8 },
    },
  ];

  const layout = {
    ...baseLayout({
      width,
      height,
      baseFontSize: 8,
      margin: { top: 100, right: 25, bottom: 45, left: 85 },
    }),
    bargap: 0.25,
    bargroupgap: 0.02,
    grid: { rows: 2, columns: 1, pattern: "independent", ygap: 0.34 },
    xaxis: { ...sidAxis(sids, positions), anchor: "y" },
    yaxis: { ...axisStyle({}), anchor: "x", zeroline: true, zerolinecolor: "#444444" },
    xaxis2: { ...sidAxis(sids, positions), anchor: "y2" },
    yaxis2: { ...axisStyle({}), anchor: "x2", rangemode: "tozero" },
    legend: topLegend(),
    annotations,
  };

  return saveFigure(renderer, {
    name: "fig04_udp_burst_clean",
    width,
    height,
    data,
    layout,
  });
}


const BEZIER_CIRCLE_K = 0.5522847498307936;

export function roundedRectPath(x, y, w, h, r) {
  const radius = Math.max(0, Math.min(r, w / 2, h / 2));
  const c = radius * BEZIER_CIRCLE_K;
  return [
    `M ${x + radius} ${y}`,
    `L ${x + w - radius} ${y}`,
    `C ${x + w - radius + c} ${y} ${x + w} ${y + radius - c} ${x + w} ${y + radius}`,
    `L ${x + w} ${y + h - radius}`,
    `C ${x + w} ${y + h - radius + c} ${x + w - radius + c} ${y + h} ${x + w - radius} ${y + h}`,
    `L ${x + radius} ${y + h}`,
    `C ${x + radius - c} ${y + h} ${x} ${y + h - radius + c} ${x} ${y + h - radius}`,
    `L ${x} ${y + radius}`,
    `C ${x} ${y + radius - c} ${x + radius - c} ${y} ${x + radius} ${y}`,
    "Z",
  ].join(" ");
}

export function diagramCanvas(options) {
  const { width, height, margin } = options;
  const plotWidth = width - margin.left - margin.right;
  const plotHeight = height - margin.top - margin.bottom;
  return {
    plotWidth,
    plotHeight,
    x: (fraction) => fraction * plotWidth,
    y: (fraction) => fraction * plotHeight,
  };
}

export function diagramLayout(options) {
  const { width, height, margin, canvas, components, title } = options;

  const layout = {
    ...baseLayout({ width, height, baseFontSize: 8, margin }),
    xaxis: {
      range: [0, canvas.plotWidth],
      visible: false,
      fixedrange: true,
      showgrid: false,
      zeroline: false,
    },
    yaxis: {
      range: [0, canvas.plotHeight],
      visible: false,
      fixedrange: true,
      showgrid: false,
      zeroline: false,
    },
    shapes: components.shapes,
    annotations: components.annotations,
    ...(title
      ? {
          title: {
            text: title.text,
            font: { size: title.size ?? 12 },
            x: 0.5,
            xanchor: "center",
          },
        }
      : {}),
  };

  return layout;
}

export class ShapeState {
  shapes = [];
  annotations = [];
  traces = [];

  box(box, text, facecolor, fontSize = 8.5, radius = 8) {
    this.shapes.push({
      type: "path",
      path: roundedRectPath(box.x, box.y, box.width, box.height, radius),
      line: { color: "#333333", width: 1 },
      fillcolor: facecolor,
      layer: "below",
    });
    this.annotations.push({
      x: box.x + box.width / 2,
      y: box.y + box.height / 2,
      text: text.replace(/\n/g, "<br>"),
      showarrow: false,
      xanchor: "center",
      yanchor: "middle",
      font: { size: fontSize },
    });
  }

  arrow(start, end, text, color = "#333333", labelOffset = 7) {
    this.annotations.push({
      x: end.x,
      y: end.y,
      ax: start.x,
      ay: start.y,
      xref: "x",
      yref: "y",
      axref: "x",
      ayref: "y",
      text: "",
      showarrow: true,
      arrowhead: 2,
      arrowwidth: 1.1,
      arrowcolor: color,
    });
    if (text) {
      this.annotations.push({
        x: (start.x + end.x) / 2,
        y: (start.y + end.y) / 2 + labelOffset,
        text,
        showarrow: false,
        xanchor: "center",
        yanchor: "middle",
        font: { size: 7, color },
      });
    }
  }

  polyline(points, color) {
    this.traces.push({
      type: "scatter",
      mode: "lines",
      x: points.map(([x]) => x),
      y: points.map(([, y]) => y),
      line: { color, width: 1.2 },
      hoverinfo: "skip",
      showlegend: false,
    });
  }

  label(position, text, options = {}) {
    this.annotations.push({
      x: position.x,
      y: position.y,
      text,
      showarrow: false,
      xanchor: "center",
      yanchor: "middle",
      textangle: options.rotate ?? 0,
      font: { size: options.fontSize ?? 7.5, color: options.color ?? "#333333" },
    });
  }
}

export async function plotArchitecture(renderer) {
  const width = inches(9.2);
  const height = inches(4.6);
  const margin = { top: 40, right: 10, bottom: 10, left: 10 };
  const shapes = new ShapeState();
  const canvas = diagramCanvas({ width, height, margin });
  const box = (
 x,
 y,
 w,
 h,
 text,
  ) =>
    shapes.box(
      {
        x: canvas.x(x),
        y: canvas.y(y),
        width: canvas.x(w),
        height: canvas.y(h),
      },
      text,
      "rgba(0,0,0,0)",
    );
  const arrow = (
 start,
 end,
 text,
 color,
  ) =>
    shapes.arrow(
      { x: canvas.x(start[0]), y: canvas.y(start[1]) },
      { x: canvas.x(end[0]), y: canvas.y(end[1]) },
      text,
      color,
    );

  box(0.03, 0.62, 0.20, 0.18, "ns-3 原生回调\nACK · 丢包 · RTT · ECN");
  box(0.29, 0.62, 0.20, 0.18, "连接状态\ncwnd · minRTT · BDP · α");
  box(0.56, 0.62, 0.18, 0.18, "C++ 决策\n同一事件进程");
  box(0.81, 0.62, 0.16, 0.18, "协议栈写回\ncwnd · ssthresh");
  box(0.08, 0.18, 0.26, 0.16, "拥塞分类\n超时 · ECN · 丢包");
  box(0.40, 0.18, 0.22, 0.16, "两级 BDP 估计\n交付速率 · 最大值");
  box(0.70, 0.18, 0.26, 0.16, "α 自适应\nRTT 反馈 · 目标窗口");

  arrow([0.23, 0.71], [0.29, 0.71], "读取", undefined);
  arrow([0.49, 0.71], [0.56, 0.71], "决策", undefined);
  arrow([0.74, 0.71], [0.81, 0.71], "更新", undefined);
  arrow([0.13, 0.62], [0.18, 0.34], "拥塞信号", undefined);
  arrow([0.34, 0.31], [0.56, 0.65], "拥塞类型", undefined);
  arrow([0.62, 0.26], [0.70, 0.26], "BDP", undefined);
  arrow([0.83, 0.34], [0.68, 0.62], "目标窗口", undefined);

  shapes.shapes.push({
    type: "path",
    path:
      `M ${canvas.x(0.89)} ${canvas.y(0.80)} ` +
      `Q ${canvas.x(0.5)} ${canvas.y(0.93)} ${canvas.x(0.13)} ${canvas.y(0.80)}`,
    line: { color: "#333333", width: 1.2, dash: "dot" },
    fillcolor: "rgba(0,0,0,0)",
    layer: "above",
  });
  shapes.label(
    { x: canvas.x(0.5), y: canvas.y(0.945) },
    "原生回调写回",
    { fontSize: 7.5 },
  );

  const layout = diagramLayout({
    width,
    height,
    margin,
    canvas,
    components: shapes,
    title: { text: "拥塞控制系统架构" },
  });

  return saveFigure(renderer, {
    name: "fig06_architecture_zh",
    width,
    height,
    data: shapes.traces,
    layout,
  });
}

export async function plotWorkflow(renderer) {
  const width = inches(9.4);
  const height = inches(7.0);
  const margin = { top: 40, right: 10, bottom: 10, left: 10 };
  const shapes = new ShapeState();
  const canvas = diagramCanvas({ width, height, margin });

  const box = (
 x,
 y,
 w,
 h,
 text,
  ) =>
    shapes.box(
      {
        x: canvas.x(x),
        y: canvas.y(y),
        width: canvas.x(w),
        height: canvas.y(h),
      },
      text,
      "rgba(0,0,0,0)",
    );
  const arrow = (
 start,
 end,
 text,
 color,
  ) =>
    shapes.arrow(
      { x: canvas.x(start[0]), y: canvas.y(start[1]) },
      { x: canvas.x(end[0]), y: canvas.y(end[1]) },
      text,
      color,
    );

  box(0.40, 0.93, 0.20, 0.045, "连接建立");
  box(0.31, 0.80, 0.38, 0.08, "S1 状态采集\n窗口 · 传输 · RTT · 事件");
  box(0.34, 0.67, 0.32, 0.075, "S2 拥塞判定");
  box(0.06, 0.42, 0.36, 0.11, "S5b 差异化缩减\n0.50 / 0.75 / 0.70 · 安全保护");
  box(0.58, 0.53, 0.36, 0.09, "S3 两级 BDP 估计\n交付速率 · 最大值滤波");
  box(0.58, 0.38, 0.36, 0.09, "S4 α 自适应\nRTT · 快慢 EMA · 连续增长");
  box(0.58, 0.23, 0.36, 0.09, "S5a 目标窗口\n有界跟踪 α × BDP");
  box(0.31, 0.06, 0.38, 0.08, "S6 原生回调写回\ncwnd · ssthresh");

  arrow([0.50, 0.93], [0.50, 0.88], undefined, undefined);
  arrow([0.50, 0.80], [0.50, 0.745], undefined, undefined);
  arrow([0.40, 0.67], [0.24, 0.53], "拥塞", undefined);
  arrow([0.60, 0.67], [0.76, 0.62], "非拥塞", undefined);
  arrow([0.76, 0.53], [0.76, 0.47], undefined, undefined);
  arrow([0.76, 0.38], [0.76, 0.32], undefined, undefined);
  arrow([0.68, 0.23], [0.60, 0.14], undefined, undefined);
  arrow([0.24, 0.42], [0.40, 0.14], undefined, undefined);

  const feedbackColor = "#333333";
  shapes.polyline(
    [
      [canvas.x(0.31), canvas.y(0.10)],
      [canvas.x(0.04), canvas.y(0.10)],
      [canvas.x(0.04), canvas.y(0.84)],
    ],
    feedbackColor,
  );
  arrow([0.04, 0.84], [0.31, 0.84], undefined, feedbackColor);
  shapes.label(
    { x: canvas.x(0.028), y: canvas.y(0.48) },
    "反馈 · EMA 更新",
    { fontSize: 7.5, color: feedbackColor, rotate: -90 },
  );

  const layout = diagramLayout({
    width,
    height,
    margin,
    canvas,
    components: shapes,
    title: { text: "拥塞控制方法整体流程", size: 12 },
  });

  return saveFigure(renderer, {
    name: "fig07_workflow_zh",
    width,
    height,
    data: shapes.traces,
    layout,
  });
}

export async function cleanStaleOutputs() {
  const removed = [];
  let entries;
  try {
    entries = new Set(await readdir(PLOTS_DIR));
  } catch {
    return removed;
  }
  for (const stem of FIGURE_STEMS) {
    for (const extension of ["png", "pdf", "svg"]) {
      const entry = `${stem}.${extension}`;
      if (!entries.has(entry)) continue;
      await rm(path.join(PLOTS_DIR, entry), { force: true });
      removed.push(entry);
    }
  }
  return removed.sort();
}

export async function main() {
  await mkdir(PLOTS_DIR, { recursive: true });

  const rows = await deriveRows();
  if (rows.length !== EXPECTED_ARTIFACTS) {
    throw new Error(
      `Expected ${EXPECTED_ARTIFACTS} artifacts, found ${rows.length}`,
    );
  }

  const csvStatus = await writeAndVerifyCsv(rows);
  const view = buildPlotView(rows);
  const removed = await cleanStaleOutputs();

  const plots = [];
  const renderer = await FigureRenderer.open();
  try {
    plots.push(await plotGoodput(view, renderer));
    plots.push(await plotDelay(view, renderer));
    plots.push(await plotTradeoff(view, renderer));
    plots.push(await plotUdpBurst(view, renderer));
    plots.push(await plotArchitecture(renderer));
    plots.push(await plotWorkflow(renderer));
  } finally {
    await renderer.close();
  }

  const manifest = {
    source: "logs/{comparison,comparison-udp}/*.flowmonitor (864 archived artifacts)",
    kpi_csv: path.relative(REPO_ROOT, KPI_CSV).split(path.sep).join("/"),
    kpi_csv_status: csvStatus,
    metric_definition:
      "forward TCP data flows only (proto 6, 10.1.x -> 10.2.x); aggregate goodput uses the shared first-TX to last-RX span",
    selected_groups: {
      tcp_only: S_ORDER.map(([sid, scenario]) => `${sid}=${scenario}`),
      udp_burst: UDP_PAIRED_SIDS,
    },
    stale_outputs_removed: removed,
    figures: plots,
  };
  await writeFile(
    path.join(PLOTS_DIR, MANIFEST_NAME),
    JSON.stringify(manifest, null, 2),
    "utf8",
  );

  console.log(
    JSON.stringify(
      {
        kpi_csv_status: csvStatus,
        selected_tcp_groups: Object.keys(view.tcp_only).length,
        selected_udp_groups: Object.keys(view.udp_burst).length,
        figures: plots.map((plot) => plot.stem),
      },
      null,
      2,
    ),
  );
}

const invokedDirectly =
  process.argv[1] !== undefined &&
  import.meta.url === pathToFileURL(path.resolve(process.argv[1])).href;

if (invokedDirectly) {
  main().then(
    () => {
      process.exitCode = 0;
    },
    (error) => {
      console.error(error);
      process.exitCode = 1;
    },
  );
}
