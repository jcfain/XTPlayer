# XTPlayer AV1 Codec Pack — Handoff (Windows & macOS)

## Background

The Qt 6.12 FFmpeg multimedia plugin (`libffmpegmediaplugin.so` /
`libffmpegmediaplugin.dll` / `libffmpegmediaplugin.dylib`) is linked against
FFmpeg **9.0.x** (`libavcodec.so.63` / `avcodec-63.dll` / `avcodec.63.dylib`).
The FFmpeg build bundled with the Qt installation has **no AV1 decoders**, so
AV1 videos play audio only (no video). The fix is a drop-in FFmpeg 9.0.1 build
with `--enable-libdav1d --enable-libaom`.

**ABI constraint:** the replacement build MUST be FFmpeg n9.0.x so the SONAMEs
match what the Qt plugin is linked against. A different FFmpeg major version
will not load.

## Linux (DONE — reference implementation)

- Build: `helper_scripts/build-ffmpeg-av1.sh` → `~/git/ffmpeg-n9.0.1-av1/lib`
- Pack: `helper_scripts/make-codec-pack.sh <version>` →
  `codec-packs/XTPlayer-codec-pack-<version>-linux-x86_64.tar.gz`
- Launcher: `xtp-av1-launcher.sh` (sets `LD_LIBRARY_PATH` to the pack's `lib/`)
- Deploy: `lin_release_deploy-appimage.sh` now bundles the codec libs into the
  AppImage (interactive prompt / CI-safe)
- Diagnostics: `MediaFormat::probeFFmpegCodecs()` (XTEngine
  `src/lib/tool/mediaformat.cpp`) logs backend + decoder availability at
  startup via `settingshandler.cpp`
- Verified: without pack → `av1: Your platform doesn't support hardware
  accelerated AV1 decoding` (0 frames); with pack → `av1 (libdav1d)`, 76/76
  frames through the real Qt plugin.

### Linux gotchas (learned the hard way)

1. **Qt's bundled FFmpeg links against OpenSSL *stubs***
   (`libQt6FFmpegStub-crypto.so.3`): they define the `OPENSSL_3.0.0` version
   tag but not the symbols. `dlopen(..., RTLD_NOW)` fails on the missing
   `RAND_bytes`; use `RTLD_LAZY` (what the plugin itself uses).
2. **Old-style `DT_RPATH` beats `LD_LIBRARY_PATH`.** The dev-build executable
   carries `RPATH=<Qt lib dir>`, so in the dev environment the plugin always
   loads the Qt-bundled FFmpeg regardless of the launcher. The deployed
   AppImage uses new-style `DT_RUNPATH` (`$ORIGIN/../lib`), which is searched
   *after* `LD_LIBRARY_PATH` — the launcher works there.
3. **Headless / no-GPU sessions:** the FFmpeg backend tries VAAPI first and
   does not fall back cleanly. Force software decoding with
   `export QT_FFMPEG_DECODING_HW_DEVICE_TYPES=`.
4. FFmpeg's *native* AV1 decoder exists in every build (so
   `avcodec_find_decoder(AV_CODEC_ID_AV1)` is always non-null) but only works
   with hardware acceleration. The real "AV1 works" signal is the presence of
   dav1d/aom in the library's dependency tree.

## Windows (MinGW) — TO DO

Target: `avcodec-63.dll` + friends, dropped **next to `XTPlayer.exe`** (the
Windows DLL search finds the exe directory first — no launcher needed; the
"fallback" behavior is automatic: no DLLs = old behavior).

### 1. Build dav1d (no Windows dev package — build from source)

```
git clone https://code.videolan.org/videolan/dav1d.git
cd dav1d && git checkout 1.5.1
meson setup build --prefix=<prefix> --libdir=lib \
    -Dasm=true -Denable_docs=false
ninja -C build && ninja -C build install
```
- Use the MinGW toolchain (`x86_64-w64-mingw32-gcc`) and `nasm`.
- Produces `dav1d.dll` (+ `dav1d.h`, `dav1d.pc`).

### 2. Build aom (optional fallback decoder)

```
git clone https://aomedia.googlesource.com/aom
cd aom && git checkout v3.15.2
cmake -B build -G "MinGW Makefiles" \
    -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON \
    -DCMAKE_INSTALL_PREFIX=<prefix>
cmake --build build -- -j && cmake --install build
```
- Produces `aom.dll` (+ headers, `aom.pc`).

### 3. Build FFmpeg n9.0.1

