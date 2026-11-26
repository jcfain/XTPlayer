#!/bin/bash
# ---------------------------------------------------------------------------
# Patch flatpak/manifest.json for a CI dev build:
#   1. runtime/sdk: substitute the $ARCH placeholder with the target arch.
#   2. xtp-web module:  replace the release-asset archive with the locally
#      built web bundle (url -> local path, sha256 -> computed).
#   3. xtplayer module: replace the git sources with the local checkouts
#      (workspace root = XTPlayer, sibling XTEngine/ - the layout the .pro
#      files expect).
#   4. Per-arch config-opts: modules may carry a config-opts OBJECT keyed
#      by arch (e.g. ffmpeg-av1) - flatpak-builder does NOT support that
#      (config-opts must be a flat string array; an object is dropped with
#      a deserialization warning), so flatten it to the target arch here.
#   5. patch sources: paths are relative to the manifest's location. The
#      committed manifest lives in flatpak/, but this script writes the
#      patched manifest to the workspace root, so prefix flatpak/ to make
#      the paths resolve from there.
#
# The committed manifest keeps the git / release-asset sources so it also
# works for tag-based (release) builds and local reproducible builds.
#
# Usage:  bash flatpak/patch-manifest.sh <manifest.json> <web-bundle.tar.xz> <out.json> <arch>
#         (run from the XTPlayer workspace root)
# ---------------------------------------------------------------------------
set -e

MANIFEST="${1:?usage: patch-manifest.sh <manifest> <web-bundle.tar.xz> <out.json> <arch>}"
WEB_BUNDLE="${2:?missing web bundle tarball}"
OUT="${3:?missing output manifest path}"
ARCH="${4:?missing target arch (e.g. x86_64 / aarch64)}"

command -v jq >/dev/null || { echo "ERROR: jq is required"; exit 1; }
[ -f "$WEB_BUNDLE" ] || { echo "ERROR: web bundle not found: $WEB_BUNDLE"; exit 1; }

WEB_SHA=$(sha256sum "$WEB_BUNDLE" | cut -d' ' -f1)
WEB_ABS=$(readlink -f "$WEB_BUNDLE")

jq --arg url "$WEB_ABS" --arg sha "$WEB_SHA" --arg arch "$ARCH" '
    .runtime = "org.freedesktop.Platform/\($arch)/26.08"
    | .sdk = "org.freedesktop.Sdk/\($arch)/26.08"
    | (.modules[] | select(.name == "xtp-web") | .sources[0]) |=
        {type: "archive", path: $url, sha256: $sha, dest: "web"}
    | (.modules[] | select(.name == "xtplayer") | .sources[]) |=
        (if .type == "git" and (.url | endswith("XTPlayer")) then
            {type: "dir", path: ".", dest: "XTPlayer"}
         elif .type == "git" and (.url | endswith("XTEngine")) then
            {type: "dir", path: "XTEngine", dest: "XTEngine"}
         else . end)
    | (.modules[] | select(."config-opts" | type == "object") | ."config-opts") |=
        (.[$arch] // [])
    | (.modules[].sources[] | select(.type == "patch") | .path) |=
        "flatpak/" + .
' "$MANIFEST" > "$OUT"

echo "==> Patched manifest written to $OUT (arch: $ARCH, web sha256: $WEB_SHA)"
