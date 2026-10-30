import { Suspense, useRef, useState } from 'react';
import { CAMERAS } from './cameras';
import { loadCatalog, siteUrl } from './catalog';
import { EffectsSection } from './components/EffectsSection';
import { ErrorBoundary } from './components/ErrorBoundary';
import { STRINGS, type Language } from './i18n';
import { ALL_MODULES, MODULES, planModules, type ModuleKey } from './modules';
import { buildPackage, buildScriptsOff, save } from './package';
import { DEFAULT_SELECTION } from './curated';

const MODULE_ICONS: Record<ModuleKey,string> = {filmLook:'looks',framing:'crop',guides:'guides',dateStamp:'date-stamp',grain:'grain',softTone:'tone',softLight:'bloom',softSplit:'split-toning',softColour:'colour'};
const CONTROL_GROUPS: {name:string;keys:ModuleKey[]}[] = [{name:'Looks & framing',keys:['filmLook','framing','guides']},{name:'Tone & colour',keys:['softTone','softSplit','softColour']},{name:'Texture & light',keys:['grain','softLight']},{name:'Date',keys:['dateStamp']}];
const CONTROL_COPY: Record<ModuleKey,{name:string;description:string}> = {
filmLook:{name:'Film looks',description:'Your selected looks, with live view.'},framing:{name:'Crop',description:'6x7, 5:4, FP100C, 4:3, 16:9 and square.'},guides:{name:'Guides',description:'Thirds, centre, golden ratio and diagonals.'},dateStamp:{name:'Date stamp',description:'Separate format, appearance and position.'},grain:{name:'Grain',description:'Strength, size, boost and pattern. New controls start at Default.'},softTone:{name:'Tone',description:'Black lift and highlight roll-off.'},softLight:{name:'Light',description:'Bloom and warm halation.'},softSplit:{name:'Split toning',description:'Warm cream, cocoa and amber, with separate strength.'},softColour:{name:'Colour',description:'Saturation, hue and brightness for seven colours.'}};

const REPOSITORY = 'https://github.com/jakeds/grkit';

type Build = { state: 'idle' } | { state: 'building' } | { state: 'ready' } | { state: 'failed' };

