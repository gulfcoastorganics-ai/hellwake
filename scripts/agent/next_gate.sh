#!/usr/bin/env bash
set -euo pipefail

ROOT="$(git rev-parse --show-toplevel)"
cd "$ROOT"

STATE="docs/execution-state.md"
if [[ ! -f "$STATE" ]]; then
  echo "FAIL: $STATE is missing" >&2
  exit 1
fi

next="$(grep -m1 -E '^- \[ \] ' "$STATE" || true)"
if [[ -z "$next" ]]; then
  echo 'All tracked execution-state gates are marked complete. Run QA/reviewer before declaring the milestone complete.'
  exit 0
fi

echo "NEXT GATE: ${next#- [ ] }"
