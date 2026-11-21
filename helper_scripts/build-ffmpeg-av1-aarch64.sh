#!/bin/bash
# ---------------------------------------------------------------------------
# Cross-build FFmpeg n9.0.1 with AV1 software decoding (dav1d + libaom)
# targeting aarch64 Linux, for the XTPlayer Qt FFmpeg media backend.
#
# Companion to build-ffmpeg-av1.sh (native x86_64): this one runs on an
# x86_64 build host (e.g. a CI runner) and produces aarch64 libraries for
# the cross-compiled aarch64 AppImage. dav1d and aom are cross-built from
# source (there are no aarch64 dev packages in apt).
#
# No hwaccel backends: cross-building VAAPI/VDPAU/Vulkan would require
# aarch64 sysroot headers, so aarch64 gets software AV1 decoding only
# (dav1d is fast on ARM cores). AV1 works; GPU acceleration does not.
#
# Usage:   helper_scripts/build-ffmpeg-av1-aarch64.sh [--install-deps] [--clean|--reuse]
# Env:     CODEC_PREFIX  install prefix (default: $HOME/git/ffmpeg-n9.0.1-av1-aarch64)
#          FFMPEG_SRC    FFmpeg source dir (default: $HOME/git/ffmpeg)
#          JOBS          parallel make jobs (default: nproc)
#
# CI-safe: never prompts; fails fast with install hints when deps are missing.
# ---------------------------------------------------------------------------
set -e

