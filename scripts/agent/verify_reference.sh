#!/usr/bin/env bash
set -euo pipefail

ROOT="$(git rev-parse --show-toplevel)"
cd "$ROOT"

MAIN_BASELINE="4361ad788052e42e9eaf42c322be332361480208"
FROZEN_TAG="hellwake-web-vertical-slice-reference"
EXPECTED_FROZEN_SHA="4c51cd2a"

printf 'HELLWAKE reference verification\n'
printf 'repo: %s\n' "$ROOT"
printf 'head: %s\n' "$(git rev-parse HEAD)"
printf 'branch: %s\n' "$(git branch --show-current || true)"

if ! git cat-file -e "${MAIN_BASELINE}^{commit}" 2>/dev/null; then
  echo "FAIL: required UE baseline commit ${MAIN_BASELINE} is missing" >&2
  exit 1
fi
printf 'PASS: UE baseline commit is present\n'

if git rev-parse -q --verify "refs/tags/${FROZEN_TAG}" >/dev/null; then
  tag_sha="$(git rev-list -n 1 "$FROZEN_TAG")"
  if [[ "$tag_sha" != "$EXPECTED_FROZEN_SHA"* ]]; then
    echo "FAIL: ${FROZEN_TAG} points to ${tag_sha}, expected ${EXPECTED_FROZEN_SHA}..." >&2
    exit 1
  fi
  printf 'PASS: %s -> %s\n' "$FROZEN_TAG" "$tag_sha"
else
  printf 'SKIP: %s is not present in this checkout; do not fabricate it\n' "$FROZEN_TAG"
fi

tracked_bad="$(git ls-files 'Binaries/**' 'DerivedDataCache/**' 'Intermediate/**' 'Saved/**' '.vs/**' || true)"
if [[ -n "$tracked_bad" ]]; then
  echo "FAIL: Unreal generated artifacts are tracked:" >&2
  echo "$tracked_bad" >&2
  exit 1
fi
printf 'PASS: Unreal generated-artifact directories are not tracked\n'

if [[ -d migration/canonical ]]; then
  count="$(find migration/canonical -maxdepth 1 -type f -name '*.json' | wc -l | tr -d ' ')"
  printf 'INFO: migration/canonical present with %s JSON files\n' "$count"
else
  printf 'SKIP: migration/canonical is not present in this checkout\n'
fi
