// @ts-check




export class ArgumentParser {
  #program;
  #description;
  #epilog;
  #options = new Map();

  constructor(init) {
    this.#program = init.program;
    this.#description = init.description ?? "";
    this.#epilog = init.epilog ?? "";
  }

  addOption(flag, spec) {
    this.#options.set(flag, spec);
    return this;
  }

  usage() {
    const lines = [];
    if (this.#description) lines.push(this.#description, "");
    lines.push(`Usage: node ${this.#program} <command> [options]`, "");
    lines.push("Options:");
    for (const [flag, spec] of this.#options) {
      const placeholder =
        spec.type === "boolean"
          ? ""
          : spec.type === "list"
            ? " <value...>"
            : " <value>";
      const overrides = Object.entries(spec.defaultByCommand ?? {})
        .map(([name, value]) => `${name}: ${JSON.stringify(value)}`)
        .join(", ");
      const fallback =
        spec.default === undefined ? "" : ` (default: ${JSON.stringify(spec.default)})`;
      const suffix = overrides ? `${fallback}, ${overrides}`.replace(" ()", " (") : fallback;
      lines.push(`  ${flag}${placeholder}`.padEnd(30) + `${spec.help ?? ""}${suffix}`);
    }
    if (this.#epilog) lines.push("", this.#epilog);
    return lines.join("\n");
  }

  parse(argv) {
    const leading = argv[0];
    const command =
      leading !== undefined && !leading.startsWith("-") ? leading : null;

    const options = {};
    for (const [flag, spec] of this.#options) {
      const override = command ? spec.defaultByCommand?.[command] : undefined;
      if (override !== undefined) options[flag] = override;
      else if (spec.default !== undefined) options[flag] = spec.default;
    }

    const positionals = [];
    let index = 0;
    while (index < argv.length) {
      const token = argv[index];
      const spec = this.#options.get(token);
      if (!spec) {
        if (token.startsWith("-") && token !== "-") {
          throw new Error(`unrecognized argument: ${token}`);
        }
        positionals.push(token);
        index += 1;
        continue;
      }

      if (spec.type === "boolean") {
        options[token] = true;
        index += 1;
        continue;
      }

      const value = argv[index + 1];
      if (value === undefined) {
        throw new Error(`argument ${token}: expected a value`);
      }
      if (spec.type === "int") {
        const parsed = Number.parseInt(value, 10);
        if (!Number.isFinite(parsed)) {
          throw new Error(`argument ${token}: invalid int value: '${value}'`);
        }
        options[token] = parsed;
        index += 2;
        continue;
      }
      if (spec.type === "list") {
        const values = [];
        let cursor = index + 1;
        while (cursor < argv.length && !argv[cursor].startsWith("-")) {
          values.push(argv[cursor]);
          cursor += 1;
        }
        if (values.length === 0) {
          throw new Error(`argument ${token}: expected at least one value`);
        }
        options[token] = values.flatMap((part) => part.split(",")).filter(Boolean);
        index = cursor;
        continue;
      }
      options[token] = value;
      index += 2;
    }

    const [resolvedCommand, ...rest] = positionals;
    return { command: resolvedCommand ?? null, options, positionals: rest };
  }
}

export function optionString(options, flag) {
  const value = options[flag];
  return typeof value === "string" ? value : null;
}

export function optionInt(options, flag) {
  const value = options[flag];
  if (typeof value === "number") return value;
  if (typeof value === "string") {
    const parsed = Number.parseInt(value, 10);
    return Number.isFinite(parsed) ? parsed : undefined;
  }
  return undefined;
}

export function optionList(options, flag) {
  const value = options[flag];
  return Array.isArray(value) ? value : [];
}