FFMPEG_TAG=n9.0.1
DAV1D_TAG=1.5.1
AOM_TAG=v3.15.2
FFMPEG_SRC="${FFMPEG_SRC:-${HOME}/git/ffmpeg}"
CODEC_PREFIX="${CODEC_PREFIX:-${HOME}/git/ffmpeg-n9.0.1-av1-aarch64}"
# A relative CODEC_PREFIX (e.g. from CI) is resolved against the current
# directory: FFmpeg's configure --prefix must be absolute.
case "$CODEC_PREFIX" in
    /*) ;;
    *) CODEC_PREFIX="$(pwd)/${CODEC_PREFIX}" ;;
esac
JOBS="${JOBS:-$(nproc)}"
CROSS=aarch64-linux-gnu

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

APT_DEPS="build-essential nasm pkg-config git meson ninja-build \
          gcc-aarch64-linux-gnu g++-aarch64-linux-gnu binutils-aarch64-linux-gnu"

if [ "$INSTALL_DEPS" -eq 1 ]; then
    if command -v apt-get >/dev/null 2>&1; then
        echo "==> Installing dependencies via apt-get"
        sudo apt-get update
        sudo apt-get install -y $APT_DEPS
    else
        echo "ERROR: this script supports apt-get only (GitHub ubuntu runners)."
        echo "       Install dependencies manually: $APT_DEPS"
        exit 1
    fi
fi

# CI cache: if the final artifact is already in place (restored from
# actions/cache), skip the whole build. --clean forces a rebuild.
# Runs AFTER the dep install above (same reason as the x86_64 script).
if [ "${CLEAN}" -eq 0 ] && [ -f "${CODEC_PREFIX}/lib/libavcodec.so.63" ]; then
    echo "==> ${CODEC_PREFIX}/lib/libavcodec.so.63 present — skipping build"
    exit 0
fi

echo "==> Preflight checks"

MISSING_TOOLS=()
for tool in git make nasm pkg-config meson ninja; do
    command -v "$tool" >/dev/null 2>&1 || MISSING_TOOLS+=("$tool")
done
command -v "${CROSS}-gcc" >/dev/null 2>&1 || MISSING_TOOLS+=("${CROSS}-gcc")
command -v "${CROSS}-g++" >/dev/null 2>&1 || MISSING_TOOLS+=("${CROSS}-g++")

if [ ${#MISSING_TOOLS[@]} -gt 0 ]; then
    echo "ERROR: missing dependencies — install ALL of these at once:"
    echo "  ${MISSING_TOOLS[*]}"
    echo
    echo "One command to install everything:"
    echo "  sudo apt-get install $APT_DEPS"
    exit 1
fi

# Handle an existing deploy dir (CODEC_PREFIX).
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

WORKDIR="$(pwd)"
DAV1D_SRC="${WORKDIR}/dav1d-src"
AOM_SRC="${WORKDIR}/aom-src"

# ---------------------------------------------------------------------------
# dav1d (cross, meson)
# ---------------------------------------------------------------------------
if [ ! -f "${CODEC_PREFIX}/lib/libdav1d.so" ]; then
    echo "==> Building dav1d ${DAV1D_TAG} (${CROSS})"
    if [ ! -d "${DAV1D_SRC}/.git" ]; then
        git clone --depth 1 --branch "${DAV1D_TAG}" \
            https://code.videolan.org/videolan/dav1d.git "${DAV1D_SRC}"
    fi
    cat > "${WORKDIR}/dav1d-${CROSS}.txt" <<EOF
[binaries]
c = '${CROSS}-gcc'
cpp = '${CROSS}-g++'
ar = '${CROSS}-ar'
strip = '${CROSS}-strip'
[host_machine]
system = 'linux'
cpu_family = 'aarch64'
cpu = 'aarch64'
endian = 'little'
EOF
    rm -rf "${WORKDIR}/dav1d-build"
    meson setup "${WORKDIR}/dav1d-build" "${DAV1D_SRC}" \
        --prefix="${CODEC_PREFIX}" --libdir=lib \
        --cross-file="${WORKDIR}/dav1d-${CROSS}.txt" \
        -Denable_asm=true -Denable_docs=false
    ninja -C "${WORKDIR}/dav1d-build" -j"${JOBS}"
    ninja -C "${WORKDIR}/dav1d-build" install
else
    echo "==> dav1d already in ${CODEC_PREFIX} (skipping)"
fi

# ---------------------------------------------------------------------------
# aom (cross, CMake)
# ---------------------------------------------------------------------------
if [ ! -f "${CODEC_PREFIX}/lib/libaom.so" ]; then
    echo "==> Building aom ${AOM_TAG} (${CROSS})"
    if [ ! -d "${AOM_SRC}/.git" ]; then
        git clone --depth 1 --branch "${AOM_TAG}" \
            https://aomedia.googlesource.com/aom "${AOM_SRC}"
    fi
    cat > "${WORKDIR}/aom-${CROSS}.cmake" <<EOF
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER ${CROSS}-gcc)
set(CMAKE_CXX_COMPILER ${CROSS}-g++)
set(CMAKE_ASM_COMPILER ${CROSS}-gcc)
EOF
    rm -rf "${WORKDIR}/aom-build"
    cmake -S "${AOM_SRC}" -B "${WORKDIR}/aom-build" \
        -DCMAKE_TOOLCHAIN_FILE="${WORKDIR}/aom-${CROSS}.cmake" \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=ON \
        -DCMAKE_INSTALL_PREFIX="${CODEC_PREFIX}"
    cmake --build "${WORKDIR}/aom-build" -j"${JOBS}"
    cmake --install "${WORKDIR}/aom-build"
else
    echo "==> aom already in ${CODEC_PREFIX} (skipping)"
fi

# ---------------------------------------------------------------------------
# FFmpeg n9.0.1 (cross)
# ---------------------------------------------------------------------------
echo "==> FFmpeg source: ${FFMPEG_SRC} (tag ${FFMPEG_TAG})"
if [ ! -d "${FFMPEG_SRC}/.git" ]; then
    git clone --depth 1 --branch "${FFMPEG_TAG}" \
        https://git.ffmpeg.org/ffmpeg.git "${FFMPEG_SRC}"
fi
git -C "${FFMPEG_SRC}" fetch --tags origin 2>/dev/null || true
git -C "${FFMPEG_SRC}" checkout "${FFMPEG_TAG}"

BUILD_DIR="${FFMPEG_SRC}/build-av1-aarch64"
echo "==> Configuring (prefix: ${CODEC_PREFIX})"
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"
PKG_CONFIG_PATH="${CODEC_PREFIX}/lib/pkgconfig" \
PKG_CONFIG_LIBDIR="${CODEC_PREFIX}/lib/pkgconfig" \
../configure --prefix="${CODEC_PREFIX}" \
    --cross-prefix="${CROSS}-" \
    --arch=arm64 \
    --target-os=linux \
    --enable-shared \
    --enable-libdav1d \
    --enable-libaom \
    --enable-network \
    --disable-doc

echo "==> Building with ${JOBS} jobs"
make -j"${JOBS}"
make install

# ---------------------------------------------------------------------------
# Verify
# ---------------------------------------------------------------------------
AVCODEC_LIB="$(ls "${CODEC_PREFIX}"/lib/libavcodec.so.63.* 2>/dev/null | head -1)"
if [ -z "${AVCODEC_LIB}" ]; then
    echo "ERROR: libavcodec.so.63 not found in ${CODEC_PREFIX}/lib"
    exit 1
fi
if "${CROSS}-nm" -D "${AVCODEC_LIB}" | grep -q dav1d; then
    echo "OK: AV1 (dav1d) decoder present in ${AVCODEC_LIB}"
else
    echo "ERROR: dav1d symbols missing from ${AVCODEC_LIB}"
    exit 1
fi
# Sanity: the library must be aarch64, not the host arch. (-L: follow
# symlinks; file does not by default.)
if file -L "${AVCODEC_LIB}" | grep -q aarch64; then
    echo "OK: library is aarch64"
else
    echo "ERROR: library is NOT aarch64:"
    file "${AVCODEC_LIB}"
    exit 1
fi
echo "==> Done. Codec libs: ${CODEC_PREFIX}/lib"

