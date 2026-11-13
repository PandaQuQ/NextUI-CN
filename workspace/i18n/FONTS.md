# Fonts (downstream overlay)

This repo is a **downstream localization fork**. To keep rebases painless, we treat fonts as an **overlay asset** (no upstream code changes required).

## Where fonts live

NextUI loads UI fonts from `/.system/res/` (see `RES_PATH` and `CFG_setFontId()` in `workspace/all/common/config.c`).

In this repo we supply those files via the skeleton overlay:

- `skeleton/SYSTEM/res/font1.ttf`
- `skeleton/SYSTEM/res/font2.ttf`
- `skeleton/SYSTEM/res/font3.ttf`

Anything under `skeleton/` is intended to be copied into the final SD card layout, so it survives upstream rebases.

## Font mapping (current convention)

We use three fonts from their respective GitHub releases:

### Font 1: Dream Han Sans CN W10 (梦源黑体)
Source: `https://github.com/Pal3love/dream-han-cjk/releases`

Mapping:
- `DreamHanSansCN-W10.ttf` → `skeleton/SYSTEM/res/font1.ttf` (默认字体)

### Font 2: Fusion Pixel (缝合像素)
Source: `https://github.com/TakWolf/fusion-pixel-font/releases`

Mapping:
- `fusion-pixel-12px-proportional-zh_hans.ttf` → `skeleton/SYSTEM/res/font2.ttf` (可选字体)
- `fusion-pixel-12px-proportional-zh_hans.ttf` → `skeleton/SYSTEM/res/BPreplayBold-unhinted.ttf`
- `fusion-pixel-12px-proportional-zh_hans.ttf` → `skeleton/SYSTEM/res/BPreplayBold.ttf` (alias)

### Font 3: ChillRound (寒蝉半圆体)
Source: `https://github.com/Warren2060/ChillRound/releases`

Mapping:
- `ChillRoundM.ttf` → `skeleton/SYSTEM/res/font3.ttf` (可选字体)

Notes:
- `BPreplayBold-unhinted.ttf` and `BPreplayBold.ttf` are used by some UI elements as a bold face.
- `font1.ttf` is the default UI font (ID 0).
- `font2.ttf` is the second UI font (ID 1).
- `font3.ttf` is the third UI font (ID 2).
- All three fonts support Simplified Chinese characters optimally.

## Re-sync fonts (Windows PowerShell)

From repo root:

```powershell
pwsh -File workspace/i18n/font_sync.ps1
```

Force re-download:

```powershell
pwsh -File workspace/i18n/font_sync.ps1 -Force
```

### Optional: subset to reduce size

If you have Python + `fonttools` installed, you can subset `font1.ttf` using the characters found in `workspace/i18n/locales/zh_CN.lang`:

```powershell
pwsh -File workspace/i18n/font_sync.ps1 -Subset
```

This keeps the UI package smaller, and still covers all translated strings.

## Custom user fonts (SD:/Fonts)

Users can add their own UI fonts without touching system files:

- Folder: `SDCARD_PATH/Fonts` (`/mnt/SDCARD/Fonts` on device), shipped empty with a `README.txt` from `skeleton/BASE/Fonts/`. It is outside `.system`, so updates never overwrite it.
- Any `*.ttf` / `*.otf` there (hidden and `._` files ignored, max 64, sorted by name) shows up in Settings → 外观 → 字体 as `自定义：<name>` (`settings.font_name.custom_fmt`).
- Persisted in `minuisettings.txt` as `font=custom:<filename>`. Built-in fonts keep the old format (`font=1` / `font=0` / `font=font3.ttf`), and legacy `fontOrder=2` ids are still migrated, so older settings files keep working.
- Path resolution: `CFG_resolveFontPath()` in `workspace/all/common/config.c` (`custom:` → `SDCARD_PATH/Fonts/<name>`, anything else → `RES_PATH/<name>`; names containing `/` are rejected).
- Fallback: if the file is missing, `CFG_setFontFile()` logs and switches to `font1.ttf`. If it exists but `TTF_OpenFont` fails, `GFX_loadSystemFont()` logs, loads `font1.ttf` instead and the setting is reverted to the default. Missing glyphs in a font just render as boxes (no crash).
- All UI programs (nextui, settings, minarch, minput, clock, …) load the font through `GFX_init()` → `CFG_init()`, so they all honour the choice. `nextval`'s `fontpath` reports the resolved path.
