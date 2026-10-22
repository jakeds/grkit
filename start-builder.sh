#!/bin/sh
# Run the already-built local Soft / GRKit builder.
set -eu
cd "$(dirname "$0")/web"
if [ ! -f src/wasm/grkit_wasm.js ] || [ ! -f ../dist/site-data/runtime/gr3-2.10/build.json ]; then
  echo 'The local browser packager and camera runtime must be built first. See web/README.md.' >&2
  exit 1
fi
exec pnpm dev --host 127.0.0.1 --port 4173 --strictPort
