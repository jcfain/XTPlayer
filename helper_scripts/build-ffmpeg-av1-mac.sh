#!/bin/bash
# ---------------------------------------------------------------------------
# Build a UNIVERSAL (arm64 + x86_64) FFmpeg n9.0.1 with AV1 (dav1d + libaom)
# for macOS, for the XTPlayer Qt FFmpeg media backend.
#
# The dylibs are dropped into XTPlayer.app/Contents/Frameworks/ BEFORE
# macdeployqt runs, so macdeployqt fixes up their install names and bundles
# them. VideoToolbox hwaccel is enabled automatically by FFmpeg on macOS.
#
# Build strategy:
#   dav1d:  built twice (arm64, x86_64) and lipo'd — meson has no fat mode.
#   aom:    CMake universal build (CMAKE_OSX_ARCHITECTURES).
#   ffmpeg: --enable-universal-binaries.
#
# Usage:   helper_scripts/build-ffmpeg-av1-mac.sh [--clean|--reuse]
# Env:     CODEC_PREFIX  install prefix (default: $HOME/git/ffmpeg-n9.0.1-av1-mac)
#          FFMPEG_SRC    FFmpeg source dir (default: $HOME/git/ffmpeg)
#          JOBS          parallel make jobs (default: nproc)
#
# CI-safe: never prompts; fails fast with install hints when deps are missing.
# ---------------------------------------------------------------------------
set -e

FFMPEG_TAG=n9.0.1
DAV1D_TAG=v1.5.1
AOM_TAG=v3.15.2
FFMPEG_SRC="${FFMPEG_SRC:-${HOME}/git/ffmpeg}"
CODEC_PREFIX="${CODEC_PREFIX:-${HOME}/git/ffmpeg-n9.0.1-av1-mac}"
JOBS="${JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 4)}"

CLEAN=0
REUSE=0
for arg in "$@"; do
    case "$arg" in
        --clean) CLEAN=1 ;;
        --reuse) REUSE=1 ;;
    esac
done

echo "==> Preflight checks"

MISSING_TOOLS=()
for tool in git make nasm pkg-config xcrun lipo meson ninja; do
    command -v "$tool" >/dev/null 2>&1 || MISSING_TOOLS+=("$tool")
