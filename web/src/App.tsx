import { Suspense, useRef, useState } from 'react';
import { CAMERAS } from './cameras';
import { loadCatalog, siteUrl } from './catalog';
import { EffectsSection } from './components/EffectsSection';
import { ErrorBoundary } from './components/ErrorBoundary';
import { STRINGS, type Language } from './i18n';
import { ALL_MODULES, MODULES, planModules, type ModuleKey } from './modules';
import { buildPackage, buildScriptsOffPackage, save } from './package';
import { DEFAULT_SELECTION } from './curated';

const MODULE_ICONS: Partial<Record<ModuleKey, string>> = { imageControl: 'looks', framing: 'crop', dateStamp: 'date-stamp', blackFloor: 'black-floor', splitToning: 'split-toning', colourRange: 'colour', colourSaturation: 'saturation', colourHue: 'hue', colourBrightness: 'brightness', highlightRolloff: 'highlight-roll-off' };

const CONTROL_GROUPS: { name: string; keys: ModuleKey[] }[] = [
  { name: 'Looks & framing', keys: ['imageControl', 'framing', 'guides'] },
  { name: 'Tone', keys: ['blackFloor', 'highlightRolloff', 'splitToning'] },
  { name: 'Colour', keys: ['colourRange', 'colourSaturation', 'colourHue', 'colourBrightness'] },
  { name: 'Texture & light', keys: ['grain', 'bloom', 'halation'] },
  { name: 'Date stamp', keys: ['dateStamp'] },
];

const CONTROL_COPY: Partial<Record<ModuleKey, { name: string; description: string }>> = {
  imageControl: { name: 'Film looks', description: 'Your selected looks, with live view.' },
  framing: { name: 'Crop', description: '6x7, 5:4, FP100C, 4:3 and 16:9.' },
  guides: { name: 'Guides', description: 'Thirds, centre, golden ratio and diagonals.' },
  blackFloor: { name: 'Black lift', description: 'Lift deep blacks: subtle to a strong matte finish.' },
  highlightRolloff: { name: 'Highlight roll-off', description: 'A gentler finish to bright tones.' },
  splitToning: { name: 'Split toning', description: 'Warm cream, cocoa and amber, from subtle to pronounced.' },
  colourRange: { name: 'Selective colour', description: 'Edit seven colour ranges independently.' },
  colourSaturation: { name: 'Saturation', description: 'Mute or enrich each colour.' },
  colourHue: { name: 'Hue', description: 'Shift each colour’s tone.' },
  colourBrightness: { name: 'Brightness', description: 'Lighten or darken each colour.' },
  grain: { name: 'Grain', description: 'Native Ricoh grain, with a moment-based seed.' },
  bloom: { name: 'Bloom', description: 'Soft glow around highlights.' },
  halation: { name: 'Halation', description: 'Warm halos around bright edges.' },
  dateStamp: { name: 'Date stamp', description: 'Classic, dot matrix or small mono.' },
};

const REPOSITORY = 'https://github.com/jakeds/grkit';

type Build = { state: 'idle' } | { state: 'building' } | { state: 'ready' } | { state: 'failed' };