```
git clone https://git.ffmpeg.org/ffmpeg.git && cd ffmpeg
git checkout n9.0.1
mkdir build-win && cd build-win
../configure --prefix=<prefix> \
    --target-os=win32 \
    --cross-prefix=x86_64-w64-mingw32- \
    --arch=x86_64 \
    --pkg-config-flags="--define-prefix=<prefix>" \
    --extra-cflags="-I<prefix>/include" \
    --extra-ldflags="-L<prefix>/lib" \
    --enable-shared --enable-libdav1d --enable-libaom \
    --enable-network --disable-doc
make -j && make install
```
- dav1d/aom are BSD-licensed → the build stays **LGPL** (no `--enable-gpl`).
- Produces `avcodec-63.dll, avformat-63.dll, avutil-61.dll,
  swscale-10.dll, swresample-7.dll` (+ import libs, headers).

### 4. Pack & verify

- DLLs to ship: the five FFmpeg DLLs + `dav1d.dll` (+ `aom.dll` if built).
  Drop them into the release directory next to `XTPlayer.exe`.
- Verify: `nm -D avcodec-63.dll | grep dav1d` (imported symbols), and a
  decode test: `ffmpeg -i test.ivf -f null -` with the pack DLLs in `PATH`.
- `win_release_deploy-release.bat`: add a step that copies the codec DLLs
  into the app directory (same pattern as the Linux deploy script: prompt
  interactively, warn-and-continue in CI).
- The diagnostics code (`MediaFormat::probeFFmpegCodecs`) already handles
  Windows: it loads `avcodec-63.dll` and checks for `dav1d.dll`/`aom.dll` in
  the DLL search path.


## macOS (Clang) — TO DO

Target: `avcodec.63.dylib` + friends, copied **into the app bundle** at
`XTPlayer.app/Contents/Frameworks/`. (macOS strips `DYLD_LIBRARY_PATH` for
SIP-protected binaries, so the Linux launcher trick does not apply; the app's
`LC_RPATH` already includes `@executable_path/../Frameworks`, which is
searched before the Qt frameworks directory.)

### 1. Build dav1d (or use Homebrew's)

```
git clone https://code.videolan.org/videolan/dav1d.git
cd dav1d && git checkout 1.5.1
meson setup build --prefix=<prefix> -Dasm=true -Denable_docs=false
ninja -C build && ninja -C build install
```
- Homebrew alternative: `brew install dav1d` (then point pkg-config at the
  Cellar).

### 2. Build FFmpeg n9.0.1 (universal)

```
git clone https://git.ffmpeg.org/ffmpeg.git && cd ffmpeg
git checkout n9.0.1
mkdir build-mac && cd build-mac
../configure --prefix=<prefix> \
    --enable-universal-binaries \
    --enable-shared --enable-libdav1d --enable-libaom \
    --enable-network --disable-doc
make -j && make install
```
- Produces `avcodec.63.dylib, avformat.63.dylib, avutil.61.dylib,
  swscale.10.dylib, swresample.7.dylib` (fat arm64+x86_64).

### 3. Pack & verify

- Copy the five FFmpeg dylibs + `libdav1d.dylib` (+ `libaom.dylib`) into
  `XTPlayer.app/Contents/Frameworks/` **before** `macdeployqt` runs, so
  macdeployqt fixes up the install names and bundles them.
- `mac_release_deploy.sh`: add the same prompt/CI-safe pattern as Linux.
- Verify: `otool -L XTPlayer.app/Contents/Frameworks/avcodec.63.dylib` shows
  the dav1d dependency; `nm -gU` on the dylib for dav1d imports.
- Diagnostics: `probeFFmpegCodecs()` already handles macOS
  (`avcodec.63.dylib` + `dlsym` dependency-tree check).

## Diagnostics (all platforms, DONE)

At startup, `settingshandler.cpp` logs (FFmpeg backend):

```
XTP INFO: Using media backend: ffmpeg
XTP INFO: FFmpeg 9.0.1 from <path> - decoders: AAC, AV1, FLAC, H264, HEVC, MP3, OPUS, VP8, VP9
XTP WARN: AV1 decoding is not available in the FFmpeg backend. ...   (only if AV1 missing)
```

Missing decoders are marked `(NO)` in the list. `MediaFormat::isCodecSupported`
also falls back to the probe when `QMediaFormat` reports an empty codec list
(the FFmpeg backend case), so the "unsupported codec" UI path works again.

