#!/bin/bash
# ---------------------------------------------------------------------------
# XTPlayer launcher with optional AV1 codec pack.
#
# If the codec pack (FFmpeg n9.0.1 + dav1d) exists in CODEC_LIB_DIR, it is
# preferred over the FFmpeg libraries bundled with Qt (LD_LIBRARY_PATH takes
# precedence over the plugin's RUNPATH). If it does not exist, XTPlayer runs
# exactly as before - automatic fallback, no AV1.
#
# Usage:
#   xtp-av1-launcher.sh [path/to/XTPlayer] [args...]
#
# Env:
#   CODEC_LIB_DIR  codec pack lib dir (default: $HOME/git/ffmpeg-n9.0.1-av1/lib)
# ---------------------------------------------------------------------------
CODEC_LIB_DIR="${CODEC_LIB_DIR:-${HOME}/git/ffmpeg-n9.0.1-av1/lib}"
APP="${1:-${HOME}/git/XTPlayer/build-release/release/XTPlayer}"
shift $([ $# -gt 0 ] && echo 1 || echo 0)

if [ -f "${CODEC_LIB_DIR}/libavcodec.so.63" ]; then
    export LD_LIBRARY_PATH="${CODEC_LIB_DIR}${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
    echo "XTPlayer: using AV1 codec pack from ${CODEC_LIB_DIR}" >&2
fi

exec "${APP}" "$@"
