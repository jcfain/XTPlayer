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

# Reverse-apply kde-sdk.patch (qt6.12-only): it rewrites licence-extractor's
# install paths to %{install-root}/... and $INSTALL_ROOT/..., but current
# flatpak/flatpak-builder substitute NEITHER (verified in both sources) -
# the script lands in a literal '%{install-root}' dir and later modules'
# shell sources die with "install-licenses.sh: command not found". The
# unpatched /usr/bin paths are correct for current flatpak (the appdir is
# the build namespace root) - exactly what KDE's main branch uses for its
# working 6.10/6.11 builds. Idempotent: only reverse-applies if present
# (on cached re-runs make skips the rule, so the patch is not re-applied).
if patch -R -p1 --dry-run < kde-sdk.patch >/dev/null 2>&1; then
    patch -R -p1 < kde-sdk.patch
fi

# The freedesktop 26.08 SDK no longer ships elfutils (eu-strip) - it provides
# a custom 'freedesktop-sdk-stripper' instead. flatpak-builder's DEFAULT
# debuginfo post-processing runs eu-strip on every module (it is NOT gated
# by --strip finish-args, by build-options only), so the build dies with
# "Failed to execute child process eu-strip". Switch to plain binutils strip
# (present in the SDK toolchain) via the top-level build-options: this also
# keeps the runtime slim, since modules build RelWithDebInfo.
#
# Prune the module list: KDE's manifest builds the FULL org.kde.Platform
# (QtWebEngine/Chromium, the whole QML/Quick stack, ~70 KDE Frameworks,
# pyside, ...) - XTPlayer is a QtWidgets app and needs only a small slice.
# Keeping just the modules below cuts the build time dramatically (qtwebengine
# alone is the single longest module). The keep list is what the app links
# (QT += lines in src/*.pro, Qt6 path) plus build-time dependencies:
#   qtbase           core gui widgets network concurrent serialport
#                    texttospeech websockets bluetooth compress printsupport
#   qtshadertools    required by qtmultimedia (its dependencies.yaml)
#   qtmultimedia     multimedia + multimediawidgets; the SDK provides both
#                    FFmpeg and GStreamer, so BOTH backend plugins are built
#                    (the app picks ffmpeg via QT_MULTIMEDIA_PREFERRED_PLUGINS)
#   qtwayland        Wayland platform plugin (Steam Frame runs gamescope)
#   qthttpserver     QT += httpserver (serves the web UI)
#   qtsvg            SVG image plugin
#   qtimageformats   extra image format plugins
#   qttools          provides lrelease, needed to BUILD qttranslations
#   qttranslations   Qt's own .qm translations - main.cpp loads
#                    qtbase_<lang>.qm (QFileDialog, QMessageBox buttons)
#   licence-extractor / os-release: infra, trivial
# (gamepad: the Qt6 path uses the custom 'gamepadlegacy' module, which is
#  built in the app manifest; 'QT += gamepad' only appears in the QT5 blocks.)
# Idempotent on re-runs (filtering an already-filtered list is a no-op).
python3 - org.kde.Sdk.json <<'PYEOF'
import json, sys
path = sys.argv[1]
with open(path) as f:
    manifest = json.load(f)
manifest.setdefault("build-options", {})["strip"] = True
KEEP = {
    "licence-extractor", "os-release",
    "qtbase", "qtshadertools", "qtmultimedia", "qtwayland",
    "qthttpserver", "qtsvg", "qtimageformats", "qttools", "qttranslations",
}
def prune(modules):
    missing = KEEP - {m.get("name") for m in modules if isinstance(m, dict)}
    if missing:
        sys.exit(f"ERROR: modules missing from KDE manifest: {sorted(missing)}")
    # String entries are sub-manifest REFs (qt6-webview, pyside) - drop them;
    # the keep list above is explicit and complete.
    kept = [m for m in modules
            if isinstance(m, dict) and m.get("name") in KEEP]
    # os-release's Makefile calls appstreamcli, which the freedesktop SDK
    # does NOT provide (KDE's own CI gets it from their BuildStream
    # toolchain, so the line was never exercised under flatpak-builder).
    # Make the compose steps no-ops when appstreamcli is absent - the
    # composed appstream metadata is not needed to run apps.
    for m in kept:
        if m.get("name") == "os-release":
            m["sources"].append({
                "type": "script",
                "commands": [
                    r"sed -i 's#^\tappstreamcli #\tcommand -v appstreamcli >/dev/null \&\& appstreamcli #; s#^\tcommand -v appstreamcli .*#& || true#' Makefile",
                ],
            })
    return kept
manifest["modules"] = prune(manifest.get("modules", []))
with open(path, "w") as f:
    json.dump(manifest, f, indent=4)
    f.write("\n")
PYEOF

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
