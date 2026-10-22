import { CAMERA_PLUGINS } from './generated/plugins';
import type { ModuleKey } from './modules';

export interface Camera {
  id: string;
  name: string;
  /**
   * The supported firmware version, the site target that holds its runtime, and the ModeSet file
   * that enables SD scripts. A camera without it is planned and cannot be selected.
   */
  support?: {
    /** The camera of the package (`camera.rs`): it selects the ModeSet file and the guide. */
    package: keyof typeof CAMERA_PLUGINS;
    firmware: string;
    target: string;
    modeset: string;
    /** True until GRKit has been tested on this camera. */
    experimental: boolean;
    /** The modules that the runtime has, in menu order (`camera.rs`, through `CAMERA_PLUGINS`). */
    modules: readonly ModuleKey[];
    /** Framing offers only XPan, as a crop of native 16:9. */
    xpanOnly?: boolean;
  };
}

export const CAMERAS: readonly Camera[] = [
  {
    id: 'gr3',
    name: 'GR III',
    support: {
      package: 'gr3',
      firmware: '2.10',
      target: 'gr3-2.10',
      modeset: '00078350.588',
      experimental: false,
      modules: CAMERA_PLUGINS.gr3,
    },
  },
  // GR III HDF runs the GR III firmware and uses its package. It has not been tested on a camera.
  {
    id: 'gr3hdf',
    name: 'GR III HDF',
    support: {
      package: 'gr3',
      firmware: '2.10',
      target: 'gr3-2.10',
      modeset: '00078350.588',
      experimental: true,
      modules: CAMERA_PLUGINS.gr3,
    },
  },
  // GR IIIx 1.60 runs the GR III 2.10 code, so it uses the same runtime.
  {
    id: 'gr3x',
    name: 'GR IIIx',
    support: {
      package: 'gr3x',
      firmware: '1.60',
      target: 'gr3-2.10',
      modeset: '00078490.609',
      experimental: true,
      modules: CAMERA_PLUGINS.gr3x,
    },
  },
  {
    id: 'gr3xhdf',
    name: 'GR IIIx HDF',
    support: {
      package: 'gr3x',
      firmware: '1.60',
      target: 'gr3-2.10',
      modeset: '00078490.609',
      experimental: true,
      modules: CAMERA_PLUGINS.gr3x,
    },
  },
  // GR IV and GR IV HDF run the same firmware.
  {
    id: 'gr4',
    name: 'GR IV / GR IV HDF',
    support: {
      package: 'gr4',
      firmware: '1.11',
      target: 'gr4-1.11',
      modeset: '00078560.636',
      experimental: true,
      modules: CAMERA_PLUGINS.gr4,
      xpanOnly: true,
    },
  },
];

/** The token in the install steps that stands for the camera's ModeSet file. */
export const MODESET_TOKEN = '`{modeset}';

/** The target whose effects and previews the site shows. */
export const PREVIEW_TARGET = 'gr3-2.10';
