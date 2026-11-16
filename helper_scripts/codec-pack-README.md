# XTPlayer AV1 Codec Pack (Linux x86_64)

Adds software **AV1** video decoding (dav1d + libaom) to XTPlayer's FFmpeg
media backend. Without it, AV1 videos play audio only (no video).

## Requirements
- XTPlayer built with Qt 6.12 (FFmpeg multimedia plugin linked against FFmpeg 9.0.x / `libavcodec.so.63`)
- Linux x86_64

## Install (existing releases)

1. Extract this pack somewhere, e.g. `~/XTPlayer-codec-pack/`
2. Launch XTPlayer through the included launcher:

   ```
   ~/XTPlayer-codec-pack/xtp-av1-launcher.sh /path/to/XTPlayer
   ```

   The launcher points the loader at the pack's `lib/` directory only when it
   exists; if you delete the pack, XTPlayer runs exactly as before (automatic
   fallback).

   To make it permanent, create a shell alias or a `.desktop` entry that calls
   the launcher.

## How it works

The Qt FFmpeg plugin (`libffmpegmediaplugin.so`) resolves its FFmpeg libraries
via `LD_LIBRARY_PATH` first, then its built-in `RUNPATH` (the Qt lib dir).
The launcher sets `LD_LIBRARY_PATH` to the pack's `lib/` dir, so the pack's
FFmpeg 9.0.1 build (with AV1 decoders) is used instead of the AV1-less copy
bundled with Qt.

## Notes

- **Headless / no-GPU sessions:** the FFmpeg backend tries VAAPI hardware
  decoding first. If that fails, force pure software decoding with:
  `export QT_FFMPEG_DECODING_HW_DEVICE_TYPES=`
- `libdav1d.so.7` is bundled; `libaom.so.3` is bundled as a fallback decoder.
  If your system already has newer versions, the pack's copies take precedence
  (they are self-contained).
- Verify in the XTPlayer log at startup: it should report the FFmpeg version
  and `AV1: yes` in the codec diagnostics line.
