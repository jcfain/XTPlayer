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
#   ffmpeg: built per-arch (arm64, x86_64) and lipo'd — no fat mode.
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
DAV1D_TAG=1.5.1
AOM_TAG=v3.15.2
FFMPEG_SRC="${FFMPEG_SRC:-${HOME}/git/ffmpeg}"
CODEC_PREFIX="${CODEC_PREFIX:-${HOME}/git/ffmpeg-n9.0.1-av1-mac}"
# A relative CODEC_PREFIX (e.g. from CI) is resolved against the current
# directory: FFmpeg's configure --prefix must be absolute.
case "$CODEC_PREFIX" in
    /*) ;;
    *) CODEC_PREFIX="$(pwd)/${CODEC_PREFIX}" ;;
esac
JOBS="${JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 4)}"

CLEAN=0
REUSE=0
for arg in "$@"; do
    case "$arg" in
        --clean) CLEAN=1 ;;
        --reuse) REUSE=1 ;;
    esac
done

# CI cache: if the final artifact is already in place (restored from
# actions/cache), skip the whole build. --clean forces a rebuild.
if [ "${CLEAN}" -eq 0 ] && [ -f "${CODEC_PREFIX}/lib/libavcodec.63.dylib" ]; then
    echo "==> ${CODEC_PREFIX}/lib/libavcodec.63.dylib present — skipping build"
    exit 0
fi

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
        # Pin cpu_family via a cross file: on Apple Silicon runners,
        # `cc -dumpmachine` reports arm64 even with -arch x86_64, which
        # makes meson compile the ARM asm into the x86_64 slice.
        case "${arch}" in
            arm64)  cpu=aarch64 ;;
            x86_64) cpu=x86_64 ;;
        esac
        cat > "${WORKDIR}/dav1d-${arch}.txt" <<EOF
[binaries]
c = 'cc'
cpp = 'c++'
ar = 'ar'
strip = 'strip'
[host_machine]
system = 'darwin'
cpu_family = '${cpu}'
cpu = '${cpu}'
endian = 'little'
EOF
        CFLAGS="-arch ${arch}" CXXFLAGS="-arch ${arch}" \
        meson setup "${WORKDIR}/dav1d-build-${arch}" "${DAV1D_SRC}" \
            --cross-file="${WORKDIR}/dav1d-${arch}.txt" \
            --prefix="${WORKDIR}/dav1d-prefix-${arch}" --libdir=lib \
            -Denable_asm=true -Denable_docs=false
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
    # aom's CMake enables NEON intrinsics whenever arm64 is in the arch
    # list, so a single universal build compiles NEON C for x86_64 and
    # fails. Build per-arch (each picks its own intrinsics) and lipo.
    # AOM_TARGET_CPU must be pinned too: aom auto-detects from
    # CMAKE_SYSTEM_PROCESSOR, which is the HOST cpu (arm64 on Apple
    # Silicon) even when cross-building the x86_64 slice.
    for arch in arm64 x86_64; do
        rm -rf "${WORKDIR}/aom-build-${arch}"
        cmake -S "${AOM_SRC}" -B "${WORKDIR}/aom-build-${arch}" \
            -DCMAKE_BUILD_TYPE=Release \
            -DBUILD_SHARED_LIBS=ON \
            -DCMAKE_OSX_ARCHITECTURES="${arch}" \
            -DAOM_TARGET_CPU="${arch}" \
            -DCMAKE_INSTALL_PREFIX="${WORKDIR}/aom-prefix-${arch}"
        cmake --build "${WORKDIR}/aom-build-${arch}" -j"${JOBS}"
        cmake --install "${WORKDIR}/aom-build-${arch}"
    done
    mkdir -p "${CODEC_PREFIX}/lib" "${CODEC_PREFIX}/include"
    lipo -create -output "${CODEC_PREFIX}/lib/libaom.dylib" \
        "${WORKDIR}/aom-prefix-arm64/lib/libaom.dylib" \
        "${WORKDIR}/aom-prefix-x86_64/lib/libaom.dylib"
    cp -R "${WORKDIR}/aom-prefix-arm64/include/." "${CODEC_PREFIX}/include/"
    # aom's .pc from the per-arch prefixes points at the wrong prefix;
    # write one that points at the merged universal prefix.
    mkdir -p "${CODEC_PREFIX}/lib/pkgconfig"
    cat > "${CODEC_PREFIX}/lib/pkgconfig/aom.pc" <<EOF
prefix=${CODEC_PREFIX}
libdir=\${prefix}/lib
includedir=\${prefix}/include

Name: aom
Description: AV1 encoding and decoding library
Version: ${AOM_TAG#v}
Libs: -L\${libdir} -laom
Cflags: -I\${includedir}
EOF
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

# FFmpeg's configure has no universal mode: build each arch against the
# universal dav1d/aom, then lipo the dylibs together.
for arch in arm64 x86_64; do
    BUILD_DIR="${FFMPEG_SRC}/build-av1-mac-${arch}"
    echo "==> Configuring ${arch} (prefix: ${WORKDIR}/ffmpeg-prefix-${arch})"
    rm -rf "${BUILD_DIR}"
    mkdir -p "${BUILD_DIR}"
    cd "${BUILD_DIR}"
    PKG_CONFIG_PATH="${CODEC_PREFIX}/lib/pkgconfig" \
    PKG_CONFIG_LIBDIR="${CODEC_PREFIX}/lib/pkgconfig" \
    CFLAGS="-arch ${arch}" CXXFLAGS="-arch ${arch}" LDFLAGS="-arch ${arch}" \
    ../configure --prefix="${WORKDIR}/ffmpeg-prefix-${arch}" \
        --arch="${arch}" \
        --enable-shared \
        --enable-libdav1d \
        --enable-libaom \
        --enable-network \
        --disable-doc

    echo "==> Building ${arch} with ${JOBS} jobs"
    make -j"${JOBS}"
    make install
done
cd "${WORKDIR}"

echo "==> Lipo'ing FFmpeg dylibs into ${CODEC_PREFIX}"
mkdir -p "${CODEC_PREFIX}/lib" "${CODEC_PREFIX}/include"
# FFmpeg's darwin SLIBNAME_WITH_MAJOR is lib<name>.<major>.dylib (SLIBPREF=lib).
for lib in libavcodec.63 libavformat.63 libavutil.61 libswscale.10 libswresample.7; do
    lipo -create -output "${CODEC_PREFIX}/lib/${lib}.dylib" \
        "${WORKDIR}/ffmpeg-prefix-arm64/lib/${lib}.dylib" \
        "${WORKDIR}/ffmpeg-prefix-x86_64/lib/${lib}.dylib"
done
cp -R "${WORKDIR}/ffmpeg-prefix-arm64/include/." "${CODEC_PREFIX}/include/"

# ---------------------------------------------------------------------------
# Verify
# ---------------------------------------------------------------------------
AVCODEC_LIB="${CODEC_PREFIX}/lib/libavcodec.63.dylib"
if [ ! -f "${AVCODEC_LIB}" ]; then
    echo "ERROR: libavcodec.63.dylib not found in ${CODEC_PREFIX}/lib"
    exit 1
fi
archs="$(lipo -archs "${AVCODEC_LIB}")"
if [[ "${archs}" == *arm64* && "${archs}" == *x86_64* ]]; then
    echo "OK: libavcodec.63.dylib is universal (${archs})"
else
    echo "ERROR: libavcodec.63.dylib is NOT universal: ${archs}"
    exit 1
fi
if nm -gU "${AVCODEC_LIB}" 2>/dev/null | grep -q dav1d || \
   otool -L "${AVCODEC_LIB}" | grep -q libdav1d; then
    echo "OK: AV1 (dav1d) decoder present in libavcodec.63.dylib"
else
    echo "ERROR: dav1d missing from libavcodec.63.dylib"
    exit 1
fi
echo "==> Done. Codec libs: ${CODEC_PREFIX}/lib"

