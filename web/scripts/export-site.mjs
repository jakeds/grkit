// Combine the checked web build with its matching, prebuilt camera data.
import { cpSync, existsSync, mkdirSync, readFileSync, rmSync } from 'node:fs';
import { resolve } from 'node:path';
const root = resolve(import.meta.dirname, '../..');
const output = resolve(root, process.argv[2] ?? 'dist/soft-site');
const web = resolve(root, 'web/dist');
const data = resolve(root, 'dist/soft16-site-data');
const paths = ['runtime/gr3-2.10', 'runtime/gr4-1.11', 'effects/gr3-2.10', 'effects/gr4-1.11', 'guides/gr3', 'guides/gr3x', 'guides/gr4', 'modules'];
if (!output.startsWith(resolve(root, 'dist') + '/') || output.startsWith(data)) throw new Error('Output must be a separate directory inside dist/.');
for (const path of paths) {
  if (!existsSync(resolve(data, path))) throw new Error(`Missing matching site data: ${path}`);
}
if (!existsSync(resolve(web, 'index.html'))) throw new Error('Build the web page first.');
const build = JSON.parse(readFileSync(resolve(data, 'runtime/gr3-2.10/build.json'), 'utf8'));
if (build.target !== 'gr3-2.10') throw new Error('Unexpected camera runtime target.');
rmSync(output, { recursive: true, force: true });
mkdirSync(output, { recursive: true });
cpSync(web, output, { recursive: true });
for (const path of paths) {
  cpSync(resolve(data, path), resolve(output, path), { recursive: true });
}
console.log(`Static Soft builder: ${output}`);
