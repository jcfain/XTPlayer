#!/usr/bin/env bash
# =============================================================================
# install-qt.sh — install Qt from the current download.qt.io layout.
#
# aqtinstall (<= 3.3.0, used by jurplel/install-qt-action) only knows the old
# flat layout (online/qtsdkrepository/<os>/desktop/qt6_XXXX/Updates.xml), which
# Qt removed from their servers. The current layout is per-arch:
#
#   online/qtsdkrepository/<os>/desktop/qt6_XXXX/qt6_XXXX[_mingw]/
#     qt.qt6.XXXX.<arch>/...               (base package)
#     qt.qt6.XXXX.addons.<mod>.<arch>/...  (addon packages)
#
# (inner dir: qt6_XXXX_mingw on Windows, qt6_XXXX on mac/linux)
#
# This script downloads the .7z archives from those package dirs (archive
# names carry a version/timestamp prefix, so the dir is listed first),
# verifies the published .sha1 sidecars, and extracts everything flat into
# $QT_DIR so that $QT_DIR/bin/qmake exists exactly where the workflow expects.
#
# Required env:
#   QT_OS_DIR   linux_x64 | linux_arm64 | windows_x86 | mac_x64
#   QT_ARCH     linux_gcc_64 | linux_gcc_arm64 | win64_mingw | clang_64
#   QT_DIR      target directory (extracted flat)
# Optional env:
#   QT_VERSION  default 6.12.0
#   QT_MODULES  space-separated addon module names (default: qtmultimedia)
#   QT_TOOLS    e.g. mingw1510 -> $QT_DIR/Tools/mingw1510_64 (Windows MinGW)
#
# Requires: curl, 7z, sha1sum (all preinstalled on GitHub-hosted runners).
# =============================================================================
set -euo pipefail

VER="${QT_VERSION:-6.12.0}"
VERNUM="${VER//./}"                                # e.g. 6120
MODULES="${QT_MODULES:-qtmultimedia}"
BASE="https://download.qt.io/online/qtsdkrepository/${QT_OS_DIR}/desktop"

# Package dirs are nested under the version dir; on Windows the inner dir
# carries the arch suffix, on mac/linux it repeats the version.
case "${QT_OS_DIR}" in
  windows_x86) PKG_SUBDIR="qt6_${VERNUM}/qt6_${VERNUM}_mingw" ;;
  *)           PKG_SUBDIR="qt6_${VERNUM}/qt6_${VERNUM}" ;;
esac

mkdir -p "${QT_DIR}"

# Download every .7z in a package dir, verify its .sha1, extract into $dest.
dl_pkg() {
  local pkg="$1" dest="$2"
  local url="${BASE}/${PKG_SUBDIR}/${pkg}"
  local tmp f expected actual
  tmp=$(mktemp -d)
  curl -fsSL "${url}/" \
    | grep -oE 'href="[^"]+\.7z"' | sed 's/href="//;s/"$//' \
    | grep -v 'meta\.7z$' \
    | while read -r f; do
        curl -fsSL "${url}/${f}" -o "${tmp}/${f}"
        # Integrity check against the published sidecar (bare hash or
        # "hash filename" - take the first field either way)
        expected=$(curl -fsSL "${url}/${f}.sha1" | awk '{print $1}')
        actual=$(sha1sum "${tmp}/${f}" | awk '{print $1}')
        if [ "${expected}" != "${actual}" ]; then
          echo "SHA1 MISMATCH for ${f}: expected ${expected}, got ${actual}" >&2
          exit 1
        fi
        # The ICU archive carries its .so files at the archive root; the Qt
        # installer places them in lib/ (qmake's RUNPATH is $ORIGIN/../lib).
        case "${f}" in
          *icu-*) 7z x -y "${tmp}/${f}" -o"${dest}/lib" >/dev/null ;;
          *)      7z x -y "${tmp}/${f}" -o"${dest}" >/dev/null ;;
        esac
        rm -f "${tmp}/${f}"
      done
  rm -rf "${tmp}"
}

# Base Qt + requested addons, all merged flat into $QT_DIR.
dl_pkg "qt.qt6.${VERNUM}.${QT_ARCH}" "${QT_DIR}"
for m in ${MODULES}; do
  dl_pkg "qt.qt6.${VERNUM}.addons.${m}.${QT_ARCH}" "${QT_DIR}"
done

# Optional toolchain (Windows MinGW): lives in a separate tools dir.
if [ -n "${QT_TOOLS:-}" ]; then
  tmp=$(mktemp -d)
  url="${BASE}/tools_${QT_TOOLS}/qt.tools.win64_${QT_TOOLS}"
  curl -fsSL "${url}/" \
    | grep -oE 'href="[^"]+\.7z"' | sed 's/href="//;s/"$//' \
    | grep -v 'meta\.7z$' \
    | while read -r f; do
        curl -fsSL "${url}/${f}" -o "${tmp}/${f}"
        7z x -y "${tmp}/${f}" -o"${QT_DIR}/Tools/${QT_TOOLS}_64" >/dev/null
        rm -f "${tmp}/${f}"
      done
  rm -rf "${tmp}"
  test -d "${QT_DIR}/Tools/${QT_TOOLS}_64/bin" \
    || { echo "MinGW toolchain ${QT_TOOLS} not found - Qt may ship a new one" >&2; exit 1; }
fi

# Fail fast if the layout ever changes again.
if [ -f "${QT_DIR}/bin/qmake" ] || [ -f "${QT_DIR}/bin/qmake.exe" ]; then
  echo "Qt ${VER} ready at ${QT_DIR}"
else
  echo "qmake not found in ${QT_DIR}/bin - download layout may have changed" >&2
  exit 1
fi
