// @ts-check


export function osPathJoin(...parts) {
  const [first = "", ...rest] = parts;
  let joined = first;
  for (const part of rest) {
    if (part.startsWith("/")) joined = part;
    else if (joined === "" || joined.endsWith("/")) joined += part;
    else joined += `/${part}`;
  }
  return joined;
}
