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
DAV1D_TAG=1.5.1
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
# GitHub Actions injects env vars in Windows form (D:\a\...); Git Bash needs
# POSIX form (/d/a/...) for [ -e ] / PATH lookups. Normalize if necessary.
case "$MINGW_PREFIX" in
    [A-Za-z]:\\*|[A-Za-z]:/*)
        if command -v cygpath >/dev/null 2>&1; then
            MINGW_PREFIX="$(cygpath -u "$MINGW_PREFIX")"
        fi
        ;;
esac
JOBS="${JOBS:-4}"
XP=x86_64-w64-mingw32
export PATH="${MINGW_PREFIX}/bin:$PATH"

# FFmpeg's configure derives ${XP}-nm, ${XP}-ar, ${XP}-ranlib and
# ${XP}-pkg-config from --cross-prefix. Qt's MinGW ships the binutils as
# ${XP}-gcc-* wrappers (no plain ${XP}-nm), so copy them under the expected
# names. (Copies, not symlinks: immune to Git Bash symlink quirks.)
for tool in ar nm ranlib dlltool; do
    if [ ! -e "${MINGW_PREFIX}/bin/${XP}-${tool}.exe" ] && [ -e "${MINGW_PREFIX}/bin/${XP}-gcc-${tool}.exe" ]; then
        cp "${MINGW_PREFIX}/bin/${XP}-gcc-${tool}.exe" "${MINGW_PREFIX}/bin/${XP}-${tool}.exe"
    fi
done
# pkg-config: a COPIED pkg-config.exe does not work from Qt's MinGW bin dir
# (it cannot find its msys-2.0.dll there), so install a wrapper that delegates
# to the MSYS2 cross pkg-config in its native location. MSYS2 does not convert
# POSIX paths in environment variables, so convert PKG_CONFIG_PATH/LIBDIR.
rm -f "${MINGW_PREFIX}/bin/${XP}-pkg-config.exe"
cat > "${MINGW_PREFIX}/bin/${XP}-pkg-config" <<'EOF'
#!/usr/bin/env bash
[ -n "${PKG_CONFIG_PATH:-}" ]   && export PKG_CONFIG_PATH="$(cygpath -m "${PKG_CONFIG_PATH}")"
[ -n "${PKG_CONFIG_LIBDIR:-}" ] && export PKG_CONFIG_LIBDIR="$(cygpath -m "${PKG_CONFIG_LIBDIR}")"
exec /c/msys64/mingw64/bin/x86_64-w64-mingw32-pkg-config.exe "$@"
EOF
chmod +x "${MINGW_PREFIX}/bin/${XP}-pkg-config"
# makedef (FFmpeg's compat/windows/makedef) builds a temporary import
# library with `lib.exe` and dumps its symbols with `dumpbin.exe` -
# neither ships with Qt's MinGW. But if AR/NM are set it uses
# `$AR rcs` / `$NM --defined-only -g` instead, so point them at the
# cross tools (copied above) and the MSVC tools are never needed.
export AR="${XP}-ar"
export NM="${XP}-nm"

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
if [ "${CLEAN}" -eq 0 ] && [ -f "${CODEC_PREFIX}/bin/avcodec-63.dll" ]; then
    echo "==> ${CODEC_PREFIX}/bin/avcodec-63.dll present — skipping build"
    exit 0
fi

echo "==> Preflight checks"

MISSING_TOOLS=()
for tool in git make nasm meson ninja cmake; do
    command -v "$tool" >/dev/null 2>&1 || MISSING_TOOLS+=("$tool")
done
[ -x "${MINGW_PREFIX}/bin/${XP}-gcc.exe" ] || MISSING_TOOLS+=("${MINGW_PREFIX} (MinGW toolchain)")
command -v "${XP}-nm" >/dev/null 2>&1 || MISSING_TOOLS+=("${XP}-nm (cross binutils)")
command -v "${XP}-pkg-config" >/dev/null 2>&1 || MISSING_TOOLS+=("${XP}-pkg-config (install mingw-w64-x86_64-pkg-config)")

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
ar = '${XP}-gcc-ar'
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
        -Denable_asm=true -Denable_docs=false
    ninja -C "${WORKDIR}/dav1d-build" -j"${JOBS}"
    ninja -C "${WORKDIR}/dav1d-build" install
else
    echo "==> dav1d already in ${CODEC_PREFIX} (skipping)"
fi

# ---------------------------------------------------------------------------
# aom (cross, CMake MinGW Makefiles)
# ---------------------------------------------------------------------------
if [ ! -f "${CODEC_PREFIX}/bin/aom.dll" ]; then
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
    # aom's CMake install doesn't ship a usable aom.pc for our prefix;
    # write one (FFmpeg's configure finds aom via pkg-config).
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
# FFmpeg n9.0.1 (cross, MinGW)
# ---------------------------------------------------------------------------
echo "==> FFmpeg source: ${FFMPEG_SRC} (tag ${FFMPEG_TAG})"
if [ ! -d "${FFMPEG_SRC}/.git" ]; then
    git clone --depth 1 --branch "${FFMPEG_TAG}" \
        https://git.ffmpeg.org/ffmpeg.git "${FFMPEG_SRC}"
fi
git -C "${FFMPEG_SRC}" fetch --tags origin 2>/dev/null || true
git -C "${FFMPEG_SRC}" checkout "${FFMPEG_TAG}"

# In-tree build: an out-of-tree build dir gets a generated one-line
# Makefile "include <abs source path>/Makefile" (configure line 8587),
# which native Windows makes (GnuWin32 3.81, MinGW mingw32-make) cannot
# resolve - they are not MSYS binaries and don't translate /c/... paths.
# In-tree, the source Makefile is used directly and works with any make.
BUILD_DIR="${FFMPEG_SRC}"
# MSYS2 make (4.x) understands POSIX /c/... paths; the default `make` on
# the runner is GnuWin32 3.81 (native Win32).
MAKE=/c/msys64/usr/bin/make.exe
[ -x "${MAKE}" ] || MAKE=make
echo "==> Tool diagnostics"
echo "MINGW_PREFIX=${MINGW_PREFIX}"
ls "${MINGW_PREFIX}/bin/" 2>/dev/null | grep -E "^${XP}-(gcc|g\+\+|ar|nm|ranlib|pkg-config)\.exe$" || echo "  (no cross tools listed!)"
command -v "${XP}-nm" "${XP}-ar" "${XP}-ranlib" "${XP}-pkg-config" || true
echo "==> Configuring (prefix: ${CODEC_PREFIX})"
cd "${BUILD_DIR}"
PKG_CONFIG_PATH="${CODEC_PREFIX}/lib/pkgconfig" \
PKG_CONFIG_LIBDIR="${CODEC_PREFIX}/lib/pkgconfig" \
./configure --prefix="${CODEC_PREFIX}" \
    --cross-prefix="${XP}-" \
    --arch=x86_64 \
    --target-os=mingw32 \
    --enable-shared \
    --enable-libdav1d \
    --enable-libaom \
    --enable-network \
    --disable-doc

echo "==> Building with ${JOBS} jobs (${MAKE})"
"${MAKE}" -j"${JOBS}"
"${MAKE}" install

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

