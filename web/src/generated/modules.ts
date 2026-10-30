// Generated from crates/grkit-core/src/modules.rs. Do not edit: change the
// list there, then run
// GRKIT_UPDATE_GENERATED=1 cargo test -p grkit-core --test generated.

/** The official modules in menu order. `needsEffects`: the module shows only with effects. */
export const OFFICIAL_MODULES = [
  { key: 'filmLook', id: 'FILMLOOK', needsEffects: true },
  { key: 'framing', id: 'FRAMING', needsEffects: false },
  { key: 'guides', id: 'GUIDES', needsEffects: false },
  { key: 'dateStamp', id: 'DATE', needsEffects: false },
  { key: 'grain', id: 'GRAIN', needsEffects: false },
  { key: 'softTone', id: 'SOFTTONE', needsEffects: false },
  { key: 'softLight', id: 'SOFTLITE', needsEffects: false },
  { key: 'softSplit', id: 'SOFTSPLT', needsEffects: false },
  { key: 'softColour', id: 'SOFTCOL', needsEffects: false },
] as const;

export type ModuleKey = (typeof OFFICIAL_MODULES)[number]['key'];

/** The official modules that each camera can install, in menu order. */
export const CAMERA_MODULES = {
  gr3: ['filmLook', 'framing', 'guides', 'dateStamp', 'grain', 'softTone', 'softLight', 'softSplit', 'softColour'],
  gr3x: ['filmLook', 'framing', 'guides', 'dateStamp', 'grain', 'softTone', 'softLight', 'softSplit', 'softColour'],
  gr4: ['filmLook', 'framing', 'guides', 'dateStamp', 'grain', 'softTone', 'softLight', 'softSplit', 'softColour'],
} as const satisfies Readonly<Record<string, readonly ModuleKey[]>>;
