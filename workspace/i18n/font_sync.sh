#!/usr/bin/env bash
# Download the three bundled NextUI-CN UI fonts into skeleton/SYSTEM/res.
# Versions are pinned to match .github/workflows/ci.yaml and release.yaml.
set -euo pipefail

DREAM_VERSION="${DREAM_VERSION:-dream-3.02-sans-2.004-serif-2.003}"
FUSION_VERSION="${FUSION_VERSION:-2026.01.04}"
CHILLROUND_VERSION="${CHILLROUND_VERSION:-v1.800}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
RES="$REPO_ROOT/skeleton/SYSTEM/res"
CACHE="${FONT_SYNC_CACHE:-$SCRIPT_DIR/.cache/fonts}"
mkdir -p "$RES" "$CACHE"

download() {
  local url="$1" out="$2"
  if [[ -f "$out" && "${FORCE:-}" != "1" ]]; then
    echo "[font_sync] reusing $out"
    return
  fi
  echo "[font_sync] downloading $url"
  curl -fL --retry 5 --retry-delay 2 --connect-timeout 20 -o "$out" "$url"
}

# --- Dream Han Sans CN W10 (梦源黑体) -> font1.ttf ---
dream_zip="$CACHE/DreamHanSansCN.zip"
download "https://github.com/Pal3love/dream-han-cjk/releases/download/${DREAM_VERSION}/DreamHanSansCN.zip" "$dream_zip"
rm -rf "$CACHE/dream-extract"
mkdir -p "$CACHE/dream-extract"
unzip -q -o "$dream_zip" -d "$CACHE/dream-extract"
dream_ttf="$(find "$CACHE/dream-extract" -maxdepth 3 -type f -name 'DreamHanSansCN-W10.ttf' | head -n 1 || true)"
if [[ -z "$dream_ttf" ]]; then
  echo "Dream Han Sans CN W10 TTF not found in $dream_zip" >&2
  find "$CACHE/dream-extract" -maxdepth 3 -type f >&2 || true
  exit 1
fi
cp -f "$dream_ttf" "$RES/font1.ttf"
echo "[font_sync] font1.ttf <- $(basename "$dream_ttf")"

# --- Fusion Pixel 12px proportional zh_hans (缝合像素) -> font2.ttf + BPreplay aliases ---
fusion_zip="$CACHE/fusion-pixel-font-12px-proportional-ttf-v${FUSION_VERSION}.zip"
download "https://github.com/TakWolf/fusion-pixel-font/releases/download/${FUSION_VERSION}/fusion-pixel-font-12px-proportional-ttf-v${FUSION_VERSION}.zip" "$fusion_zip"
rm -rf "$CACHE/fusion-extract"
mkdir -p "$CACHE/fusion-extract"
unzip -q -o "$fusion_zip" -d "$CACHE/fusion-extract"
fusion_ttf="$(find "$CACHE/fusion-extract" -maxdepth 2 -type f -name 'fusion-pixel-12px-proportional-zh_hans.ttf' | head -n 1 || true)"
if [[ -z "$fusion_ttf" ]]; then
  echo "Fusion Pixel TTF not found in $fusion_zip" >&2
  find "$CACHE/fusion-extract" -maxdepth 2 -type f >&2 || true
  exit 1
fi
cp -f "$fusion_ttf" "$RES/font2.ttf"
cp -f "$fusion_ttf" "$RES/BPreplayBold-unhinted.ttf"
cp -f "$fusion_ttf" "$RES/BPreplayBold.ttf"
echo "[font_sync] font2.ttf <- $(basename "$fusion_ttf")"

# --- ChillRound M v1.800 (寒蝉半圆体) -> font3.ttf ---
chill_zip="$CACHE/ChillRoundM_${CHILLROUND_VERSION}.zip"
download "https://github.com/Warren2060/ChillRound/releases/download/${CHILLROUND_VERSION}/ChillRoundM_${CHILLROUND_VERSION}.zip" "$chill_zip"
rm -rf "$CACHE/chill-extract"
mkdir -p "$CACHE/chill-extract"
unzip -q -o "$chill_zip" -d "$CACHE/chill-extract"
chill_font="$(find "$CACHE/chill-extract" -maxdepth 3 -type f -iname 'ChillRoundM.ttf' | head -n 1 || true)"
if [[ -z "$chill_font" ]]; then
  chill_font="$(find "$CACHE/chill-extract" -maxdepth 3 -type f -iname 'ChillRoundM.otf' | head -n 1 || true)"
fi
if [[ -z "$chill_font" ]]; then
  echo "ChillRound font not found in $chill_zip" >&2
  find "$CACHE/chill-extract" -maxdepth 3 -type f >&2 || true
  exit 1
fi
cp -f "$chill_font" "$RES/font3.ttf"
echo "[font_sync] font3.ttf <- $(basename "$chill_font")"

echo "[font_sync] installed:"
ls -lh "$RES"/font1.ttf "$RES"/font2.ttf "$RES"/font3.ttf
