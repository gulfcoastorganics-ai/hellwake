#!/usr/bin/env bash
set -euo pipefail

ROOT="$(git rev-parse --show-toplevel)"
cd "$ROOT"

fail=0
run() {
  local name="$1"
  shift
  printf '\n== %s ==\n' "$name"
  if "$@"; then
    printf 'PASS: %s\n' "$name"
  else
    printf 'FAIL: %s\n' "$name" >&2
    fail=1
  fi
}

run "reference integrity" bash scripts/agent/verify_reference.sh
run "shell syntax" bash -c 'for f in scripts/agent/*.sh; do bash -n "$f"; done'

if [[ -d migration/canonical ]]; then
  run "canonical JSON parse" python3 - <<'PY'
import json
from pathlib import Path
files = sorted(Path('migration/canonical').glob('*.json'))
if not files:
    raise SystemExit('migration/canonical exists but contains no JSON files')
for path in files:
    with path.open('r', encoding='utf-8') as fh:
        json.load(fh)
print(f'parsed {len(files)} canonical JSON files')
PY
else
  printf '\nSKIP: canonical JSON parse (migration/canonical not present)\n'
fi

if [[ -f hellwake-ue/Scripts/ValidateHellwakeData.py ]]; then
  run "Hellwake generated-data validator" python3 hellwake-ue/Scripts/ValidateHellwakeData.py
elif [[ -f Scripts/ValidateHellwakeData.py ]]; then
  run "Hellwake generated-data validator" python3 Scripts/ValidateHellwakeData.py
else
  printf '\nSKIP: generated-data validator is not present\n'
fi

if [[ -x scripts/agent/test_core.sh ]]; then
  run "portable gameplay core" bash scripts/agent/test_core.sh
else
  printf '\nSKIP: portable gameplay core test runner is not present yet\n'
fi

if [[ -f Hellwake.uproject ]]; then
  printf '\nINFO: Hellwake.uproject present\n'
else
  printf '\nFAIL: Hellwake.uproject missing\n' >&2
  fail=1
fi

if [[ -n "${UE_ROOT:-}" && -x "${UE_ROOT}/Engine/Build/BatchFiles/Linux/Build.sh" ]]; then
  run "UE Linux HellwakeEditor build" "${UE_ROOT}/Engine/Build/BatchFiles/Linux/Build.sh" HellwakeEditor Linux Development "$ROOT/Hellwake.uproject" -WaitMutex
else
  printf '\nSKIP: UE build (set UE_ROOT to a compatible UE5.4 install)\n'
fi

if [[ -n "${UE_EDITOR_CMD:-}" && -x "${UE_EDITOR_CMD}" ]]; then
  run "UE headless smoke" "$UE_EDITOR_CMD" "$ROOT/Hellwake.uproject" -nullrhi -unattended -nop4 -nosplash -ExecCmds='Quit'
else
  printf '\nSKIP: UE headless smoke (set UE_EDITOR_CMD to UnrealEditor-Cmd/UnrealEditor)\n'
fi

if ! git diff --check; then
  printf '\nFAIL: git diff --check\n' >&2
  fail=1
fi

if [[ "$fail" -ne 0 ]]; then
  echo 'HELLWAKE validation failed.' >&2
  exit 1
fi

echo 'HELLWAKE validation completed: all available gates passed; unavailable gates were skipped explicitly.'