done
if [ ${#MISSING_TOOLS[@]} -gt 0 ]; then
    echo "ERROR: missing dependencies: ${MISSING_TOOLS[*]}"
    echo "Install them:  brew install nasm meson ninja"
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
# dav1d (two native builds + lipo)
# ---------------------------------------------------------------------------
if [ ! -f "${CODEC_PREFIX}/lib/libdav1d.dylib" ]; then
    echo "==> Building dav1d ${DAV1D_TAG} (arm64 + x86_64)"
    if [ ! -d "${DAV1D_SRC}/.git" ]; then
        git clone --depth 1 --branch "${DAV1D_TAG}" \
            https://code.videolan.org/videolan/dav1d.git "${DAV1D_SRC}"
    fi
    for arch in arm64 x86_64; do
        echo "==> dav1d: ${arch}"
        rm -rf "${WORKDIR}/dav1d-build-${arch}"
        CFLAGS="-arch ${arch}" CXXFLAGS="-arch ${arch}" \
        meson setup "${WORKDIR}/dav1d-build-${arch}" "${DAV1D_SRC}" \
            --prefix="${WORKDIR}/dav1d-prefix-${arch}" --libdir=lib \
            -Dasm=true -Denable_docs=false
        ninja -C "${WORKDIR}/dav1d-build-${arch}" -j"${JOBS}"
        ninja -C "${WORKDIR}/dav1d-build-${arch}" install
    done
    mkdir -p "${CODEC_PREFIX}/lib" "${CODEC_PREFIX}/include"
    lipo -create -output "${CODEC_PREFIX}/lib/libdav1d.dylib" \
        "${WORKDIR}/dav1d-prefix-arm64/lib/libdav1d.dylib" \
        "${WORKDIR}/dav1d-prefix-x86_64/lib/libdav1d.dylib"
    cp -R "${WORKDIR}/dav1d-prefix-arm64/include/." "${CODEC_PREFIX}/include/"
    # dav1d's .pc from the per-arch prefixes points at the wrong prefix;
    # write one that points at the merged universal prefix.
    mkdir -p "${CODEC_PREFIX}/lib/pkgconfig"
    cat > "${CODEC_PREFIX}/lib/pkgconfig/dav1d.pc" <<EOF
prefix=${CODEC_PREFIX}
libdir=\${prefix}/lib
includedir=\${prefix}/include

Name: dav1d
Description: AV1 decoding library
Version: ${DAV1D_TAG}
Libs: -L\${libdir} -ldav1d
Cflags: -I\${includedir}
EOF
else
    echo "==> dav1d already in ${CODEC_PREFIX} (skipping)"
fi

# ---------------------------------------------------------------------------
# aom (CMake universal)
# ---------------------------------------------------------------------------
if [ ! -f "${CODEC_PREFIX}/lib/libaom.dylib" ]; then
    echo "==> Building aom ${AOM_TAG} (universal)"
    if [ ! -d "${AOM_SRC}/.git" ]; then
        git clone --depth 1 --branch "${AOM_TAG}" \
            https://aomedia.googlesource.com/aom "${AOM_SRC}"
    fi
    rm -rf "${WORKDIR}/aom-build"
    cmake -S "${AOM_SRC}" -B "${WORKDIR}/aom-build" \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=ON \
        -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
        -DCMAKE_INSTALL_PREFIX="${CODEC_PREFIX}"
    cmake --build "${WORKDIR}/aom-build" -j"${JOBS}"
    cmake --install "${WORKDIR}/aom-build"
else
    echo "==> aom already in ${CODEC_PREFIX} (skipping)"
fi

# ---------------------------------------------------------------------------
# FFmpeg n9.0.1 (universal)
# ---------------------------------------------------------------------------
echo "==> FFmpeg source: ${FFMPEG_SRC} (tag ${FFMPEG_TAG})"
if [ ! -d "${FFMPEG_SRC}/.git" ]; then
    git clone --depth 1 --branch "${FFMPEG_TAG}" \
        https://git.ffmpeg.org/ffmpeg.git "${FFMPEG_SRC}"
fi
git -C "${FFMPEG_SRC}" fetch --tags origin 2>/dev/null || true
git -C "${FFMPEG_SRC}" checkout "${FFMPEG_TAG}"

BUILD_DIR="${FFMPEG_SRC}/build-av1-mac"
echo "==> Configuring (prefix: ${CODEC_PREFIX})"
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"
PKG_CONFIG_PATH="${CODEC_PREFIX}/lib/pkgconfig" \
PKG_CONFIG_LIBDIR="${CODEC_PREFIX}/lib/pkgconfig" \
../configure --prefix="${CODEC_PREFIX}" \
    --enable-universal-binaries \
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
AVCODEC_LIB="${CODEC_PREFIX}/lib/avcodec.63.dylib"
if [ ! -f "${AVCODEC_LIB}" ]; then
    echo "ERROR: avcodec.63.dylib not found in ${CODEC_PREFIX}/lib"
    exit 1
fi
if lipo -info "${AVCODEC_LIB}" | grep -q "arm64 x86_64"; then
    echo "OK: avcodec.63.dylib is universal (arm64 x86_64)"
else
    echo "ERROR: avcodec.63.dylib is NOT universal:"
    lipo -info "${AVCODEC_LIB}"
    exit 1
fi
if nm -gU "${AVCODEC_LIB}" 2>/dev/null | grep -q dav1d || \
   otool -L "${AVCODEC_LIB}" | grep -q libdav1d; then
    echo "OK: AV1 (dav1d) decoder present in avcodec.63.dylib"
else
    echo "ERROR: dav1d missing from avcodec.63.dylib"
    exit 1
fi
echo "==> Done. Codec libs: ${CODEC_PREFIX}/lib"

