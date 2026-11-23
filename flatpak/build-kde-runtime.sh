#!/bin/bash
# ---------------------------------------------------------------------------
# Build org.kde.Platform 6.12-beta + org.kde.Sdk (the runtime the XTPlayer
# Flatpak manifest depends on) from KDE's manifests, and package the result
# as a flatpak repo tarball.
#
# org.kde.Platform 6.12 is not published on Flathub yet (KDE's qt6.12 branch
# is "6.12-beta"), so it is built once per arch and hosted in a personal
# flatpak repo (GitHub release asset). The freedesktop 26.08 base + GL /
# GStreamer / VAAPI / codecs extensions are pulled from Flathub; only the
# KDE layer (Qt 6.12, qtmultimedia, qmake, ...) builds from source.
#
# Usage:   flatpak/build-kde-runtime.sh [ARCH] [WORKDIR]
#   ARCH     x86_64 | aarch64 (default: flatpak --default-arch)
#   WORKDIR  build dir (default: ./kde-runtime-build). Caching this dir
#            makes re-runs incremental (flatpak-builder reuses module builds
#            and make skips regenerating the manifest).
#
# Requires: flatpak, flatpak-builder, git, make
# Output:   $WORKDIR/xtp-kde-runtime-6.12-beta-<ARCH>.tar.xz
# ---------------------------------------------------------------------------
set -e

ARCH="${1:-$(flatpak --default-arch)}"
WORKDIR="${2:-$(pwd)/kde-runtime-build}"
REPO_URL="https://invent.kde.org/packaging/flatpak-kde-runtime.git"
BRANCH="qt6.12"
FLATHUB_URL="https://dl.flathub.org/repo/flathub.flatpakrepo"

for tool in flatpak flatpak-builder git make tar; do
    command -v "$tool" >/dev/null || { echo "ERROR: $tool not found"; exit 1; }
done

mkdir -p "$WORKDIR"
cd "$WORKDIR"

if [ ! -d kde-runtime/.git ]; then
    git clone --depth 1 --branch "$BRANCH" "$REPO_URL" kde-runtime
fi

# Flathub remote for --install-deps-from=flathub. Always use the USER
# installation: it is writable without root (CI runners and local dev
# alike), and flatpak-builder below is passed --user so it installs deps
# into the SAME installation the remote lives in. Mixing the two is what
# produced "No remote refs found for 'flathub'" on CI: the remote was
# registered in one installation while flatpak-builder targeted the other.
flatpak --user remote-add --if-not-exists flathub "$FLATHUB_URL"

cd kde-runtime
# Generate org.kde.Sdk.json from org.kde.Sdk.json.in (appends
# inherit-sdk-extensions, then applies kde-sdk.patch to the GENERATED file).
# Re-runs: make skips the rule if the json is up to date, so the patch is
# never applied twice.
make org.kde.Sdk.json ARCH="$ARCH"

# Build org.kde.Platform (id-platform) + org.kde.Sdk into ./repo.
# No --force-clean: with a cached WORKDIR, unchanged modules are reused.
flatpak-builder \
    --user \
    --install-deps-from=flathub \
    --arch="$ARCH" \
    --ccache \
    --repo=repo \
    --subject="org.kde.Platform 6.12-beta ($(git rev-parse --short HEAD)) $(date -Is)" \
    build org.kde.Sdk.json

flatpak build-update-repo repo

tar -cJf "xtp-kde-runtime-6.12-beta-${ARCH}.tar.xz" repo
echo "==> Done: $(pwd)/xtp-kde-runtime-6.12-beta-${ARCH}.tar.xz"
