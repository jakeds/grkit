/** Soft's restrained everyday selection, reviewed against the rendered preview. */
export const EVERYDAY_LOOKS = {
  'portra-400-endura-v1': 'Balanced colour with a gentle film tone. A good everyday starting point.',
  'portra-160-ultra-v1': 'Clean, softer colour with a restrained finish.',
  'pro-400h-crystal-v1': 'Cooler colour and a soft, quieter palette.',
  'gold-200-premier-v1': 'A warmer, slightly livelier everyday option.',
  'vision3-250d-2383-v1': 'Muted cinema colour with a gentle shift in tone.',
  'kodachrome-64-v1': 'A more subdued, faded palette in this simulation.',
} as const;
export const DEFAULT_SELECTION = Object.keys(EVERYDAY_LOOKS);
