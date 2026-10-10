#!/usr/bin/env node
// @ts-check


import { spawnSync } from "node:child_process";
import { readdir, rm } from "node:fs/promises";
import path from "node:path";
import process from "node:process";
import { fileURLToPath, pathToFileURL } from "node:url";

const DOCS_DIR = path.dirname(fileURLToPath(import.meta.url));

const THESIS_DIR = path.join(DOCS_DIR, "NJUPT_Professional_Thesis_draft1");

const AUXILIARY_SUFFIXES = [
  ".aux",
  ".bbl",
  ".blg",
  ".lof",
  ".log",
  ".lot",
  ".toc",
  ".out",
];

const AUTO_CLEAN_TARGETS = new Set(["njupt", "thesis", "all"]);

class BuildAbort extends Error {
  constructor() {
    super("build aborted");
    this.name = "BuildAbort";
  }
}

function log(message) {
  console.log(`[BUILD] ${message}`);
}

function warn(message) {
  console.warn(`[WARN] ${message}`);
}

function err(message) {
  console.error(`[ERROR] ${message}`);
}

export function runCommand(command, cwd, ignoreFailure = false) {
  const [executable, ...args] = command;
  const result = spawnSync(executable, args, {
    cwd,
    stdio: "ignore",
  });
  if (result.error) {
    err(`Command not found: ${executable}`);
    if (ignoreFailure) return false;
    throw result.error;
  }
  if (result.status !== 0) {
    if (ignoreFailure) return false;
    const failure = new Error(
      `Command failed with exit code ${result.status}: ${command.join(" ")}`,
    );
    throw failure;
  }
  return true;
}

export async function cleanTrash(directory = DOCS_DIR) {
  let entries;
  try {
    entries = await readdir(directory, { withFileTypes: true });
  } catch {
    return;
  }

  for (const entry of entries) {
    if (!entry.isFile()) continue;
    if (AUXILIARY_SUFFIXES.some((suffix) => entry.name.endsWith(suffix))) {
      await rm(path.join(directory, entry.name), { force: true });
    }
  }

  await removeNestedAux(directory, directory, 1);
}

async function removeNestedAux(root, current, depth) {
  if (depth > 2) return;
  let entries;
  try {
    entries = await readdir(current, { withFileTypes: true });
  } catch {
    return;
  }
  for (const entry of entries) {
    const full = path.join(current, entry.name);
    if (entry.isFile() && entry.name.endsWith(".aux")) {
      await rm(full, { force: true });
      continue;
    }
    if (entry.isDirectory()) {
      const relative = path.relative(root, full);
      if (relative.split(path.sep).length <= 2) {
        await removeNestedAux(root, full, depth + 1);
      }
    }
  }
}

export async function buildConferenceWithBib(tex, compiler = "xelatex") {
  const name = tex.replace(/\.tex$/, "");
  log(`Compiling ${tex} (with BibTeX)...`);

  await runPass(compiler, tex, DOCS_DIR, `Pass 1 failed for ${tex}. Please check ${name}.log for details.`);

  try {
    runCommand(["bibtex", name], DOCS_DIR);
  } catch {
    err(`BibTeX failed for ${tex}. Please check ${name}.blg for details.`);
    process.exitCode = 1;
    throw new BuildAbort();
  }

  await runPass(compiler, tex, DOCS_DIR, `Pass 2 failed for ${tex}. Please check ${name}.log for details.`);
  await runPass(compiler, tex, DOCS_DIR, `Pass 3 failed for ${tex}. Please check ${name}.log for details.`);

  log(`${name}.pdf successfully generated ✓`);
}

export async function buildWithBib(tex) {
  const name = tex.replace(/\.tex$/, "");
  log(`Compiling ${tex} (with BibTeX)...`);

  await runPass("xelatex", tex, THESIS_DIR, `Pass 1 failed for ${tex}.`);

  if (!runCommand(["bibtex", name], THESIS_DIR, true)) {
    warn(`bibtex generated warnings for ${name}.`);
  }

  await runPass("xelatex", tex, THESIS_DIR, `Pass 2 failed for ${tex}.`);
  await runPass("xelatex", tex, THESIS_DIR, `Pass 3 failed for ${tex}.`);

  log(`${name}.pdf successfully generated ✓`);
}

async function runPass(compiler, tex, cwd, message) {
  const command = [compiler, "-interaction=nonstopmode", "-halt-on-error", tex];
  try {
    runCommand(command, cwd);
  } catch {
    err(message);
    process.exitCode = 1;
    throw new BuildAbort();
  }
}

export function usage() {
  const program = path.basename(process.argv[1] ?? "build.js");
  console.log(`Usage: node ${program} <target...>`);
  console.log("");
  console.log("  njupt    Compile NJUPT master's thesis (with BibTeX)");
  console.log("  thesis   Compile Chinese conference paper (thesis.tex)");
  console.log("  all      Compile all targets above");
  console.log("  clean    Clean compilation auxiliary files");
}

export async function cleanAll() {
  log("Cleaning auxiliary files...");
  await cleanTrash(DOCS_DIR);
  await cleanTrash(THESIS_DIR);
  await cleanTrash(path.join(THESIS_DIR, "chapters"));
  log("Cleanup complete ✓");
}

export async function buildAllParallel() {
  const targets = ["njupt", "thesis"];
  const results = await Promise.allSettled(
    targets.map((target) => doBuild(target)),
  );
  for (const [index, result] of results.entries()) {
    if (result.status === "rejected") {
      err(`Parallel build failed for target: ${targets[index]}`);
      throw result.reason;
    }
  }
}

export async function doBuild(target) {
  if (target === "njupt") {
    await buildWithBib("NJUPT_Professional_Thesis_d1.tex");
    return;
  }
  if (target === "thesis") {
    await buildConferenceWithBib("thesis.tex", "xelatex");
    return;
  }
  if (target === "all") {
    await buildAllParallel();
    return;
  }
  if (target === "clean") {
    await cleanAll();
    return;
  }

  err(`Unknown target: ${target}`);
  usage();
  process.exitCode = 1;
  throw new BuildAbort();
}

export async function main(argv = process.argv.slice(2)) {
  if (argv.length === 0) {
    usage();
    return 0;
  }

  try {
    for (const target of argv) {
      await doBuild(target);
      if (AUTO_CLEAN_TARGETS.has(target)) {
        await cleanAll();
      }
    }
  } catch (error) {
    if (error instanceof BuildAbort) return 1;
    throw error;
  }

  log("All tasks completed successfully ✓");
  return 0;
}

const invokedDirectly =
  process.argv[1] !== undefined &&
  import.meta.url === pathToFileURL(path.resolve(process.argv[1])).href;

if (invokedDirectly) {
  main().then(
    (code) => {
      process.exitCode = code;
    },
    (error) => {
      console.error(error);
      process.exitCode = 1;
    },
  );
}
