#!/bin/bash
# ---------------------------------------------------------------------------
# Build FFmpeg n9.0.1 with AV1 software decoding (dav1d + libaom) for the
# XTPlayer Qt FFmpeg media backend.
#
# Why: the Qt 6.12 FFmpeg multimedia plugin (libffmpegmediaplugin.so) is
# linked against FFmpeg 9.0.x (libavcodec.so.63). The copy bundled with the
# Qt installation has no AV1 decoders, so AV1 videos play audio-only.
# This script produces a drop-in replacement build with AV1 decoders enabled.
#
# Usage:   helper_scripts/build-ffmpeg-av1.sh [--install-deps] [--clean|--reuse]
#          --install-deps  Install all build dependencies via the system
#                          package manager (apt/dnf/yum/pacman) first.
#          --clean         Delete the existing deploy dir before building.
#          --reuse         Reuse the existing deploy dir (default if present).
#                          (If the deploy dir exists and you're at a terminal,
#                           the script asks R/D; these flags skip the prompt.)
# Env:     CODEC_PREFIX  install prefix (default: $HOME/git/ffmpeg-n9.0.1-av1)
#          FFMPEG_SRC    FFmpeg source dir (default: $HOME/git/ffmpeg)
#          JOBS          parallel make jobs (default: nproc)
#
# CI-safe: never prompts; fails fast with install hints when deps are missing.
# ---------------------------------------------------------------------------
set -e

FFMPEG_TAG=n9.0.1
FFMPEG_SRC="${FFMPEG_SRC:-${HOME}/git/ffmpeg}"
CODEC_PREFIX="${CODEC_PREFIX:-${HOME}/git/ffmpeg-n9.0.1-av1}"
JOBS="${JOBS:-$(nproc)}"

# ---------------------------------------------------------------------------
# Optional one-shot dependency install (--install-deps)
# ---------------------------------------------------------------------------
INSTALL_DEPS=0
CLEAN=0
REUSE=0
for arg in "$@"; do
    case "$arg" in
        --install-deps) INSTALL_DEPS=1 ;;
        --clean)        CLEAN=1 ;;
        --reuse)        REUSE=1 ;;
    esac
done

# Package lists per distribution
APT_DEPS="build-essential nasm pkg-config git \
          libdav1d-dev libaom-dev \
          libva-dev libva-drm-dev libva-x11-dev \
          libvdpau-dev libvulkan-dev"

DNF_DEPS="gcc-c++ make nasm pkgconf-pkg-config git \
          dav1d-devel aom-devel \
          libva-devel \
          libvdpau-devel \
          vulkan-devel"

PACMAN_DEPS="base-devel nasm pkgconf git \
             dav1d aom \
             libva \
             libvdpau \
             vulkan-headers vulkan-loader"

if [ "$INSTALL_DEPS" -eq 1 ]; then
    if command -v apt-get >/dev/null 2>&1; then
        echo "==> Installing dependencies via apt-get"
        sudo apt-get update
        sudo apt-get install -y $APT_DEPS
    elif command -v dnf >/dev/null 2>&1; then
        echo "==> Installing dependencies via dnf"
        sudo dnf install -y $DNF_DEPS
    elif command -v yum >/dev/null 2>&1; then
        echo "==> Installing dependencies via yum"
        sudo yum install -y $DNF_DEPS
    elif command -v pacman >/dev/null 2>&1; then
        echo "==> Installing dependencies via pacman"
        sudo pacman -S --noconfirm $PACMAN_DEPS
    else
        echo "ERROR: no supported package manager found (apt/dnf/yum/pacman)."
        echo "       Install dependencies manually:"
        echo "         Debian/Ubuntu: sudo apt-get install $APT_DEPS"
        echo "         Fedora:        sudo dnf install $DNF_DEPS"
        echo "         Arch:          sudo pacman -S $PACMAN_DEPS"
        exit 1
    fi
fi

echo "==> Preflight checks"

# Collect ALL missing deps up front and report them in one shot, so you
# install everything in a single command (no more "fix one, re-run, find
# the next one missing" loop).
MISSING_TOOLS=()
for tool in git make gcc nasm pkg-config; do
    command -v "$tool" >/dev/null 2>&1 || MISSING_TOOLS+=("$tool")
done

MISSING_LIBS=()
pkg-config --exists dav1d 2>/dev/null || MISSING_LIBS+=("dav1d")
pkg-config --exists aom   2>/dev/null || MISSING_LIBS+=("aom")

