#!/usr/bin/env bash

set -uo pipefail

ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
cd "$ROOT" || exit 1

ALLOWLISTED_FILES=(
  "./8-Miscellany/8.3_Policy-Based_Data_Structures.cpp"
)

is_allowlisted() {
  local file=$1
  local allowed
  for allowed in "${ALLOWLISTED_FILES[@]}"; do
    if [[ "$file" == "$allowed" ]]; then
      return 0
    fi
  done
  return 1
}

include_cleaner=$(command -v clang-include-cleaner || true)
if [[ -z "$include_cleaner" && -x /opt/homebrew/opt/llvm/bin/clang-include-cleaner ]]; then
  include_cleaner=/opt/homebrew/opt/llvm/bin/clang-include-cleaner
fi
if [[ -z "$include_cleaner" ]]; then
  echo "clean_includes.sh: clang-include-cleaner is not on PATH" >&2
  exit 1
fi

build_dir=$(mktemp -d)
trap 'rm -rf "$build_dir"' EXIT
printf '%s\n' '-std=c++17' > "$build_dir/compile_flags.txt"

count=0
skipped=0
failed=0
while IFS= read -r -d '' file; do
  if is_allowlisted "$file"; then
    printf 'SKIPPED (allowlisted)  %s\n' "$file"
    skipped=$((skipped + 1))
    continue
  fi
  if report=$("$include_cleaner" \
      -p "$build_dir" \
      --print=changes \
      --disable-insert \
      "$file"); then
    if [[ -n "$report" ]]; then
      printf '\n%s\n%s\n' "$file" "$report"
      count=$((count + 1))
    fi
  else
    printf 'FAILED  %s\n' "$file" >&2
    failed=$((failed + 1))
  fi
done < <(find . -type d -name Book -prune -o -type f -name '*.cpp' -print0)

printf '\n%d files have suggested removals\n' "$count"
printf '%d allowlisted files skipped\n' "$skipped"
if ((failed > 0)); then
  printf '%d files failed analysis\n' "$failed" >&2
  exit 1
fi
