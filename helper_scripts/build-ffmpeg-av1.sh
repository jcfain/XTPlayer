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
# Usage:   helper_scripts/build-ffmpeg-av1.sh
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

echo "==> Preflight checks"
for tool in git make gcc nasm pkg-config; do
    command -v "$tool" >/dev/null 2>&1 || {
        echo "ERROR: '$tool' not found. Install build tools first (e.g. sudo apt install build-essential nasm pkg-config)."
        exit 1; }
done
if ! pkg-config --exists dav1d 2>/dev/null; then
    echo "ERROR: dav1d development library not found (fast AV1 decoder)."
    echo "       Debian/Ubuntu: sudo apt install libdav1d-dev"
    echo "       Fedora:        sudo dnf install dav1d-devel"
    exit 1
fi
if ! pkg-config --exists aom 2>/dev/null; then
    echo "ERROR: aom development library not found (AV1 fallback decoder)."
    echo "       Debian/Ubuntu: sudo apt install libaom-dev"
    echo "       Fedora:        sudo dnf install aom-devel"
    exit 1
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
    --enable-vaapi \
    --enable-vdpau \
    --enable-vulkan \
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
echo "==> Done. Codec libs: ${CODEC_PREFIX}/lib"
