#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# Cross-build FFmpeg n9.0.1 with AV1 (dav1d + libaom) for Windows x86_64
# (MinGW), for the XTPlayer Qt FFmpeg media backend.
#
# Runs on a Windows machine (CI: windows-latest, via Git Bash). Uses Qt's
# bundled MinGW 13.1.0 toolchain (the same one that builds the app) as the
# cross compiler, so the DLLs match the app's ABI.
#
# Produces avcodec-63.dll, avformat-63.dll, avutil-61.dll, swscale-10.dll,
# swresample-7.dll (in $CODEC_PREFIX/bin) + dav1d.dll, aom.dll (in
# $CODEC_PREFIX/lib). Drop them NEXT TO XTPlayer.exe: the Windows DLL search
# finds the exe directory first, so they replace the AV1-less FFmpeg DLLs
# bundled by Qt automatically (no launcher needed; absent DLLs = old
# behavior, automatic fallback).
#
# Usage:   helper_scripts/build-ffmpeg-av1-win.sh [--clean|--reuse]
# Env:     CODEC_PREFIX  install prefix (default: $HOME/git/ffmpeg-n9.0.1-av1-win)
#          FFMPEG_SRC    FFmpeg source dir (default: $HOME/git/ffmpeg)
#          MINGW_PREFIX  MinGW root (default: $HOME/Qt/Tools/mingw1310_64)
#          JOBS          parallel make jobs (default: 4)
#
# CI-safe: never prompts; fails fast with install hints when deps are missing.
# ---------------------------------------------------------------------------
set -e