export function App() {
  const language: Language = 'en';
  const [cameraId, setCameraId] = useState('gr3x');
  const [selection, setSelection] = useState<ReadonlySet<string>>(() => new Set(DEFAULT_SELECTION));
  const [enabled, setEnabled] = useState<ReadonlySet<ModuleKey>>(() => new Set([...ALL_MODULES].filter((key) => !!key)));
  const [build, setBuild] = useState<Build>({ state: 'idle' });
  const [removeState, setRemoveState] = useState('');
  // A newer build or a changed selection makes the result of an older build stale.
  const buildRun = useRef(0);

  const t = STRINGS[language];
  const camera = CAMERAS.find((item) => item.key === cameraId) ?? CAMERAS[0]!;
  // Offer only the firmware builds rebuilt and checked for this Soft release.
  const support = camera.support && (camera.id === "gr4" ? {...camera.support, targets:["gr4-1.11"], firmware:camera.key === "gr4m" ? "1.00" : "1.11"} : camera.support);
  const available = support?.modules ?? [];
  const plan = planModules(enabled, selection.size, available);
  const effectCount = plan.includeEffects ? selection.size : 0;

  const filename = `Soft-${cameraId === 'gr4m' ? 'GRIV-Monochrome' : camera.id === 'gr3' ? 'GRIII' : camera.id === 'gr4' ? 'GRIV' : 'GRIIIx'}.zip`;

  function chooseCamera(id: string) {
    setCameraId(id);
    setRemoveState('');
    resetBuild();
  }

  async function downloadScriptsOff() {
    if (!support) return;
    const run = buildRun.current;
    setRemoveState('Preparing…');
    try { const blob = await buildScriptsOff(camera.id); if (run !== buildRun.current) return; save(blob, filename.replace('.zip', '-scripts-off.zip')); setRemoveState('Script-disable package downloaded.'); }
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
        ? (await loadCatalog(support.targets[0]!)).effects.filter((effect) => selection.has(effect.id))
        : [];
      const blob = await buildPackage(support.targets, camera.id, effects, plan.modules, language, () => {});
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
        <div className="camera-picker"><label htmlFor="camera">Your camera</label><select id="camera" value={cameraId} onChange={event => chooseCamera(event.target.value)}>{CAMERAS.map(item => <option key={item.key} value={item.key}>{item.name}</option>)}</select><span>Firmware {support?.firmware} only</span></div>
        <p className="note">Choose your looks and controls. Download them to your SD card.</p>
        <nav className="builder-nav" aria-label="Builder sections"><a href="#looks">01 Looks</a><a href="#controls">02 Controls</a><a href="#download">03 Download</a></nav>
      </header>

      <main>
        <section id="looks" className="section" aria-labelledby="effects-label">
          <h2 id="effects-label"><span className="section-number">01</span>Film looks</h2>
          {!available.includes('filmLook') ? (
            <p className="status section-body">Film looks are unavailable on GR IV Monochrome.</p>
          ) : plan.includeEffects ? (
            <ErrorBoundary fallback={<p className="status error section-body">{t.loadError}</p>}>
              <Suspense fallback={<p className="status section-body">{t.loading}</p>}>
                <EffectsSection
                  key={support?.targets[0]}
                  strings={t}
                  target={support!.targets[0]!}
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
            <p className="note">Tick to include a control. Adjust its strength on the camera; new effects start off or neutral. Find them in MENU → Customize → Soft.</p>
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
                      <img className="module-icon" src={siteUrl(`menu-icons/${MODULE_ICONS[module.key] ?? module.key}.png`)} alt="" width={32} height={32} />
                      <span className="module-name">{CONTROL_COPY[module.key]?.name ?? t.moduleNames[module.key]}</span>
                    </span>
                    <span className="module-description">
                      {module.key === 'grain' && camera.id === 'gr4' ? 'Strength and size, using the GR IV grain stage.' : CONTROL_COPY[module.key]?.description ?? t.modulesText[module.key]}
                    </span>
                    {module.key === 'filmLook' &&
                      enabled.has('filmLook') &&
                      selection.size === 0 && (
                        <span className="module-warning">{t.filmLookEmpty}</span>
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
              <li><strong>Start normally.</strong> Turn the camera on without holding MENU. Wait for the loaded indicator. The new loader reduces the delay, but features are still unavailable during startup.</li>
              <li><strong>Open Soft.</strong> Go to MENU → Customize → Soft, or press ADJ and select the extra Soft entry.</li>
            </ol>
            <p>Once enabled, the script setting stays on, including when you switch SD cards. Keep <code>SCRIPT</code> on each card you want to use with Soft. You can remove <code>{support?.modeset}</code> and <code>DEVELOP.MOD</code> after activation.</p>
            <p>If the activation screen does not appear, check that those two files are at the card’s top level and that they came from the package for your selected camera.</p>
          </details>
          <details className="help-panel">
            <summary>Update an existing Soft card</summary>
            <p>Turn the camera off and back up your card. Unzip your kit and copy the new <code>SD_ROOT/SCRIPT</code> onto your existing <code>SCRIPT</code>, merging folders and replacing files with the same names. For this first upgrade from Soft.15, keep <code>SCRIPT/PLUGINS/STATE.TXT</code> until Soft has started successfully. It converts your old settings into <code>GRKIT/STATE.TXT</code>. Future updates can replace <code>SCRIPT</code> while keeping the top-level <code>GRKIT</code> folder. Turn the camera on normally. You do not need to enable scripts again.</p>
            <p>Your scripts remain enabled. You only need the activation procedure once.</p>
          </details>
          <details className="help-panel">
            <summary>Using the camera menus</summary>
            <p>Up / Down selects. Right opens. Left goes back. OK applies. Press ADJ to close.</p>
            <p>Most strengths use 5% steps. Grain also has Default, Auto, Small, Medium and Large sizes. Split toning has separate preset and strength controls; date stamps have separate format and appearance controls.</p>
          </details>
          <details className="help-panel">
            <summary>Camera differences and experimental features</summary>
            <p>III and IIIx share a runtime; HDF models use their base camera’s package. GR IV uses a separate runtime. It uses its own native grain stage and supports the crop controls. GR IV Monochrome excludes colour film looks.</p>
            <p>Grain groups Strength, Size, Boost and Pattern. Boost defaults to Default (no gain); Pattern defaults to Moment (the existing seed). Choose Fixed seed only for a repeatable generator pattern. Default size, Default boost and Moment pattern restore the previous grain path.  New crop modes, III-series grain seeding and JPEG effects have software checks but still need physical camera testing. Bloom and halation may increase saving time. This is an SD-card runtime, not a replacement firmware image.</p>
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
          <a href={`${REPOSITORY}/blob/jake/soft16-upstream-menu/effects/NOTICE.md`}>{t.licenses}</a>
          <a href={siteUrl('THIRD_PARTY_LICENSES.txt')}>{t.thirdParty}</a>
          <a href={`${REPOSITORY}/blob/jake/soft16-upstream-menu/resources/RECOVERY${language === 'en' ? '' : `.${language}`}.md`}>
            {t.recovery}
          </a>
          <a href={`${REPOSITORY}/tree/jake/soft16-upstream-menu`}>Source</a>
          <span>Soft / GRKit</span>
        </div>
      </footer>
    </div>
  );
}
