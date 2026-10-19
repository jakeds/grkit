import { describe, expect, it } from 'vitest';
import { CAMERAS } from './cameras';

describe('CAMERAS', () => {
  it('gives each camera the firmware, runtime, and ModeSet file of its package camera', () => {
    for (const camera of CAMERAS) {
      const support = camera.support;
      if (!support) continue;
      const base = CAMERAS.find((item) => item.id === support.package)?.support;
      expect(base, camera.id).toBeDefined();
      const { firmware, target, modeset, modules } = support;
      expect({ firmware, target, modeset, modules }, camera.id).toEqual({
        firmware: base!.firmware,
        target: base!.target,
        modeset: base!.modeset,
        modules: base!.modules,
      });
    }
  });

  it('marks the HDF models that have their own entry as not tested', () => {
    const hdf = CAMERAS.filter((camera) => camera.id.endsWith('hdf'));
    expect(hdf.map((camera) => camera.id)).toEqual(['gr3hdf', 'gr3xhdf']);
    expect(hdf.every((camera) => camera.support?.experimental)).toBe(true);
  });
});
