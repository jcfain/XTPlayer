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
# The script lives in <repo>/flatpak/ - the qtgamepadlegacy module added
# below references a patch that is staged next to the generated manifest
# (patch paths are relative to the manifest location).
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

for tool in flatpak flatpak-builder git make tar; do
    command -v "$tool" >/dev/null || { echo "ERROR: $tool not found"; exit 1; }
done

mkdir -p "$WORKDIR"
# Resolve to an absolute path: the script cd's into $WORKDIR/kde-runtime
# later, and the final tar must land back in $WORKDIR (where the workflow
# uploads it from) - a relative WORKDIR would break after the cd.
WORKDIR="$(cd "$WORKDIR" && pwd)"
cd "$WORKDIR"

if [ ! -d kde-runtime/.git ]; then
    # A cache restore may have left a partial kde-runtime/ behind (just the
    # cached .flatpak-builder state) - `git clone` refuses a non-empty
    # destination, so init+fetch into the existing dir instead. Untracked
    # files (the cached .flatpak-builder) are left alone.
    mkdir -p kde-runtime
    git -C kde-runtime init --quiet
    git -C kde-runtime fetch --depth 1 "$REPO_URL" "refs/heads/$BRANCH:refs/heads/$BRANCH"
    git -C kde-runtime checkout --quiet "$BRANCH"
fi

# Flathub remote for --install-deps-from=flathub. Always use the USER
# installation: it is writable without root (CI runners and local dev
# alike), and flatpak-builder below is passed --user so it installs deps
# into the SAME installation the remote lives in. Mixing the two is what
# produced "No remote refs found for 'flathub'" on CI: the remote was
# registered in one installation while flatpak-builder targeted the other.
flatpak --user remote-add --if-not-exists flathub "$FLATHUB_URL"

cd kde-runtime
# Stage the qtgamepadlegacy patch next to the manifest (the module added
# below references it by bare filename; patch paths are relative to the
# manifest). cp is idempotent on re-runs.
cp "$SCRIPT_DIR/patches/qtgamepadlegacy-no-qml.patch" qtgamepadlegacy-no-qml.patch

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

# os-release's Makefile calls appstreamcli, which the freedesktop SDK does
# NOT provide (KDE's own CI gets it from their BuildStream toolchain, so the
# line was never exercised under flatpak-builder). Make the compose steps
# no-ops when appstreamcli is absent - the composed appstream metadata is
# not needed to run apps. Patch the file in the checkout (NOT via a manifest
# script source: with a cached workdir flatpak-builder may reuse the module's
# source dir and never re-run script sources). The grep guard makes it
# idempotent on re-runs.
if grep -q $'^\tappstreamcli ' os-release-Makefile; then
    sed -i 's#^\tappstreamcli #\tcommand -v appstreamcli >/dev/null \&\& appstreamcli #; s#^\tcommand -v appstreamcli .*#& || true#' os-release-Makefile
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
#   qtbase           core gui widgets network concurrent compress printsupport
#   qtserialport     QT += serialport
#   qtspeech         QT += texttospeech
#   qtwebsockets     QT += websockets
#   qtconnectivity   QT += bluetooth (the Bluetooth module lives in the
#                    qtconnectivity repo together with NFC - there is no
#                    standalone qtbluetooth repo/tarball; without BlueZ
#                    headers it builds the dummy backend, the app talks to
#                    BlueZ over D-Bus at runtime)
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
# (gamepad: the Qt6 path uses the third-party 'gamepadlegacy' module, which
#  is NOT in KDE's manifest and is added below. It MUST be in the runtime,
#  not the app manifest: XTEngine builds with the SDK's qmake (prefix /usr),
#  which resolves QT += modules from the SDK's mkspecs/modules - a module
#  installed to /app is invisible to it.)
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
    "qtserialport", "qtspeech", "qtwebsockets", "qtconnectivity",
}
def prune(modules):
    missing = KEEP - {m.get("name") for m in modules if isinstance(m, dict)}
    if missing:
        sys.exit(f"ERROR: modules missing from KDE manifest: {sorted(missing)}")
    # String entries are sub-manifest REFs (qt6-webview, pyside) - drop them;
    # the keep list above is explicit and complete.
    return [m for m in modules
            if isinstance(m, dict) and m.get("name") in KEEP]
manifest["modules"] = prune(manifest.get("modules", []))
# qtgamepadlegacy is NOT in KDE's manifest - it is a third-party Qt module
# (the 'gamepadlegacy' module XTEngine links on the Qt6 path). Build it from
# git with the no-qml patch (the KDE SDK has no Qt Quick, so the QML imports
# subdirectory must be skipped - the app only uses the C++ API + the evdev
# plugin). Its CMake build installs the qmake .pri spec into the SDK's
# mkspecs/modules, which is what XTEngine's qmake build resolves against.
if not any(isinstance(m, dict) and m.get("name") == "qtgamepadlegacy"
           for m in manifest["modules"]):
    manifest["modules"].append({
        "name": "qtgamepadlegacy",
        "buildsystem": "cmake-ninja",
        "config-opts": [
            "-DCMAKE_BUILD_TYPE=Release",
            "-DQT_GENERATE_SBOM=OFF"
        ],
        "sources": [
            {
                "type": "git",
                "url": "https://github.com/pumphaus/qtgamepadlegacy",
                "commit": "434fe59d70f42fa0f745af05921ba64169fce90b"
            },
            {
                "type": "patch",
                "path": "qtgamepadlegacy-no-qml.patch"
            }
        ]
    })
with open(path, "w") as f:
    json.dump(manifest, f, indent=4)
    f.write("\n")
PYEOF

# Build org.kde.Platform (id-platform) + org.kde.Sdk into ./repo.
# No --force-clean: with a cached WORKDIR, unchanged modules are reused.
# --disable-rofiles-fuse: current flatpak-builder mounts the appdir
# read-only (rofiles-fuse) during the "Cleaning up" phase to protect
# hardlinked file trees. KDE's manifest runs /usr/libexec/freedesktop-post.sh
# as a top-level cleanup-command, which chmods files (e.g.
# /etc/pki/ca-trust/extracted/...) - that fails with "Read-only file
# system" under the rofiles overlay and aborts the build. Disabling
# rofiles restores the classic (writable appdir) cleanup behaviour.
# The flag is only passed to flatpak-builder versions that have it
# (older ones reject unknown options).
ROFILES_ARGS=()
if flatpak-builder --help 2>&1 | grep -q 'disable-rofiles-fuse'; then
    ROFILES_ARGS=(--disable-rofiles-fuse)
fi
flatpak-builder \
    --user \
    --install-deps-from=flathub \
    --arch="$ARCH" \
    --ccache \
    "${ROFILES_ARGS[@]}" \
    --repo=repo \
    --subject="org.kde.Platform 6.12-beta ($(git rev-parse --short HEAD)) $(date -Is)" \
    build org.kde.Sdk.json

flatpak build-update-repo repo

# NOTE: we are inside $WORKDIR/kde-runtime here - tar into $WORKDIR
# explicitly, that is where the workflow's upload-artifact looks.
tar -cJf "$WORKDIR/xtp-kde-runtime-6.12-beta-${ARCH}.tar.xz" repo
echo "==> Done: $WORKDIR/xtp-kde-runtime-6.12-beta-${ARCH}.tar.xz"
