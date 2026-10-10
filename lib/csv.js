// @ts-check


export function quoteField(value, delimiter) {
  const needsQuotes =
    value.includes(delimiter) ||
    value.includes('"') ||
    value.includes("\r") ||
    value.includes("\n");
  return needsQuotes ? `"${value.replace(/"/g, '""')}"` : value;
}

export function buildCsv(fieldnames, rows, options = {}) {
  const delimiter = options.delimiter ?? ",";
  const terminator = options.lineterminator ?? "\r\n";
  const lines = [
    fieldnames.map((name) => quoteField(name, delimiter)).join(delimiter),
  ];
  for (const row of rows) {
    lines.push(
      fieldnames
        .map((name) => {
          const value = row[name];
          return quoteField(value === undefined || value === null ? "" : String(value), delimiter);
        })
        .join(delimiter),
    );
  }
  return lines.join(terminator) + terminator;
}

export function parseCsv(text, delimiter = ",") {
  const records = parseCsvRecords(text, delimiter);
  if (records.length === 0) return [];
  const [header, ...rows] = records;
  return rows.map((cells) => {
    const row = {};
    header.forEach((name, index) => {
      row[name] = cells[index] ?? "";
    });
    return row;
  });
}

function parseCsvRecords(text, delimiter) {
  const records = [];
  let record = [];
  let field = "";
  let inQuotes = false;
  let index = 0;

  const endField = () => {
    record.push(field);
    field = "";
  };
  const endRecord = () => {
    endField();
    records.push(record);
    record = [];
  };

  while (index < text.length) {
    const char = text[index];
    if (inQuotes) {
      if (char === '"') {
        if (text[index + 1] === '"') {
          field += '"';
          index += 2;
          continue;
        }
        inQuotes = false;
        index += 1;
        continue;
      }
      field += char;
      index += 1;
      continue;
    }
    if (char === '"' && field === "") {
      inQuotes = true;
      index += 1;
      continue;
    }
    if (char === delimiter) {
      endField();
      index += 1;
      continue;
    }
    if (char === "\r" && text[index + 1] === "\n") {
      endRecord();
      index += 2;
      continue;
    }
    if (char === "\n") {
      endRecord();
      index += 1;
      continue;
    }
    field += char;
    index += 1;
  }

  if (field !== "" || record.length > 0) endRecord();
  return records.filter((cells) => !(cells.length === 1 && cells[0] === ""));
}