export function App() {
  const language: Language = 'en';
  const [cameraId, setCameraId] = useState('gr3x');
  const [selection, setSelection] = useState<ReadonlySet<string>>(() => new Set(DEFAULT_SELECTION));
  const [enabled, setEnabled] = useState<ReadonlySet<ModuleKey>>(() => new Set([...ALL_MODULES].filter((key) => key !== 'softShortcut')));
  const [build, setBuild] = useState<Build>({ state: 'idle' });
  const [removeState, setRemoveState] = useState('');
  // A newer build or a changed selection makes the result of an older build stale.
  const buildRun = useRef(0);

  const t = STRINGS[language];
  const camera = CAMERAS.find((item) => item.id === cameraId) ?? CAMERAS[0]!;
  const support = camera.support;
  const available = support?.modules ?? [];
  const plan = planModules(enabled, selection.size, available, support?.excluded);
  const effectCount = plan.includeEffects ? selection.size : 0;

  const filename = `Soft-${cameraId === 'gr4m' ? 'GRIV-Monochrome' : support?.package === 'gr3' ? 'GRIII' : support?.package === 'gr4' ? 'GRIV' : 'GRIIIx'}.zip`;

  function chooseCamera(id: string) {
    setCameraId(id);
    setRemoveState('');
    resetBuild();
  }

  async function downloadScriptsOff() {
    if (!support) return;
    const run = buildRun.current;
    setRemoveState('Preparing…');
    try { const blob = await buildScriptsOffPackage(support.package); if (run !== buildRun.current) return; save(blob, filename.replace('.zip', '-scripts-off.zip')); setRemoveState('Script-disable package downloaded.'); }
    catch { if (run === buildRun.current) setRemoveState('Could not prepare the package. Please try again.'); }
  }

  function resetBuild() {
    buildRun.current += 1;
    setBuild({ state: 'idle' });
  }

  function changeSelection(next: ReadonlySet<string>) {
    setSelection(next);
    resetBuild();
  }

  function toggleModule(key: ModuleKey) {
    const next = new Set(enabled);
    if (!next.delete(key)) {
      next.add(key);
      if (key === 'colourRange') {
        next.add('colourSaturation');
        next.add('colourHue');
        next.add('colourBrightness');
      } else if (key === 'colourSaturation' || key === 'colourHue' || key === 'colourBrightness') {
        next.add('colourRange');
      }
    } else if (key === 'colourRange') {
      next.delete('colourSaturation');
      next.delete('colourHue');
      next.delete('colourBrightness');
    }
    setEnabled(next);
    resetBuild();
  }

  async function download() {
    if (!support || plan.visible.length === 0) {
      return;
    }
    buildRun.current += 1;
    const run = buildRun.current;
    setBuild({ state: 'building' });
    try {
      const effects = plan.includeEffects
        ? (await loadCatalog(support.target)).effects.filter((effect) => selection.has(effect.id))
        : [];
      const blob = await buildPackage(support.target, support.package, effects, plan.omitted, language);
      if (run === buildRun.current) {
        save(blob, filename);
        setBuild({ state: 'ready' });
      }
    } catch (error) {
      console.error(error);
      if (run === buildRun.current) {
        setBuild({ state: 'failed' });
      }
    }
  }

  return (
    <div className="page">
      <header className="masthead">
        <div className="brand-row"><h1>Soft</h1><a className="button header-download" href="#download">Get your kit ↓</a></div>
        <p className="lead">A little more film in your GR.</p>
        <div className="camera-picker"><label htmlFor="camera">Your camera</label><select id="camera" value={cameraId} onChange={event => chooseCamera(event.target.value)}>{CAMERAS.map(item => <option key={item.id} value={item.id}>{item.name}</option>)}</select><span>Firmware {support?.firmware} only</span></div>
        <p className="note">Choose your looks and controls. Download them to your SD card.</p>
        <nav className="builder-nav" aria-label="Builder sections"><a href="#looks">01 Looks</a><a href="#controls">02 Controls</a><a href="#download">03 Download</a></nav>
      </header>

      <main>
        <section id="looks" className="section" aria-labelledby="effects-label">
          <h2 id="effects-label"><span className="section-number">01</span>Film looks</h2>
          {!available.includes('imageControl') ? (
            <p className="status section-body">Film looks are unavailable on GR IV Monochrome.</p>
          ) : plan.includeEffects ? (
            <ErrorBoundary fallback={<p className="status error section-body">{t.loadError}</p>}>
              <Suspense fallback={<p className="status section-body">{t.loading}</p>}>
                <EffectsSection
                  key={support?.target}
                  strings={t}
                  target={support!.target}
                  selection={selection}
                  onSelectionChange={changeSelection}
                />
              </Suspense>
            </ErrorBoundary>
          ) : (
            <p className="status section-body">{t.effectsOff}</p>
          )}
        </section>


        <section id="controls" className="section" aria-labelledby="modules-label">
          <h2 id="modules-label"><span className="section-number">02</span>Camera controls</h2>
          <div className="section-body">
            <p className="note">Tick to include a control. Adjust its strength on the camera; new effects start off or neutral.</p>
            <div role="group" aria-labelledby="modules-label" className="modules">
              {CONTROL_GROUPS.map((group) => <fieldset className="control-group" key={group.name}><legend>{group.name}</legend><div className="control-options">{group.keys.flatMap(key => MODULES.filter(module => module.key === key && available.includes(key))).map((module) => (
                <label key={module.key} className="module">
                  <input
                    type="checkbox"
                    aria-label={CONTROL_COPY[module.key]?.name ?? t.moduleNames[module.key]}
                    checked={enabled.has(module.key)}
                    onChange={() => toggleModule(module.key)}
                  />
                  <span className="module-text">
                    <span className="module-heading">
                      <img className="module-icon" src={siteUrl(`menu-icons/${MODULE_ICONS[module.key] ?? module.plugin}.png`)} alt="" width={32} height={32} />
                      <span className="module-name">{CONTROL_COPY[module.key]?.name ?? t.moduleNames[module.key]}</span>
                    </span>
                    <span className="module-description">
                      {module.key === 'framing' && support?.xpanOnly ? 'XPan crop. Use the camera’s native menu for 4:3 and 16:9.' : module.key === 'grain' && support?.target === 'gr4-1.11' ? 'Native grain: Weak, Medium or Strong.' : CONTROL_COPY[module.key]?.description ?? t.modulesText[module.key]}
                    </span>
                    {module.key === 'imageControl' &&
                      enabled.has('imageControl') &&
                      selection.size === 0 && (
                        <span className="module-warning">{t.imageControlEmpty}</span>
                      )}
                  </span>
                </label>
              ))}</div></fieldset>)}
            </div>
          </div>
        </section>

        <section id="download" className="section" aria-labelledby="package-label">
          <h2 id="package-label"><span className="section-number">03</span>Download</h2>
          <div className="package">
            <dl>
              <dt>{t.camera}</dt>
              <dd>{camera.name}</dd>
              <dt>{t.firmware}</dt>
              <dd>{support?.firmware ?? '—'}</dd>
              <dt>Included</dt>
              <dd>{effectCount} looks · {plan.visible.length} controls</dd>
            </dl>
            <div className="package-action">
              <p className="package-caution">Experimental camera build. Soft’s custom features still need physical testing on each camera model.</p>
              <button
                type="button"
                className="download"
                disabled={!support || plan.visible.length === 0 || build.state === 'building'}
                aria-busy={build.state === 'building'}
                onClick={download}
              >
                {build.state === 'building' ? 'Preparing your kit…' : 'Download your kit ↓'}
              </button>
              <div aria-live="polite">
                {plan.visible.length === 0 && <p className="status error">{t.noModules}</p>}
                {build.state === 'building' && <p className="status">{t.building}</p>}
                {build.state === 'ready' && <p className="status">{filename} is ready.</p>}
                {build.state === 'failed' && (
                  <p className="status error">{t.buildError}</p>
                )}
              </div>
            </div>
          </div>
        </section>

        <section className="section" aria-labelledby="install-label">
          <h2 id="install-label"><span className="section-number">04</span>Install</h2>
          <div className="section-body">
          <details className="help-panel" open>
            <summary>First time? Enable SD scripts</summary>
            <p>Do this once so the camera can load Soft from your card. If Soft already runs on your camera, skip to the update instructions below.</p>
            <ol className="steps">
              <li><strong>Check your camera.</strong> These files are for {camera.name} firmware {support?.firmware} only. Turn the camera off.</li>
              <li><strong>Prepare the card.</strong> Unzip <code>{filename}</code>. Copy everything inside <code>SD_ROOT</code> to the top level of the SD card, alongside DCIM — not inside it. You should see <code>SCRIPT</code>, <code>{support?.modeset}</code> and <code>DEVELOP.MOD</code>.</li>
              <li><strong>Enable scripts.</strong> Insert the card. Hold the camera’s <strong>MENU</strong> button and, while holding it, press the power button.</li>
              <li><strong>Wait for confirmation.</strong> The screen shows <strong>Mode Set Command</strong>, <strong>Script - Enable</strong>, then <strong>Complete</strong>. Wait for the camera to turn itself off.</li>
              <li><strong>Start normally.</strong> Turn the camera on without holding MENU. Give Soft a few seconds to load.</li>
              <li><strong>Open Soft.</strong> {support?.target === 'gr4-1.11' ? 'Press ADJ and select the extra Soft entry.' : <>In Customize Controls → ADJ Mode Setting, assign <strong>Image Control</strong> to a slot. Press ADJ and select that slot. Set the other four slots to Off if you want only Soft in the toolbar.</>}</li>
            </ol>
            <p>Once enabled, the script setting stays on, including when you switch SD cards. Keep <code>SCRIPT</code> on each card you want to use with Soft. You can remove <code>{support?.modeset}</code> and <code>DEVELOP.MOD</code> after activation.</p>
            <p>If the activation screen does not appear, check that those two files are at the card’s top level and that they came from the package for your selected camera.</p>
          </details>
          <details className="help-panel">
            <summary>Update an existing Soft card</summary>
            <p>Turn the camera off and back up your card. Unzip your kit and copy its <code>SD_ROOT/SCRIPT</code> folder to the card root, replacing the existing Soft files. Turn the camera on normally. You do not need to enable scripts again.</p>
            <p>These instructions describe the current Soft menus. The included upstream PDF has older menu screenshots.</p>
          </details>
          <details className="help-panel">
            <summary>Using the camera menus</summary>
            <p>Up / Down selects. Right opens. Left goes back. OK applies. Press ADJ to close.</p>
            <p>Most strengths use 5% steps. GR IV grain uses Weak, Medium and Strong. Split toning has separate preset and strength controls; date stamps have separate format and appearance controls.</p>
          </details>
          <details className="help-panel">
            <summary>Camera differences and experimental features</summary>
            <p>III and IIIx share a runtime; HDF models use their base camera’s package. GR IV uses a separate runtime. Its grain uses native strengths and its extra crop is XPan. GR IV Monochrome excludes colour film looks.</p>
            <p>Black lift and split toning have a stronger range in this update; saved percentages will look stronger. Grain keeps its existing Ricoh amplitude scale. New crop modes, III-series grain seeding and JPEG effects have software checks but still need physical camera testing. Bloom and halation may increase saving time. This is an SD-card runtime, not a replacement firmware image.</p>
          </details>
          </div>
        </section>
        <section className="section support-section" aria-labelledby="remove-label">
          <h2 id="remove-label">Remove</h2>
          <div className="section-body">
            <details className="help-panel"><summary>Remove Soft from your card</summary>
              <p>Turn the camera off and remove the SCRIPT folder from the card to stop loading Soft.</p>
              <p>To also disable the persistent SD-script setting, copy this package’s SD_ROOT contents to the card root. Hold MENU while powering on and wait for the camera to switch off. This disable command still needs a physical camera check.</p>
              <button className="button" type="button" onClick={downloadScriptsOff}>Download script-disable package</button>
              <p className="status" aria-live="polite">{removeState}</p>
            </details>
          </div>
        </section>
      </main>

      <footer className="footer">
        <p>{t.trademarks}</p>
        <div>
          <a href={`${REPOSITORY}/blob/jake/gr3x-seeded-grain/effects/NOTICE.md`}>{t.licenses}</a>
          <a href={siteUrl('THIRD_PARTY_LICENSES.txt')}>{t.thirdParty}</a>
          <a href={`${REPOSITORY}/blob/jake/gr3x-seeded-grain/resources/RECOVERY${language === 'en' ? '' : `.${language}`}.md`}>
            {t.recovery}
          </a>
          <a href={`${REPOSITORY}/tree/jake/gr3x-seeded-grain`}>{t.source}</a>
          <span>Soft / GRKit</span>
        </div>
      </footer>
    </div>
  );
}
