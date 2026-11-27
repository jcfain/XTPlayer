#!/bin/bash
# ---------------------------------------------------------------------------
# bump-version.sh — sync the Flatpak version references to XTEngine_global.h
#
# The app version lives in XTEngine/src/XTEngine_global.h (the single source
# of truth). The Flatpak manifest + .desktop carry their OWN copies of it
# (the .desktop Version=, the manifest's git tags, and the web-bundle URL) -
# these are what Flathub builds from, so they must match the header. This
# script rewrites those copies to match the header so they can't drift.
#
# Run from the XTPlayer repo root, AFTER merging development into main:
#   bash flatpak/bump-version.sh            # apply + show the diff
#   bash flatpak/bump-version.sh --commit   # apply + commit + push
#
# You never type a version number - it is read from the header.
# ---------------------------------------------------------------------------
set -euo pipefail

# --- Locate the header (XTEngine must be a sibling of XTPlayer) -----------
HEADER="../XTEngine/src/XTEngine_global.h"
if [ ! -f "$HEADER" ]; then
  echo "ERROR: $HEADER not found."
  echo "Run this from the XTPlayer repo root, with XTEngine as a sibling directory."
  exit 1
fi
if [ ! -f "flatpak/manifest.json" ]; then
  echo "ERROR: flatpak/manifest.json not found. Run this from the XTPlayer repo root."
  exit 1
fi
command -v jq >/dev/null || { echo "ERROR: jq is required"; exit 1; }

# --- Read the target version from the header -------------------------------
VERSION=$(awk '/#define VERSION_MAJOR/{m=$3} /#define VERSION_MINOR/{n=$3} /#define VERSION_REVISION/{r=$3} /#define VERSION_PHASE/{p=$3} END{printf "%d.%d.%d%s", m, n, r, p}' "$HEADER")
echo "Target version (from XTEngine_global.h): $VERSION"
echo

# --- Nothing to do if already in sync --------------------------------------
CURRENT_DESKTOP=$(grep '^Version=' flatpak/XTPlayer.desktop | cut -d= -f2)
MANIFEST_TAGS=$(jq -r '.modules[] | select(.name=="xtplayer") | .sources[] | select(.type=="git") | .tag' flatpak/manifest.json | sort -u | tr '\n' ' ')
if [ "$CURRENT_DESKTOP" = "$VERSION" ] && [ "$(echo $MANIFEST_TAGS)" = "v${VERSION}" ]; then
  echo "Already at $VERSION - nothing to do."
  exit 0
fi

# --- Update the .desktop file ----------------------------------------------
sed -i.bak "s/^Version=.*/Version=${VERSION}/" flatpak/XTPlayer.desktop
rm -f flatpak/XTPlayer.desktop.bak

# --- Update the manifest (git tags + web-bundle URL) with jq ---------------
# Targets ONLY the app's version references:
#   - the xtp-web module's release-download URL
#   - the xtplayer module's XTPlayer + XTEngine git tags
# Leaves all other module tags (aom, dav1d, ffmpeg, Qt) untouched.
# --indent 4 preserves the manifest's existing 4-space formatting.
jq --indent 4 --arg ver "v${VERSION}" '
  (.modules[] | select(.name == "xtp-web") | .sources[] | select(.url | contains("releases/download"))) |=
    (.url = (.url | gsub("v[0-9]+\\.[0-9]+\\.[0-9]+[a-z]*"; $ver)))
  | (.modules[] | select(.name == "xtplayer") | .sources[] | select(.type == "git")) |=
    (.tag = $ver)
' flatpak/manifest.json > /tmp/manifest.json.$$ && mv /tmp/manifest.json.$$ flatpak/manifest.json

# --- Report ----------------------------------------------------------------
echo "Updated:"
echo "  flatpak/XTPlayer.desktop : $(grep '^Version=' flatpak/XTPlayer.desktop)"
echo "  flatpak/manifest.json    : tags=$(jq -r '.modules[] | select(.name=="xtplayer") | .sources[] | select(.type=="git") | .tag' flatpak/manifest.json | sort -u | tr '\n' ' ') url=$(jq -r '.modules[] | select(.name=="xtp-web") | .sources[].url' flatpak/manifest.json)"
echo

if [ "${1:-}" = "--commit" ]; then
  git add flatpak/XTPlayer.desktop flatpak/manifest.json
  git commit -m "Bump Flatpak version to ${VERSION}"
  git push
  echo "Committed and pushed."
else
  echo "Changes applied but NOT committed."
  echo "Review with 'git diff', then re-run with --commit to commit + push."
  git --no-pager diff -- flatpak/XTPlayer.desktop flatpak/manifest.json
fi
