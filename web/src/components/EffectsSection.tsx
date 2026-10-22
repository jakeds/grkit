import { use } from 'react';
import { loadCatalog, MAX_EFFECTS, type Effect } from '../catalog';
import type { Strings } from '../i18n';
import { DEFAULT_SELECTION, EVERYDAY_LOOKS } from '../curated';
import { EffectIcon } from './EffectIcon';

interface Props {
  strings: Strings;
  target: string;
  selection: ReadonlySet<string>;
  onSelectionChange: (selection: ReadonlySet<string>) => void;
}

export function EffectsSection({ strings: t, target, selection, onSelectionChange }: Props) {
  const catalog = use(loadCatalog(target));
  const effects = [
    ...DEFAULT_SELECTION.flatMap(id => catalog.effects.filter(effect => effect.id === id)),
    ...catalog.effects.filter(effect => !DEFAULT_SELECTION.includes(effect.id)),
  ];
  const selected = effects.filter(effect => selection.has(effect.id)).length;

  function toggle(effect: Effect) {
    const next = new Set(selection);
    if (!next.delete(effect.id) && next.size < MAX_EFFECTS) next.add(effect.id);
    onSelectionChange(next);
  }

  return (
    <div className="section-body">
      <p className="note">Six everyday looks to start. Tick the ones you want on your camera.</p>
      <div className="toolbar">
        <p><span className="count">{selected}</span> of {effects.length} looks included</p>
        <div>
          <button type="button" className="button" onClick={() => onSelectionChange(new Set(DEFAULT_SELECTION))}>Everyday six</button>
          <button type="button" className="button" onClick={() => onSelectionChange(new Set(effects.slice(0, MAX_EFFECTS).map(effect => effect.id)))}>{t.selectAll}</button>
          <button type="button" className="button" onClick={() => onSelectionChange(new Set())}>{t.clear}</button>
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
            </div>
          </article>
        ))}</div>;
        return other ? <details className="more-looks" key="other"><summary>More looks <span>{effects.length - DEFAULT_SELECTION.length} to explore · {effects.filter(effect => !DEFAULT_SELECTION.includes(effect.id) && selection.has(effect.id)).length} included</span></summary>{list}</details> : <div key="everyday">{list}</div>;
      })}
    </div>
  );
}
