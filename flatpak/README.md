# XTPlayer Flatpak

Flatpak packaging for XTPlayer on top of **org.kde.Platform 6.12-beta**
(Qt 6.12, whose `libffmpegmediaplugin.so` links FFmpeg 9.x /
`libavcodec.so.63`).

## Why a custom runtime build?

`org.kde.Platform` 6.12 is not published on Flathub yet (KDE's `qt6.12`
branch is `6.12-beta`). We build it once per arch with
`build-kde-runtime.sh` and ship it **in the same flatpak repo as the app**,
so installing is a single remote + a single `flatpak install`.

When KDE publishes 6.12 on Flathub, the manifest's `runtime`/`sdk` lines
can be pointed at the published branch and the custom runtime build retired.

## What's in the app

- **XTPlayer + XTEngine** — built from source (qmake), sibling layout
- **Custom FFmpeg n9.0.1** — dav1d + aom (AV1); `--enable-v4l2-m2m` on
  aarch64 (RK3588 hw decode path, needs the rkmpp libv4l plugin at runtime
  to actually offload — without it, dav1d software decode),
  `--enable-vaapi --enable-vdpau --enable-vulkan` on x86_64. Installed to
  `/app/lib`, which shadows the runtime for the Qt FFmpeg plugin's
  `libavcodec.so.63` dependency
- **qtgamepadlegacy** — evdev gamepad support (LGPLv3/GPL); needs
  `--device=all` + `--filesystem=/run/udev:ro` (RetroArch's pattern)
- **Angular web bundle** — CI-built, injected at `XTEngine/src/www`
- **Media backend** — FFmpeg by default
  (`QT_MULTIMEDIA_PREFERRED_PLUGINS=ffmpeg`); the runtime's GStreamer
  backend (with codecs) is present as a fallback

## Building locally

```bash
# 1. Build the runtime once (per arch) — ~1-3 h, cached in ./kde-runtime-build
flatpak/build-kde-runtime.sh x86_64          # and/or: aarch64

# 2. Build the app (uses the git sources in the manifest; for a dev build
#    against local checkouts, patch the manifest first):
bash flatpak/patch-manifest.sh flatpak/manifest.json xtp-web.tar.xz manifest-patched.json
flatpak-builder --repo=kde-runtime-build/repo --force-clean build manifest-patched.json

# 3. Install from the repo
flatpak --user install --repo=kde-runtime-build/repo io.github.jcfain.XTPlayer
```

`xtp-web.tar.xz` is the Angular bundle (`src/wwwsrc/dist/xtp-web/browser`
tarred) — CI builds it in the `web` job.

## Installing (from a release asset)

CI publishes one combined repo tarball per arch (runtime + app):

```bash
flatpak remote-add --if-not-exists xtp \
    https://github.com/jcfain/XTPlayer/releases/download/<tag>/xtp-flatpak-<arch>-<tag>.tar.xz
flatpak install xtp io.github.jcfain.XTPlayer
```

## Headless service (SteamOS / any systemd host)

```bash
mkdir -p ~/.config/systemd/user
cp flatpak/xtplayer.service ~/.config/systemd/user/
systemctl --user daemon-reload
systemctl --user enable --now xtplayer
```

Runs `flatpak run io.github.jcfain.XTPlayer -s` (QCoreApplication, no
display needed). The `XTP_SYSTEMD=1` env var makes the app's in-app
"restart" exit and let `Restart=always` respawn it, keeping systemd's
process tracking consistent.

## CI (`.github/workflows/flatpak.yml`)

Reusable workflow — called by `nightly.yml` (Development, `version: nightly`)
and `release.yml` (main, `version: <tag>`), the same pattern as `build.yml`.
Also runnable standalone via "Run workflow".

- `web` — Angular bundle (artifact `xtp-web-flatpak`; named differently from
  `build.yml`'s `xtp-web` so both can run in the same workflow run)
- `runtime-{x86_64,aarch64}` — `build-kde-runtime.sh`, cached in
  `kde-runtime-build/` so re-runs are incremental
- `flatpak-{x86_64,aarch64}` — x86_64 on `ubuntu-24.04`, **aarch64 native
  on `ubuntu-24.04-arm`** (no qemu/binfmt); builds the app into the runtime
  repo and publishes the combined tarball

The nightly/release publish jobs flatten all artifacts into the release, so
the `xtp-flatpak-<arch>-<version>.tar.xz` repos are published alongside the
AppImages/zips/DMGs (the internal `xtp-runtime-*` repos and web bundle are
excluded). A flatpak failure does not block the release — publish only
requires the `build` job to succeed.

## Release flow: web bundle sha256

The committed `xtp-web` source points at the release asset
`xtp-web-<tag>.tar.xz` (no sha256 until the asset exists). For a release:

1. Upload `xtp-web-<tag>.tar.xz` to the release
2. Add its sha256 to the manifest's `xtp-web` source
3. Bump `version` in the manifest + the `tag` fields of the `xtplayer`
   sources to `<tag>`
4. Tag — CI builds from the committed manifest (no patching)

## Settings / data location

Per-user data lives in `~/.var/app/io.github.jcfain.XTPlayer/` (standard
flatpak redirect of the XDG paths the app already uses via
`QStandardPaths`). The portable `settings.json` next to the executable is
an AppImage/exe feature — `/app/bin` is read-only in the sandbox, so the
native QSettings location is used inside the Flatpak.
