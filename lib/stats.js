// @ts-check


export function mean(values) {
  if (values.length === 0) return Number.NaN;
  let total = 0;
  for (const value of values) total += value;
  return total / values.length;
}

export function sum(values) {
  let total = 0;
  for (const value of values) total += value;
  return total;
}

export function round(value, digits) {
  if (!Number.isFinite(value)) return value;
  const factor = 10 ** digits;
  const scaled = value * factor;
  if (!Number.isFinite(scaled)) return value;
  const floor = Math.floor(scaled);
  const fraction = scaled - floor;
  if (fraction > 0.5) return (floor + 1) / factor;
  if (fraction < 0.5) return floor / factor;
  return (floor % 2 === 0 ? floor : floor + 1) / factor;
}

export function floatToString(value) {
  if (!Number.isFinite(value)) {
    if (Number.isNaN(value)) return "nan";
    return value > 0 ? "inf" : "-inf";
  }
  if (value === 0) return Object.is(value, -0) ? "-0.0" : "0.0";

  const magnitude = Math.abs(value);
  const needsExponent = magnitude < 1e-4 || magnitude >= 1e16;
  if (!needsExponent) {
    return Number.isInteger(value) ? `${value}.0` : String(value);
  }

  const [mantissa, exponent] = value.toExponential().split("e");
  const exponentValue = Number(exponent);
  const sign = exponentValue < 0 ? "-" : "+";
  return `${mantissa}e${sign}${String(Math.abs(exponentValue)).padStart(2, "0")}`;
}

export function isFiniteNumber(value) {
  return typeof value === "number" && Number.isFinite(value);
}

export function sampleStdDev(values) {
  if (values.length < 2) return Number.NaN;
  const average = mean(values);
  const squaredDeviations = values.map((value) => (value - average) ** 2);
  return Math.sqrt(sum(squaredDeviations) / (values.length - 1));
}

const T_CRITICAL_95 = { 1: 12.706, 2: 4.303 };

export function metricSummary(values) {
  const average = mean(values);
  if (values.length < 2) return [average, 0, 0];
  const deviation = sampleStdDev(values);
  const tCritical = T_CRITICAL_95[values.length - 1] ?? 1.96;
  return [
    average,
    deviation,
    (tCritical * deviation) / Math.sqrt(values.length),
  ];
}
