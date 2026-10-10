// @ts-check


import path from "node:path";
import { createRequire } from "node:module";
import { accessSync, constants, readFileSync } from "node:fs";
import { chromium } from "playwright";


const require = createRequire(import.meta.url);

const CSS_PX_PER_INCH = 96;

const POINTS_PER_INCH = 72;

const PRINT_SCALE = CSS_PX_PER_INCH / POINTS_PER_INCH;

const FIGURE_ID = "figure";






function resolvePlotlyBundle() {
  const entry = require.resolve("plotly.js-dist-min");
  return path.join(path.dirname(entry), "plotly.min.js");
}

function readBundle(bundle) {
  try {
    accessSync(bundle, constants.R_OK);
  } catch {
    throw new Error(
      `plotly bundle not readable at ${bundle}; run "pnpm install" to restore it`,
    );
  }
  return readFileSync(bundle, "utf8");
}

function decodeDataUrl(dataUrl) {
  const separator = dataUrl.indexOf(",");
  if (separator < 0) {
    throw new Error(`unexpected data URL from Plotly.toImage: ${dataUrl.slice(0, 40)}`);
  }
  const header = dataUrl.slice(0, separator);
  const payload = dataUrl.slice(separator + 1);
  if (header.includes(";base64")) return Buffer.from(payload, "base64");
  return Buffer.from(decodeURIComponent(payload), "utf8");
}

export function retagSvgToPoints(svg, width, height) {
  return svg.replace(
    /<svg([^>]*?)\swidth="[^"]*"\sheight="[^"]*"/,
    `<svg$1 width="${width}pt" height="${height}pt"`,
  );
}

export class FigureRenderer {
  #browser;
  #bundleSource;
  #page = null;
  #closed = false;

  constructor(browser, bundleSource) {
    this.#browser = browser;
    this.#bundleSource = bundleSource;
  }

  static async open(options = {}) {
    const bundle = options.plotlyBundle ?? resolvePlotlyBundle();
    const source = readBundle(bundle);
    const browser = await chromium.launch();
    const renderer = new FigureRenderer(browser, source);
    if (options.verbose) {
      const page = await renderer.#ensurePage(800, 600);
      console.log(`[INFO] Rendering with plotly.js ${await renderer.#version(page)}`);
    }
    return renderer;
  }

  async #ensurePage(width, height) {
    const viewport = {
      width: Math.max(1, Math.ceil(width)),
      height: Math.max(1, Math.ceil(height)),
    };
    if (this.#page) {
      await this.#page.setViewportSize(viewport);
      return this.#page;
    }
    const page = await this.#browser.newPage({ viewport });
    await page.setContent(
      '<!doctype html><html><head><meta charset="utf-8"></head>' +
        `<body style="margin:0;padding:0;background:#ffffff"><div id="${FIGURE_ID}"></div></body></html>`,
    );
    await page.addScriptTag({ content: this.#bundleSource });
    this.#page = page;
    return page;
  }

  #version(page) {
    return page.evaluate(() => {
      const global = (
 (globalThis)
      );
      return (global.Plotly).version;
    });
  }

  #reset(page) {
    return page.evaluate((args) => {
      const global = (
 (globalThis)
      );
      const plotly = (global.Plotly);
      const document = (global.document);
      const existing = document.getElementById(args.figureId);
      if (existing) plotly.purge(existing);
      const replacement = document.createElement("div");
      replacement.id = args.figureId;
      document.body.replaceChildren(replacement);
    }, { figureId: FIGURE_ID });
  }

  async #draw(page, spec) {
    await this.#reset(page);
    const layout = {
      ...spec.layout,
      width: spec.width,
      height: spec.height,
      autosize: false,
    };
    const config = { staticPlot: true, displayModeBar: false, responsive: false };
    await page.evaluate(
      (args) => {
        const global = (
 (globalThis)
        );
        const plotly = (global.Plotly);
        return plotly.newPlot(
          args.figureId,
          args.data,
          args.layout,
          args.config,
        );
      },
      { figureId: FIGURE_ID, data: spec.data, layout, config },
    );
    await page.evaluate(() => {
      const global = (
 (globalThis)
      );
      const document = (global.document);
      return document.fonts?.ready ?? Promise.resolve();
    });
  }

  async render(spec, formats = { png: true }, pngScale = 1) {
    if (this.#closed) throw new Error("renderer is closed");
    const page = await this.#ensurePage(spec.width, spec.height);
    await this.#draw(page, spec);

    const output = {};

    if (formats.png === true) {
      const dataUrl = await this.#toImage(page, "png", spec, pngScale);
      output.png = decodeDataUrl(dataUrl);
    }

    if (formats.svg === true) {
      const dataUrl = await this.#toImage(page, "svg", spec, 1);
      output.svg = retagSvgToPoints(
        decodeDataUrl(dataUrl).toString("utf8"),
        spec.width,
        spec.height,
      );
    }

    if (formats.pdf === true) {
      output.pdf = await page.pdf({
        width: `${spec.width / POINTS_PER_INCH}in`,
        height: `${spec.height / POINTS_PER_INCH}in`,
        margin: { top: 0, right: 0, bottom: 0, left: 0 },
        printBackground: true,
        scale: PRINT_SCALE,
      });
    }

    return output;
  }

  #toImage(page, format, spec, scale) {
    return page.evaluate(
      (args) => {
        const global = (
 (globalThis)
        );
        const plotly = (global.Plotly);
        const document = (global.document);
        return plotly.toImage(document.getElementById(args.figureId), {
          format: args.format,
          width: args.width,
          height: args.height,
          scale: args.scale,
        });
      },
      {
        figureId: FIGURE_ID,
        format,
        width: spec.width,
        height: spec.height,
        scale,
      },
    );
  }

  async close() {
    if (this.#closed) return;
    this.#closed = true;
    this.#page = null;
    await this.#browser.close();
  }
}



export async function withRenderer(options, run) {
  const renderer = await FigureRenderer.open(options);
  try {
    return await run(renderer);
  } finally {
    await renderer.close();
  }
}
