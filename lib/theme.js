// @ts-check



export const POINTS_PER_INCH = 72;

export const FONT_FAMILY =
  "Helvetica, Arial, 'Helvetica Neue', 'DejaVu Sans', 'PingFang SC', 'Songti SC', 'Hiragino Sans GB', 'Microsoft YaHei', sans-serif";

export const GRID_COLOR = "#d9d9d9";

export const AXIS_COLOR = "#333333";

export const MISSING_LABEL_COLOR = "#555555";

export function inches(inches) {
  return inches * POINTS_PER_INCH;
}

export function baseLayout(options) {
  const {
    width,
    height,
    baseFontSize = 9,
    margin,
  } = options;

  return {
    width,
    height,
    font: { family: FONT_FAMILY, size: baseFontSize, color: "#111111" },
    paper_bgcolor: "#ffffff",
    plot_bgcolor: "#ffffff",
    showlegend: true,
    ...(margin
      ? {
          margin: {
            t: margin.top ?? 0,
            r: margin.right ?? 0,
            b: margin.bottom ?? 0,
            l: margin.left ?? 0,
            autoexpand: false,
          },
        }
      : {}),
  };
}

export function axisStyle(options = {}) {
  const { title, titleSize = 9, tickSize = 8, grid = true, log = false } = options;
  return {
    type: log ? "log" : "linear",
    showgrid: grid,
    gridcolor: GRID_COLOR,
    gridwidth: 0.6,
    zeroline: false,
    showline: false,
    ticks: "outside",
    ticklen: 4,
    tickcolor: AXIS_COLOR,
    tickfont: { size: tickSize },
    title: title
      ? { text: title, font: { size: titleSize }, standoff: 8 }
      : undefined,
    automargin: true,
  };
}

export function tickLabel(value, digits = 4) {
  if (value === 0) return "0";
  const magnitude = Math.abs(value);
  if (magnitude >= 1e5 || magnitude < 1e-3) {
    return value.toExponential(0).replace("e+", "e");
  }
  const decimals = magnitude >= 100 ? 0 : magnitude >= 1 ? 1 : digits;
  const text = value.toFixed(decimals);
  return text.includes(".") ? text.replace(/0+$/, "").replace(/\.$/, "") : text;
}
