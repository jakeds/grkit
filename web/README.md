# Soft builder

Public builder: https://soft-gr.jakeds372351.chatgpt.site

The selector supports GR III / HDF 2.10, GR IIIx / HDF 1.60, GR IV / HDF 1.11 and experimental GR IV Monochrome 1.00. The browser selects film looks and camera controls and makes `Soft-GRIIIx.zip` locally. The on-camera runtime is experimental; browser validation does not prove physical camera behaviour.

After building the matching WASM, runtime, effect tables and guides as described below, run `pnpm build` in `web/`, then `node web/scripts/export-site.mjs` from the repository root. The complete static site is written to `dist/soft-site`, including runtime data and licences. A web-only deployment without these matching files cannot make packages.

The public GPT Site is a static snapshot of that directory; its source reference identifies the corresponding GitHub commit. Keep firmware inputs and decoded firmware outside the published site and Git.

---

# GRKit site

The site lets a user select a camera, controls and film looks, and download a model-specific Soft ZIP. There is no photo preview or upload. The browser builds the package with `grkit-wasm`. No server code runs.

The site serves these files:

| Path | Source |
|---|---|
| `index.html`, `assets/`, `_headers`, `THIRD_PARTY_LICENSES.txt` | `pnpm build` in this directory |
| `runtime/gr3-2.10/`, `runtime/gr4-1.11/` | `grkit site` |
| `effects/gr3-2.10/`, `effects/gr4-1.11/` | `grkit effects publish` |

`THIRD_PARTY_LICENSES.txt` gives the licenses of the bundled JavaScript packages and fonts, and of the Rust crates in the WebAssembly module. `pnpm build` writes it with `scripts/third-party-licenses.mjs`. The script reads the JavaScript licenses from the Vite build and the crate licenses from `cargo metadata`, so Cargo must be on `PATH`.

The site serves its fonts from `assets/` and sends no request to another host. The Noto Sans SC and TC styles load only on a Chinese page, and the Noto Sans JP styles only on a Japanese page.

## Develop

The tools are Node.js 24, pnpm, the `wasm32-unknown-unknown` Rust target, and wasm-bindgen 0.2.128. The wasm-bindgen version must equal the version that `Cargo.toml` pins.

1. Install the dependencies: `pnpm install`.
2. Build the browser bindings: `pnpm wasm`. If `wasm-bindgen` is not on PATH, set `WASM_BINDGEN` to its path.
3. Make the site data from the repository root. The commands need the official firmware and the ARM toolchain. See [CONTRIBUTING.md](../CONTRIBUTING.md).

   ```sh
   grkit build --target gr3-2.10 --firmware fwdc239b.bin --out dist/runtime-local
   grkit site --build dist/runtime-local --out dist/site-data
   grkit effects publish --source effects --firmware fwdc239b.bin --out dist/site-data/effects/gr3-2.10
   sh .github/scripts/install-guide-tools.sh dist/guide-tools
   sh .github/scripts/build-guides.sh grkit dist/guide-tools dist/site-data/guides
   ```

   For GR IV, also build `gr4-1.11` from `fwdc248b.bin`, add it with `grkit site`, and publish its effects with `--target gr4-1.11`. Without them, only the GR III and GR IIIx packages work.

4. Start the development server: `pnpm dev`. The server reads the site data from `../dist/site-data`. To use another directory, set `GRKIT_SITE_DATA`.

To add Plausible analytics to a build, set `PLAUSIBLE_HOST` to the Plausible origin, for example `https://plausible.example.com`. The build then adds the script to `index.html` and allows it in the Content-Security-Policy of `_headers`. Release reads the repository variable `PLAUSIBLE_HOST`.

The site data must come from the current source. If the runtime templates change, the browser rejects an older runtime with "Runtime scripts differ from the production templates".

## Check

```sh
pnpm typecheck
pnpm test
pnpm build
```

## Camera differences

The III-series uses gr3-2.10; GR IV and Monochrome use gr4-1.11. Publish both runtime directories, both effect catalogs and all three base-camera guides. Monochrome uses the GR IV package and activation file while excluding Image Control and all film looks. HDF variants use their base-camera package.

The old preview-processing source and tests remain in the repository as reference, but the builder does not import, load or display the previewer.
