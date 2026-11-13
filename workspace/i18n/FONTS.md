# Fonts (downstream overlay)

This repo is a **downstream localization fork**. UI fonts live under
`skeleton/SYSTEM/res/` and are copied onto the SD card as `/.system/res/`
(see `RES_PATH` and `CFG_setFontFile()` / `CFG_resolveFontPath()` in
`workspace/all/common/config.c`).

The three built-in files are committed in the tree so local, desktop, and
CI packages all use the same fonts. The CI and release workflows still
re-run `workspace/i18n/font_sync.sh` before `make setup` so a stale checkout
is overwritten with the pinned upstream files.

## Bundled fonts

| Setting | File | Typeface | Upstream | Version | License | Size |
| --- | --- | --- | --- | --- | --- | --- |
| default (ID 0) | `font1.ttf` | 梦源黑体 CN W10 | [Pal3love/dream-han-cjk](https://github.com/Pal3love/dream-han-cjk) `DreamHanSansCN-W10.ttf` | dream-3.02-sans-2.004-serif-2.003 | SIL OFL 1.1 | 10.95 MiB |
| option (ID 1) | `font2.ttf` | 缝合像素 12px zh_hans | [TakWolf/fusion-pixel-font](https://github.com/TakWolf/fusion-pixel-font) `fusion-pixel-12px-proportional-zh_hans.ttf` | 2026.01.04 | SIL OFL 1.1 | 6.57 MiB |
| option (ID 2) | `font3.ttf` | 寒蝉半圆体 | [Warren2060/ChillRound](https://github.com/Warren2060/ChillRound) `ChillRoundM.ttf` | v1.800 | SIL OFL 1.1 | 6.25 MiB |

License texts are kept next to this file in `workspace/i18n/fonts/`.

W10 is the intended default weight (lighter than W16; see commit `012f5014`).
Settings labels come from `settings.font_name.font1/font2/font3` in
`zh_CN.lang` (梦源黑体 / 缝合像素 / 寒蝉半圆体). `font3.ttf` is only listed
if the file exists; with it in the skeleton it appears on every platform.

The same Fusion Pixel file is also copied to `BPreplayBold-unhinted.ttf` and
`BPreplayBold.ttf` (legacy aliases). Upstream `BPreplayBold-unhinted.otf` files
are left in place; the UI loads `font1.ttf` / `font2.ttf` / `font3.ttf`.

These are the official full fonts (not subsetted). Coverage of GB2312 level-1,
all characters in `zh_CN.lang`, and the install/update boot strings is complete
for all three. Fusion Pixel is missing 156 uncommon GB2312 level-2 characters;
Dream Han and ChillRound cover level-2 fully. Re-check with:

```
python3 workspace/i18n/font_coverage.py
```

Total added relative to the old in-tree placeholders (Rounded M+ 3.6 MiB +
BPreplay 166 KiB, no font3) is about 20 MiB unique font data in `MinUI.zip`.
CI-built releases already injected these same full fonts at package time, so
device zip size does not grow versus a correct CI package; local/desktop
builds now match.

## Re-sync fonts

From the repo root (Linux/macOS, also used by CI):

```bash
bash workspace/i18n/font_sync.sh
FORCE=1 bash workspace/i18n/font_sync.sh   # re-download
```

Windows PowerShell:

```powershell
pwsh -File workspace/i18n/font_sync.ps1
pwsh -File workspace/i18n/font_sync.ps1 -Force
```

Downloads are cached under `workspace/i18n/.cache/` (gitignored).

## Custom user fonts (SD:/Fonts)

Users can add their own UI fonts without touching system files:

- Folder: `SDCARD_PATH/Fonts` (`/mnt/SDCARD/Fonts` on device), shipped empty with a `README.txt` from `skeleton/BASE/Fonts/`. It is outside `.system`, so updates never overwrite it.
- Any `*.ttf` / `*.otf` there that SDL_ttf can open (hidden, `._` and unloadable files are skipped and logged; max 64, sorted by name) shows up in Settings → 外观 → 字体 as `自定义：<name>` (`settings.font_name.custom_fmt`).
- Persisted in `minuisettings.txt` as `font=custom:<filename>`. Built-in fonts keep the old format (`font=1` / `font=0` / `font=font3.ttf`), and legacy `fontOrder=2` ids are still migrated, so older settings files keep working.
- Path resolution: `CFG_resolveFontPath()` in `workspace/all/common/config.c` (`custom:` → `SDCARD_PATH/Fonts/<name>`, anything else → `RES_PATH/<name>`; names containing `/` are rejected).
- Fallback: if the file is missing, `CFG_setFontFile()` logs and switches to `font1.ttf`. If it exists but `TTF_OpenFont` fails, `GFX_loadSystemFont()` logs, loads `font1.ttf` instead and the setting is reverted to the default; the Settings menu re-reads the value so it immediately shows the font actually in use. Missing glyphs in a font just render as boxes (no crash).
- All UI programs (nextui, settings, minarch, minput, clock, …) load the font through `GFX_init()` → `CFG_init()`, so they all honour the choice. `nextval`'s `fontpath` reports the resolved path.