if [ ${#MISSING_TOOLS[@]} -gt 0 ] || [ ${#MISSING_LIBS[@]} -gt 0 ]; then
    echo "ERROR: missing dependencies — install ALL of these at once:"
    if [ ${#MISSING_TOOLS[@]} -gt 0 ]; then echo "  build tools:   ${MISSING_TOOLS[*]}"; fi
    if [ ${#MISSING_LIBS[@]}  -gt 0 ]; then echo "  dev libraries: ${MISSING_LIBS[*]}"; fi
    echo
    echo "One command to install everything:"
    echo "  Debian/Ubuntu: sudo apt-get install $APT_DEPS"
    echo "  Fedora:        sudo dnf install $DNF_DEPS"
    echo "  Arch:          sudo pacman -S $PACMAN_DEPS"
    echo
    echo "Tip: re-run with --install-deps to install these automatically."
    exit 1
fi

# Handle an existing deploy dir (CODEC_PREFIX). `make install` overwrites
# files, so reuse is usually fine — but a clean start is safer if the build
# config changed (e.g. a backend was added/removed). Ask interactively when
# possible; --clean / --reuse skip the prompt (CI-safe).
if [ -d "${CODEC_PREFIX}" ]; then
    if [ -t 0 ] && [ "${CLEAN}" -eq 0 ] && [ "${REUSE}" -eq 0 ]; then
        read -r -p "Deploy dir ${CODEC_PREFIX} exists. Reuse [R] or Delete & rebuild [D]? [R]: " _ans
        case "${_ans}" in
            [Dd]*) echo "==> Deleting deploy dir"; rm -rf "${CODEC_PREFIX}" ;;
            *)     echo "==> Reusing existing deploy dir" ;;
        esac
    elif [ "${CLEAN}" -eq 1 ]; then
        echo "==> Deleting deploy dir (--clean)"
        rm -rf "${CODEC_PREFIX}"
    else
        echo "==> Reusing existing deploy dir"
    fi
fi

echo "==> FFmpeg source: ${FFMPEG_SRC} (tag ${FFMPEG_TAG})"
if [ ! -d "${FFMPEG_SRC}/.git" ]; then
    git clone https://git.ffmpeg.org/ffmpeg.git "${FFMPEG_SRC}"
fi
git -C "${FFMPEG_SRC}" fetch --tags origin
git -C "${FFMPEG_SRC}" checkout "${FFMPEG_TAG}"

BUILD_DIR="${FFMPEG_SRC}/build-av1"
echo "==> Configuring (prefix: ${CODEC_PREFIX})"
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"
../configure --prefix="${CODEC_PREFIX}" \
    --enable-shared \
    --enable-libdav1d \
    --enable-libaom \
    --enable-network \
    --disable-doc

echo "==> Building with ${JOBS} jobs"
make -j"${JOBS}"
make install

echo "==> Verifying"
AVCODEC_LIB="$(ls "${CODEC_PREFIX}"/lib/libavcodec.so.63* 2>/dev/null | head -1)"
if [ -z "${AVCODEC_LIB}" ]; then
    echo "ERROR: libavcodec.so.63 not found in ${CODEC_PREFIX}/lib"
    exit 1
fi
if nm -D "${AVCODEC_LIB}" | grep -q dav1d; then
    echo "OK: AV1 (dav1d) decoder present in ${AVCODEC_LIB}"
else
    echo "ERROR: dav1d symbols missing from ${AVCODEC_LIB}"
    exit 1
fi

# Report which hwaccel backends made it into the build (auto-detected by
# FFmpeg). This is the confirmation you were missing: if libva/libvdpau/
# vulkan weren't installed, the corresponding backend is silently skipped.
echo "==> Hwaccel backends in this build"
if [ -x "${CODEC_PREFIX}/bin/ffmpeg" ]; then
    HWACCELS=$("${CODEC_PREFIX}/bin/ffmpeg" -hwaccels 2>/dev/null | tail -n +2 | tr '\n' ' ' | sed 's/ *$//')
    if [ -n "${HWACCELS}" ]; then
        echo "OK: ${HWACCELS}"
    else
        echo "WARNING: no hwaccel backends detected."
        echo "         Install the dev packages (libva/libvdpau/libvulkan) and rebuild for GPU acceleration."
    fi
else
    echo "WARNING: ${CODEC_PREFIX}/bin/ffmpeg not found; skipping hwaccel check."
fi

echo "==> Done. Codec libs: ${CODEC_PREFIX}/lib"
