// @ts-check


function encodeString(value) {
  let out = '"';
  for (const character of value) {
    const code = (character.codePointAt(0));
    switch (code) {
      case 0x08:
        out += "\\b";
        continue;
      case 0x09:
        out += "\\t";
        continue;
      case 0x0a:
        out += "\\n";
        continue;
      case 0x0c:
        out += "\\f";
        continue;
      case 0x0d:
        out += "\\r";
        continue;
      case 0x22:
        out += '\\"';
        continue;
      case 0x5c:
        out += "\\\\";
        continue;
      default:
        break;
    }
    if (code < 0x20 || code > 0x7e) {
      if (code > 0xffff) {
        const offset = code - 0x10000;
        const high = 0xd800 + (offset >> 10);
        const low = 0xdc00 + (offset & 0x3ff);
        out += `\\u${high.toString(16).padStart(4, "0")}`;
        out += `\\u${low.toString(16).padStart(4, "0")}`;
      } else {
        out += `\\u${code.toString(16).padStart(4, "0")}`;
      }
      continue;
    }
    out += character;
  }
  return `${out}"`;
}

function encodeNumber(value) {
  if (!Number.isFinite(value)) {
    if (Number.isNaN(value)) return "NaN";
    return value > 0 ? "Infinity" : "-Infinity";
  }
  return String(value);
}

export function pyJsonDumps(value, options = {}) {
  const indent = options.indent ?? 0;
  const level = options.level ?? 0;
  const pad = " ".repeat(indent * level);
  const childPad = " ".repeat(indent * (level + 1));

  if (value === null || value === undefined) return "null";
  const type = typeof value;
  if (type === "boolean") return value ? "true" : "false";
  if (type === "number") {
    return encodeNumber( (value));
  }
  if (type === "string") return encodeString( (value));

  if (Array.isArray(value)) {
    if (value.length === 0) return "[]";
    if (indent === 0) {
      return `[${value.map((item) => pyJsonDumps(item, { indent, level })).join(",")}]`;
    }
    const items = value.map(
      (item) => childPad + pyJsonDumps(item, { indent, level: level + 1 }),
    );
    return `[\n${items.join(",\n")}\n${pad}]`;
  }

  const entries = Object.entries( (value))
    .filter(([, item]) => item !== undefined)
    .sort(([a], [b]) => (a < b ? -1 : a > b ? 1 : 0));
  if (entries.length === 0) return "{}";
  if (indent === 0) {
    const pairs = entries.map(
      ([key, item]) =>
        `${encodeString(key)}:${pyJsonDumps(item, { indent, level })}`,
    );
    return `{${pairs.join(",")}}`;
  }
  const pairs = entries.map(
    ([key, item]) =>
      `${childPad}${encodeString(key)}: ${pyJsonDumps(item, {
        indent,
        level: level + 1,
      })}`,
  );
  return `{\n${pairs.join(",\n")}\n${pad}}`;
}

export function isoformatUtc(date = new Date()) {
  const pad = ( value, width = 2) =>
    String(value).padStart(width, "0");
  const base =
    `${date.getUTCFullYear()}-${pad(date.getUTCMonth() + 1)}-${pad(date.getUTCDate())}` +
    `T${pad(date.getUTCHours())}:${pad(date.getUTCMinutes())}:${pad(date.getUTCSeconds())}`;
  const microseconds = `${pad(date.getUTCMilliseconds(), 3)}000`;
  return `${base}.${microseconds}+00:00`;
}
