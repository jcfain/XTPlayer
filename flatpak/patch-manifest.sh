#!/bin/bash
# ---------------------------------------------------------------------------
# Patch flatpak/manifest.json for a CI dev build:
#   1. xtp-web module:  replace the release-asset archive with the locally
#      built web bundle (url -> local path, sha256 -> computed).
#   2. xtplayer module: replace the git sources with the local checkouts
#      (workspace root = XTPlayer, sibling XTEngine/ - the layout the .pro
#      files expect).
#
# The committed manifest keeps the git / release-asset sources so it also
# works for tag-based (release) builds and local reproducible builds.
#
# Usage:  bash flatpak/patch-manifest.sh <manifest.json> <web-bundle.tar.xz> <out.json>
#         (run from the XTPlayer workspace root)
# ---------------------------------------------------------------------------
set -e

MANIFEST="${1:?usage: patch-manifest.sh <manifest> <web-bundle.tar.xz> <out.json>}"
WEB_BUNDLE="${2:?missing web bundle tarball}"
OUT="${3:?missing output manifest path}"

command -v jq >/dev/null || { echo "ERROR: jq is required"; exit 1; }
[ -f "$WEB_BUNDLE" ] || { echo "ERROR: web bundle not found: $WEB_BUNDLE"; exit 1; }

WEB_SHA=$(sha256sum "$WEB_BUNDLE" | cut -d' ' -f1)
WEB_ABS=$(readlink -f "$WEB_BUNDLE")

jq --arg url "$WEB_ABS" --arg sha "$WEB_SHA" '
    (.modules[] | select(.name == "xtp-web") | .sources[0]) |=
        (.url = $url | .sha256 = $sha)
    | (.modules[] | select(.name == "xtplayer") | .sources[]) |=
        (if .type == "git" and (.url | endswith("XTPlayer")) then
            {type: "dir", path: ".", dest: "XTPlayer"}
         elif .type == "git" and (.url | endswith("XTEngine")) then
            {type: "dir", path: "XTEngine", dest: "XTEngine"}
         else . end)
' "$MANIFEST" > "$OUT"

echo "==> Patched manifest written to $OUT (web sha256: $WEB_SHA)"
