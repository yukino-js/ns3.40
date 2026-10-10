#!/usr/bin/env node
import { execFileSync } from 'node:child_process';
import { existsSync, statSync } from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const ROOT = path.dirname(fileURLToPath(import.meta.url));

const RELEASE_NAME = 'ns3-docs';

const ARTIFACTS = [
  'docs/NJUPT_Professional_Thesis_draft1/NJUPT_Professional_Thesis_d1.pdf',
  'docs/thesis.pdf',
  'docs/nanjing-hangtiancheng-network-congestion-control.docx',
];

const args = process.argv.slice(2);

const flag = (name, fallback) => {
  const i = args.indexOf(name);
  return i >= 0 ? args[i + 1] : fallback;
};

const dryRun = args.includes('--dry-run');
const draft = args.includes('--draft');

const today = new Date().toISOString().slice(0, 10).replace(/-/g, '');
const tag = flag('--tag', `${RELEASE_NAME}-${today}`);

const files = ARTIFACTS.map((a) => path.join(ROOT, a));

const notes = flag('--notes', ARTIFACTS.map((a) => `- ${path.basename(a)}`).join('\n'));

function gh(...cmd) {
  return execFileSync('gh', cmd, {
    cwd: ROOT,
    encoding: 'utf8',
    stdio: ['ignore', 'pipe', 'pipe'],
  }).trim();
}

const missing = files.filter((f) => !existsSync(f));
if (missing.length) {
  console.error('Missing artifact files:\n' + missing.map((m) => `  ${m}`).join('\n'));
  process.exit(1);
}

const listing = files.map((f) => `${path.basename(f)} (${(statSync(f).size / 1024).toFixed(0)} KB)`);
console.log(`release ${RELEASE_NAME} / tag ${tag}\nUploading:\n  ${listing.join('\n  ')}`);

if (dryRun) {
  console.log('\n--dry-run: GitHub not contacted');
  process.exit(0);
}

try {
  gh('--version');
} catch {
  console.error('gh CLI not found. Install it first: brew install gh && gh auth login');
  process.exit(1);
}

let exists = false;
try {
  gh('release', 'view', tag, '--json', 'tagName');
  exists = true;
} catch {}

if (exists) {
  console.log(`\ntag ${tag} already exists, overwriting same-named artifacts`);
} else {
  gh(
    'release',
    'create',
    tag,
    '--title',
    RELEASE_NAME,
    '--notes',
    notes,
    ...(draft ? ['--draft'] : []),
  );
  console.log(`\nCreated release ${tag}`);
}

gh('release', 'upload', tag, '--clobber', ...files);
console.log(`Uploaded ${files.length} artifacts`);
console.log(gh('release', 'view', tag, '--json', 'url', '--jq', '.url'));
