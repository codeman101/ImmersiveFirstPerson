#!/bin/bash
# Point a Homebrew dependency at @loader_path, then re-sign ad hoc.
# This plugin is meant to link nothing from Homebrew. A library that is not linked is not an error.
# Usage: fix-install-names.sh <path to ImmersiveFirstPerson.dylib>
set -eu
DYLIB="$1"
found=0
while IFS= read -r old; do
  [[ -z "$old" ]] && continue
  found=1
  case "$old" in
    @loader_path/*) echo "fix-install-names: already $old" ;;
    *)
      new="@loader_path/$(basename "$old")"
      install_name_tool -change "$old" "$new" "$DYLIB"
      echo "fix-install-names: $old -> $new"
      ;;
  esac
done < <(otool -L "$DYLIB" | awk 'NR > 1 { print $1 }' | grep '/opt/homebrew/' || true)
if [[ "$found" -eq 0 ]]; then
  echo "fix-install-names: no Homebrew dependency"
else
  if ! otool -l "$DYLIB" | grep -A2 LC_RPATH | grep -q '@loader_path'; then
    install_name_tool -add_rpath @loader_path "$DYLIB"
    echo "fix-install-names: rpath @loader_path added"
  fi
fi
codesign -f -s - "$DYLIB"
