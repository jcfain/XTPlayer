#!/bin/bash
# ---------------------------------------------------------------------------
# Assemble the Linux x86_64 XTPlayer AV1 codec pack (drop-in for existing
# releases; also the source for baking AV1 into new AppImages).
#
# Usage:   helper_scripts/make-codec-pack.sh [appVersion]
# Env:     CODEC_PREFIX  codec build prefix (default: $HOME/git/ffmpeg-n9.0.1-av1)
#
# Output:  codec-packs/XTPlayer-codec-pack-v<ver>-linux-x86_64.tar.gz
# ---------------------------------------------------------------------------
set -e

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
XTPLAYER_ROOT="$(dirname "${SCRIPT_DIR}")"
APP_VERSION="${1:-v0.6b}"
CODEC_PREFIX="${CODEC_PREFIX:-${HOME}/git/ffmpeg-n9.0.1-av1}"
CODEC_LIB="${CODEC_PREFIX}/lib"

OUT="${XTPLAYER_ROOT}/codec-packs"
PACK_DIR="${OUT}/XTPlayer-codec-pack-${APP_VERSION}-linux-x86_64"

for f in libavcodec.so.63 libavformat.so.63 libavutil.so.61 libswscale.so.10 libswresample.so.7; do
    [ -e "${CODEC_LIB}/${f}" ] || { echo "ERROR: ${CODEC_LIB}/${f} not found. Run helper_scripts/build-ffmpeg-av1.sh first."; exit 1; }
done

rm -rf "${PACK_DIR}"
mkdir -p "${PACK_DIR}/lib"

# FFmpeg 9.0.1 libs (drop-in replacement for the Qt-bundled ones)
cp -P "${CODEC_LIB}"/libavcodec.so.63*    "${PACK_DIR}/lib/"
cp -P "${CODEC_LIB}"/libavformat.so.63*   "${PACK_DIR}/lib/"
cp -P "${CODEC_LIB}"/libavutil.so.61*     "${PACK_DIR}/lib/"
cp -P "${CODEC_LIB}"/libswscale.so.10*    "${PACK_DIR}/lib/"
cp -P "${CODEC_LIB}"/libswresample.so.7*  "${PACK_DIR}/lib/"

# AV1 decoders (from the system; must be present on the target machine
# unless bundled here)
for extra in libdav1d.so.7* libaom.so.3*; do
    for src in /usr/lib/x86_64-linux-gnu/${extra} /usr/local/lib/${extra}; do
        [ -e "${src}" ] && cp -P "${src}" "${PACK_DIR}/lib/"
    done
done
[ -e "${PACK_DIR}/lib/libdav1d.so.7" ] || { echo "ERROR: libdav1d.so.7 not found on this system."; exit 1; }

cp "${SCRIPT_DIR}/../xtp-av1-launcher.sh" "${PACK_DIR}/"
cp "${SCRIPT_DIR}/codec-pack-README.md" "${PACK_DIR}/README.md"
chmod +x "${PACK_DIR}/xtp-av1-launcher.sh"

# Verify the packed libavcodec actually has dav1d
nm -D "${PACK_DIR}/lib/libavcodec.so.63.1.101" | grep -q dav1d || { echo "ERROR: packed libavcodec has no dav1d symbols"; exit 1; }

cd "${OUT}"
tar czf "XTPlayer-codec-pack-${APP_VERSION}-linux-x86_64.tar.gz" "$(basename "${PACK_DIR}")"
rm -rf "${PACK_DIR}"
echo "OK: ${OUT}/XTPlayer-codec-pack-${APP_VERSION}-linux-x86_64.tar.gz"
