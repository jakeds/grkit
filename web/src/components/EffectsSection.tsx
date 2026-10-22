import {
  Suspense,
  use,
  useCallback,
  useRef,
  useState,
  useTransition,
  type ChangeEvent,
} from 'react';
import { Handle, HandleRoot, Item, Provider, Root } from 'react-compare-slider/components';
import { useReactCompareSlider } from 'react-compare-slider/hooks';
import { PREVIEW_TARGET } from '../cameras';
import { effectUrl, loadCatalog, MAX_EFFECTS, type Effect } from '../catalog';
import type { Language, Strings } from '../i18n';
import { forget, preview, type PreviewEffect, type Source } from '../preview/render';
import { DEFAULT_SAMPLE } from '../preview/samples';
import { PhotoLoader } from '../preview/raw';
import { STOCKS } from '../stocks';
import { DEFAULT_SELECTION, EVERYDAY_LOOKS } from '../curated';
import { EffectIcon } from './EffectIcon';
import { ErrorBoundary } from './ErrorBoundary';

const DEFAULT_PREVIEW = 'portra-400-endura-v1';

/** The film number, then the print film of a cine look: "5219 · 2393 print". */
function stockText(stock: (typeof STOCKS)[string] & {}, t: Strings): string {
  return stock.print ? `${stock.film} · ${t.printFilm(stock.print)}` : stock.film;
}

interface Props {
  strings: Strings;
  language: Language;
  selection: ReadonlySet<string>;
  onSelectionChange: (selection: ReadonlySet<string>) => void;
}

