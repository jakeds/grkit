import { describe, expect, it } from 'vitest';
import { CAMERAS } from './cameras';
import { ALL_MODULES, planModules } from './modules';

describe('camera packages', () => {
  it.each([
    ['gr3', 'gr3', '2.10', 'gr3-2.10', '00078350.588'],
    ['gr3x', 'gr3x', '1.60', 'gr3-2.10', '00078490.609'],
    ['gr4', 'gr4', '1.11', 'gr4-1.11', '00078560.636'],
    ['gr4m', 'gr4', '1.00', 'gr4-1.11', '00078560.636'],
  ])('routes %s to its firmware, activation command and runtime', (id, pkg, firmware, target, modeset) => {
    expect(CAMERAS.find(camera => camera.id === id)?.support).toMatchObject({ package: pkg, firmware, target, modeset });
  });

  it('excludes Image Control from Monochrome packages even when globally enabled', () => {
    const support = CAMERAS.find(camera => camera.id === 'gr4m')!.support!;
    const plan = planModules(ALL_MODULES, 6, support.modules, support.excluded);
    expect(plan.includeEffects).toBe(false);
    expect(plan.visible).not.toContain('imageControl');
    expect(plan.omitted).toContain('image-control');
    expect(plan.omitted).not.toContain('soft-shortcut');
  });
});
