// Generated from crates/grkit-core/src/plugins.rs. Do not edit: change the
// registry there, then run
// GRKIT_UPDATE_GENERATED=1 cargo test -p grkit-core --test generated.

/** The plugins in menu order. `needsEffects`: the plugin shows only with effects. */
export const PLUGINS = [
  { key: 'imageControl', plugin: 'image-control', needsEffects: true },
  { key: 'framing', plugin: 'framing', needsEffects: false },
  { key: 'guides', plugin: 'guides', needsEffects: false },
  { key: 'dateStamp', plugin: 'date-stamp', needsEffects: false },
  { key: 'grain', plugin: 'grain', needsEffects: false },
  { key: 'bloom', plugin: 'bloom', needsEffects: false },
  { key: 'halation', plugin: 'halation', needsEffects: false },
  { key: 'blackFloor', plugin: 'black-floor', needsEffects: false },
  { key: 'splitToning', plugin: 'split-toning', needsEffects: false },
  { key: 'colourRange', plugin: 'colour-range', needsEffects: false },
  { key: 'colourSaturation', plugin: 'colour-saturation', needsEffects: false },
  { key: 'colourHue', plugin: 'colour-hue', needsEffects: false },
  { key: 'colourBrightness', plugin: 'colour-brightness', needsEffects: false },
  { key: 'highlightRolloff', plugin: 'highlight-rolloff', needsEffects: false },
  { key: 'softShortcut', plugin: 'soft-shortcut', needsEffects: false },
] as const;

export type PluginKey = (typeof PLUGINS)[number]['key'];

/** The plugins that the runtime of each camera has, in menu order. */
export const CAMERA_PLUGINS = {
  gr3: ['imageControl', 'framing', 'guides', 'dateStamp', 'grain', 'bloom', 'halation', 'blackFloor', 'splitToning', 'colourRange', 'colourSaturation', 'colourHue', 'colourBrightness', 'highlightRolloff', 'softShortcut'],
  gr3x: ['imageControl', 'framing', 'guides', 'dateStamp', 'grain', 'bloom', 'halation', 'blackFloor', 'splitToning', 'colourRange', 'colourSaturation', 'colourHue', 'colourBrightness', 'highlightRolloff', 'softShortcut'],
  gr4: ['imageControl', 'framing', 'guides', 'dateStamp', 'grain', 'bloom', 'halation', 'blackFloor', 'splitToning', 'colourRange', 'colourSaturation', 'colourHue', 'colourBrightness', 'highlightRolloff'],
} as const satisfies Readonly<Record<string, readonly PluginKey[]>>;