export function EffectsSection({ strings: t, selection, onSelectionChange }: Props) {
  const catalog = use(loadCatalog(PREVIEW_TARGET));
  const effects = [...DEFAULT_SELECTION.flatMap(id => catalog.effects.filter(effect => effect.id === id)), ...catalog.effects.filter(effect => !DEFAULT_SELECTION.includes(effect.id))];
  const [previewId, setPreviewId] = useState(DEFAULT_PREVIEW);
  const [source, setSource] = useState<Source>(DEFAULT_SAMPLE);
  const photo = useRef<Source | null>(null);
  const [loader] = useState(() => new PhotoLoader());
  const [decoding, setDecoding] = useState(false);
  const ownPhoto = useCallback(
    () => () => {
      loader.cancel();
      if (photo.current) forget(photo.current);
      photo.current = null;
    },
    [loader],
  );
  const [photoError, setPhotoError] = useState(false);
  const [previewAttempt, setPreviewAttempt] = useState(0);
  const [isPending, startTransition] = useTransition();
  const photoCount = useRef(0);

  const shown = effects.find((effect) => effect.id === previewId) ?? effects[0];
  const name = shown?.name ?? '';

  function toggle(effect: Effect) {
    const next = new Set(selection);
    if (next.has(effect.id)) {
      next.delete(effect.id);
    } else if (next.size < MAX_EFFECTS) {
      next.add(effect.id);
    }
    onSelectionChange(next);
  }

  function showEffect(effect: Effect) {
    startTransition(() => setPreviewId(effect.id));
  }

  function showSample() {
    loader.cancel();
    setDecoding(false);
    setPhotoError(false);
    startTransition(() => setSource(DEFAULT_SAMPLE));
    if (photo.current) forget(photo.current);
    photo.current = null;
  }

  async function openPhoto(event: ChangeEvent<HTMLInputElement>) {
    const file = event.target.files?.[0];
    event.target.value = '';
    if (!file) return;
    setPhotoError(false);
    setDecoding(true);
    try {
      const image = await loader.open(file);
      // A replacement, sample selection, or unmount cancelled this request.
      if (!image) return;
      const next: Source = { key: `photo-${++photoCount.current}`, load: async () => image };
      const previous = photo.current;
      photo.current = next;
      startTransition(() => setSource(next));
      if (previous) forget(previous);
      setDecoding(false);
    } catch {
      setPhotoError(true);
      setDecoding(false);
    }
  }

  const sourceLabel = source === DEFAULT_SAMPLE ? t.samplePhoto : t.yourPhoto;

  function retryPreview() {
    forget(source);
    setPreviewAttempt((attempt) => attempt + 1);
  }

  return (
    <div className="section-body" ref={ownPhoto}>
      <p className="note">Six everyday looks to start. Preview a look, then tick it to keep it.</p>
      <div className="preview-row">
        <div className="viewer">
          {shown && (
            <ErrorBoundary
              resetKey={`${source.key}\n${shown.id}\n${previewAttempt}`}
              fallback={
                <div className="frame">
                  <div className="frame-placeholder">
                    <p>{t.previewError}</p>
                    <button type="button" className="button small" onClick={retryPreview}>
                      {t.retryPreview}
                    </button>
                  </div>
                </div>
              }
            >
              <Suspense fallback={<Placeholder text={sourceLabel} busy />}>
                <PreviewFrame
                  source={source}
                  effect={{
                    id: shown.id,
                    table: effectUrl(catalog.target, shown.preview),
                    size: catalog.lutSize,
                    baseline: effectUrl(catalog.target, catalog.baseline),
                  }}
                  compareLabel={t.compare}
                  busy={isPending || decoding}
                  originalLabel={t.original}
                  originalAlt={sourceLabel}
                  effectName={name}
                />
              </Suspense>
            </ErrorBoundary>
          )}
        </div>

        <div className="preview-side">
          <div className="field">
            <span className="field-label">{t.previewing}</span>
            <span className="previewing">{name}</span>
          </div>
          <label htmlFor="photo-input" className="upload">
            <svg
              width="18"
              height="18"
              viewBox="0 0 24 24"
              fill="none"
              stroke="currentColor"
              strokeWidth="2"
              strokeLinecap="round"
              strokeLinejoin="round"
              aria-hidden="true"
            >
              <path d="M12 16V4" />
              <path d="M7 9l5-5 5 5" />
              <path d="M4 20h16" />
            </svg>
            <span>{t.upload}</span>
          </label>
          <input
            id="photo-input"
            className="visually-hidden"
            type="file"
            accept=".dng,image/x-adobe-dng"
            onChange={openPhoto}
          />
          {decoding && (
            <p role="status" className="status">
              {t.decodingPhoto}
            </p>
          )}
          {photoError && (
            <p role="alert" className="status error">
              {t.photoError}
            </p>
          )}
          {source !== DEFAULT_SAMPLE && (
            <button type="button" className="button small" onClick={showSample}>
              {t.useSample}
            </button>
          )}
          <p className="note">DNG only. Photos stay in your browser.</p>

          {source === DEFAULT_SAMPLE && (
            <p className="note">
              <a
                href="https://www.flickr.com/photos/cafetricot/51803682755/"
                target="_blank"
                rel="noreferrer"
              >
                {t.sampleCredit}
              </a>
            </p>
          )}
        </div>
      </div>

      <div className="toolbar">
        <p>
          <span className="count">{selection.size}</span> of {effects.length} looks included
        </p>
        <div>
          <button type="button" className="button" onClick={() => onSelectionChange(new Set(DEFAULT_SELECTION))}>Everyday six</button>
          <button
            type="button"
            className="button"
            onClick={() =>
              onSelectionChange(new Set(effects.slice(0, MAX_EFFECTS).map((effect) => effect.id)))
            }
          >
            {t.selectAll}
          </button>
          <button type="button" className="button" onClick={() => onSelectionChange(new Set())}>
            {t.clear}
          </button>
        </div>
      </div>

      {[false, true].map(other => {
        const list = <div className="look-list">{effects.filter(effect => DEFAULT_SELECTION.includes(effect.id) !== other).map(effect => (
          <article className={`look-choice ${selection.has(effect.id) ? 'included' : ''}`} key={effect.id}>
            <input id={`effect-${effect.id}`} type="checkbox" checked={selection.has(effect.id)} onChange={() => toggle(effect)} />
            <EffectIcon effect={effect} />
            <div className="look-copy">
              <label htmlFor={`effect-${effect.id}`} className="effect-name">{effect.name}</label>
              <p>{EVERYDAY_LOOKS[effect.id as keyof typeof EVERYDAY_LOOKS] ?? t.looks[effect.id] ?? effect.look}</p>
              {STOCKS[effect.id] && <span className="effect-stock">{stockText(STOCKS[effect.id]!, t)}</span>}
            </div>
            <button type="button" className="button small" aria-pressed={effect.id === shown?.id} aria-label={t.previewOf(effect.name)} onClick={() => showEffect(effect)}>{t.preview}</button>
          </article>
        ))}</div>;
        return other ? <details className="more-looks" key="other"><summary>More looks <span>{effects.length - DEFAULT_SELECTION.length} to explore · {effects.filter(effect => !DEFAULT_SELECTION.includes(effect.id) && selection.has(effect.id)).length} included</span></summary>{list}</details> : <div key="everyday">{list}</div>;
      })}
    </div>
  );
}

interface FrameProps {
  source: Source;
  effect: PreviewEffect;
  compareLabel: string;
  busy: boolean;
  originalLabel: string;
  originalAlt: string;
  effectName: string;
}

/** The baseline development on the left and the effect on the right. */
function PreviewFrame({
  source,
  effect,
  compareLabel,
  busy,
  originalLabel,
  originalAlt,
  effectName,
}: FrameProps) {
  const result = use(preview(source, effect));
  const comparison = useReactCompareSlider({ clip: 'itemTwo' });
  return (
    <div className="frame" aria-busy={busy}>
      <Provider {...comparison}>
        <Root className="image-comparison">
          <Item item="itemOne">
            <img src={result.original} alt={`${originalAlt} — ${originalLabel}`} />
          </Item>
          <Item item="itemTwo" className="comparison-effect">
            <img src={result.processed} alt={`${originalAlt} — ${effectName}`} />
          </Item>
          <HandleRoot aria-label={compareLabel}>
            <Handle buttonStyle={{ width: 44, height: 44 }} />
          </HandleRoot>
        </Root>
      </Provider>
      <span className="tag">{effectName}</span>
    </div>
  );
}

function Placeholder({ text, busy = false }: { text: string; busy?: boolean }) {
  return (
    <div className="frame" aria-busy={busy}>
      <span className="frame-placeholder">{text}</span>
    </div>
  );
}
