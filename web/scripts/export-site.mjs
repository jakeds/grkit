// Combine the checked web build with its matching, prebuilt camera data.
import { cpSync, existsSync, mkdirSync, readFileSync } from 'node:fs';
import { resolve } from 'node:path';
const root = resolve(import.meta.dirname, '../..');
const output = resolve(root, process.argv[2] ?? 'dist/soft-site');
const web = resolve(root, 'web/dist');
const data = resolve(root, 'dist/site-data');
for (const path of ['runtime/gr3-2.10', 'effects/gr3-2.10', 'guides/gr3x']) {
  if (!existsSync(resolve(data, path))) throw new Error(`Missing matching site data: ${path}`);
}
if (!existsSync(resolve(web, 'index.html'))) throw new Error('Build the web page first.');
const build = JSON.parse(readFileSync(resolve(data, 'runtime/gr3-2.10/build.json'), 'utf8'));
if (build.target !== 'gr3-2.10') throw new Error('Unexpected camera runtime target.');
mkdirSync(output, { recursive: true });
cpSync(web, output, { recursive: true });
for (const path of ['runtime/gr3-2.10', 'effects/gr3-2.10', 'guides/gr3x']) {
  cpSync(resolve(data, path), resolve(output, path), { recursive: true });
}
console.log(`Static Soft builder: ${output}`);