FFMPEG_TAG=n9.0.1
DAV1D_TAG=v1.5.1
AOM_TAG=v3.15.2
FFMPEG_SRC="${FFMPEG_SRC:-${HOME}/git/ffmpeg}"
CODEC_PREFIX="${CODEC_PREFIX:-${HOME}/git/ffmpeg-n9.0.1-av1-win}"
# A relative CODEC_PREFIX (e.g. from CI) is resolved against the current
# directory: FFmpeg's configure --prefix must be absolute.
case "$CODEC_PREFIX" in
    /*) ;;
    *) CODEC_PREFIX="$(pwd)/${CODEC_PREFIX}" ;;
esac
MINGW_PREFIX="${MINGW_PREFIX:-${HOME}/Qt/Tools/mingw1310_64}"
JOBS="${JOBS:-4}"
XP=x86_64-w64-mingw32
export PATH="${MINGW_PREFIX}/bin:$PATH"

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
for tool in git make nasm meson ninja cmake; do
    command -v "$tool" >/dev/null 2>&1 || MISSING_TOOLS+=("$tool")
done
[ -x "${MINGW_PREFIX}/bin/${XP}-gcc.exe" ] || MISSING_TOOLS+=("${MINGW_PREFIX} (MinGW toolchain)")

if [ ${#MISSING_TOOLS[@]} -gt 0 ]; then
    echo "ERROR: missing dependencies: ${MISSING_TOOLS[*]}"
    echo "Install them (GitHub windows-latest):"
    echo "  choco install nasm -y"
    echo "  pip install meson ninja"
    echo "  (MinGW comes with the Qt installer: Tools/mingw1310_64)"
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
if [ ! -f "${CODEC_PREFIX}/lib/dav1d.dll" ]; then
    echo "==> Building dav1d ${DAV1D_TAG} (${XP})"
    if [ ! -d "${DAV1D_SRC}/.git" ]; then
        git clone --depth 1 --branch "${DAV1D_TAG}" \
            https://code.videolan.org/videolan/dav1d.git "${DAV1D_SRC}"
    fi
    cat > "${WORKDIR}/dav1d-mingw.txt" <<EOF
[binaries]
c = '${XP}-gcc'
cpp = '${XP}-g++'
ar = '${XP}-ar'
strip = '${XP}-strip'
[host_machine]
system = 'windows'
cpu_family = 'x86_64'
cpu = 'x86_64'
endian = 'little'
EOF
    rm -rf "${WORKDIR}/dav1d-build"
    meson setup "${WORKDIR}/dav1d-build" "${DAV1D_SRC}" \
        --prefix="${CODEC_PREFIX}" --libdir=lib \
        --cross-file="${WORKDIR}/dav1d-mingw.txt" \
        -Dasm=true -Denable_docs=false
    ninja -C "${WORKDIR}/dav1d-build" -j"${JOBS}"
    ninja -C "${WORKDIR}/dav1d-build" install
else
    echo "==> dav1d already in ${CODEC_PREFIX} (skipping)"
fi

# ---------------------------------------------------------------------------
# aom (cross, CMake MinGW Makefiles)
# ---------------------------------------------------------------------------
if [ ! -f "${CODEC_PREFIX}/lib/aom.dll" ]; then
    echo "==> Building aom ${AOM_TAG} (${XP})"
    if [ ! -d "${AOM_SRC}/.git" ]; then
        git clone --depth 1 --branch "${AOM_TAG}" \
            https://aomedia.googlesource.com/aom "${AOM_SRC}"
    fi
    rm -rf "${WORKDIR}/aom-build"
    cmake -S "${AOM_SRC}" -B "${WORKDIR}/aom-build" -G "MinGW Makefiles" \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=ON \
        -DCMAKE_C_COMPILER="${MINGW_PREFIX}/bin/${XP}-gcc.exe" \
        -DCMAKE_CXX_COMPILER="${MINGW_PREFIX}/bin/${XP}-g++.exe" \
        -DCMAKE_ASM_COMPILER="${MINGW_PREFIX}/bin/${XP}-gcc.exe" \
        -DCMAKE_INSTALL_PREFIX="${CODEC_PREFIX}"
    cmake --build "${WORKDIR}/aom-build" -j"${JOBS}"
    cmake --install "${WORKDIR}/aom-build"
else
    echo "==> aom already in ${CODEC_PREFIX} (skipping)"
fi

# ---------------------------------------------------------------------------
# FFmpeg n9.0.1 (cross, MinGW)
# ---------------------------------------------------------------------------
echo "==> FFmpeg source: ${FFMPEG_SRC} (tag ${FFMPEG_TAG})"
if [ ! -d "${FFMPEG_SRC}/.git" ]; then
    git clone --depth 1 --branch "${FFMPEG_TAG}" \
        https://git.ffmpeg.org/ffmpeg.git "${FFMPEG_SRC}"
fi
git -C "${FFMPEG_SRC}" fetch --tags origin 2>/dev/null || true
git -C "${FFMPEG_SRC}" checkout "${FFMPEG_TAG}"

BUILD_DIR="${FFMPEG_SRC}/build-av1-win"
echo "==> Configuring (prefix: ${CODEC_PREFIX})"
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"
PKG_CONFIG_PATH="${CODEC_PREFIX}/lib/pkgconfig" \
PKG_CONFIG_LIBDIR="${CODEC_PREFIX}/lib/pkgconfig" \
../configure --prefix="${CODEC_PREFIX}" \
    --cross-prefix="${XP}-" \
    --arch=x86_64 \
    --pkg-config-flags="--define-prefix=${CODEC_PREFIX}" \
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
AVCODEC_DLL="${CODEC_PREFIX}/bin/avcodec-63.dll"
if [ ! -f "${AVCODEC_DLL}" ]; then
    echo "ERROR: avcodec-63.dll not found in ${CODEC_PREFIX}/bin"
    exit 1
fi
if "${MINGW_PREFIX}/bin/${XP}-nm.exe" -D "${AVCODEC_DLL}" | grep -q dav1d; then
    echo "OK: AV1 (dav1d) decoder present in avcodec-63.dll"
else
    echo "ERROR: dav1d symbols missing from avcodec-63.dll"
    exit 1
fi
for dll in avformat-63.dll avutil-61.dll swscale-10.dll swresample-7.dll; do
    [ -f "${CODEC_PREFIX}/bin/${dll}" ] || { echo "ERROR: ${dll} missing"; exit 1; }
done
[ -f "${CODEC_PREFIX}/lib/dav1d.dll" ] || { echo "ERROR: dav1d.dll missing"; exit 1; }
echo "==> Done. Codec DLLs: ${CODEC_PREFIX}/bin + ${CODEC_PREFIX}/lib"

