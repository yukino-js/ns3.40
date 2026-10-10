// @ts-check

/**
 * Run `worker` over `items` with at most `concurrency` calls in flight.
 *
 * Each worker invocation must resolve to a boolean success flag. Results are
 * returned once every item has settled; the pool never rejects on a worker
 * failure, so a single failed run does not abort a campaign.
 *
 * @template T
 * @param {T[]} items
 * @param {(item: T, index: number) => Promise<boolean>} worker
 * @param {number} [concurrency]
 * @returns {Promise<{ total: number, done: number, failed: number, failures: T[] }>}
 */
export async function runWithConcurrency(items, worker, concurrency = 1) {
  const limit = Math.min(Math.max(1, Math.trunc(concurrency)), Math.max(items.length, 1));
  let cursor = 0;
  let done = 0;
  let failed = 0;
  const failures = [];

  async function drain() {
    while (cursor < items.length) {
      const index = cursor;
      cursor += 1;
      const ok = await worker(items[index], index);
      done += 1;
      if (!ok) {
        failed += 1;
        failures.push(items[index]);
      }
    }
  }

  await Promise.all(Array.from({ length: limit }, () => drain()));
  return { total: items.length, done, failed, failures };
}
