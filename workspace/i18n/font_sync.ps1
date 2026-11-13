<#
.SYNOPSIS
  Sync the three bundled NextUI-CN UI fonts into skeleton/SYSTEM/res.

.DESCRIPTION
  Downloads the pinned upstream releases and copies:

    DreamHanSansCN-W10.ttf                         -> font1.ttf          (梦源黑体 CN W10, default)
    fusion-pixel-12px-proportional-zh_hans.ttf     -> font2.ttf          (缝合像素)
    fusion-pixel-12px-proportional-zh_hans.ttf     -> BPreplayBold-unhinted.ttf, BPreplayBold.ttf
    ChillRoundM.ttf                                -> font3.ttf          (寒蝉半圆体)

  Versions match workspace/i18n/font_sync.sh and the CI/release workflows.

  On Linux/macOS prefer:  bash workspace/i18n/font_sync.sh
  Force re-download:      $env:FORCE=1; or bash with FORCE=1

.NOTES
  Fonts are also committed in skeleton/SYSTEM/res so local and desktop builds
  do not depend on this script. Re-run it only to refresh from upstream.
#>

param(
  [string]$DreamVersion = "dream-3.02-sans-2.004-serif-2.003",
  [string]$FusionVersion = "2026.01.04",
  [string]$ChillRoundVersion = "v1.800",
  [string]$OutDir = "$PSScriptRoot/.cache/fonts",
  [switch]$Force
)

$ErrorActionPreference = 'Stop'

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$dstRes = Join-Path $repoRoot "skeleton\SYSTEM\res"
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
New-Item -ItemType Directory -Force -Path $dstRes | Out-Null

function Get-ReleaseZip([string]$Url, [string]$ZipPath) {
  if ($Force -or -not (Test-Path $ZipPath)) {
    Write-Host "[font_sync] Downloading: $Url"
    Invoke-WebRequest -Uri $Url -OutFile $ZipPath
  } else {
    Write-Host "[font_sync] Reusing $ZipPath"
  }
}

function Expand-Clean([string]$ZipPath, [string]$Dest) {
  if (Test-Path $Dest) { Remove-Item -Recurse -Force $Dest }
  New-Item -ItemType Directory -Force -Path $Dest | Out-Null
  Expand-Archive -Path $ZipPath -DestinationPath $Dest -Force
}

# Dream Han Sans CN W10
$dreamZip = Join-Path $OutDir "DreamHanSansCN.zip"
Get-ReleaseZip "https://github.com/Pal3love/dream-han-cjk/releases/download/$DreamVersion/DreamHanSansCN.zip" $dreamZip
$dreamExt = Join-Path $OutDir "dream-extract"
Expand-Clean $dreamZip $dreamExt
$dreamTtf = Get-ChildItem -Path $dreamExt -Recurse -File -Filter "DreamHanSansCN-W10.ttf" | Select-Object -First 1
if (-not $dreamTtf) { throw "DreamHanSansCN-W10.ttf not found in $dreamZip" }
Copy-Item -Force $dreamTtf.FullName (Join-Path $dstRes "font1.ttf")
Write-Host "[font_sync] font1.ttf <- $($dreamTtf.Name)"

# Fusion Pixel
$fusionZip = Join-Path $OutDir "fusion-pixel-font-12px-proportional-ttf-v$FusionVersion.zip"
Get-ReleaseZip "https://github.com/TakWolf/fusion-pixel-font/releases/download/$FusionVersion/fusion-pixel-font-12px-proportional-ttf-v$FusionVersion.zip" $fusionZip
$fusionExt = Join-Path $OutDir "fusion-extract"
Expand-Clean $fusionZip $fusionExt
$fusionTtf = Get-ChildItem -Path $fusionExt -Recurse -File -Filter "fusion-pixel-12px-proportional-zh_hans.ttf" | Select-Object -First 1
if (-not $fusionTtf) { throw "fusion-pixel-12px-proportional-zh_hans.ttf not found in $fusionZip" }
Copy-Item -Force $fusionTtf.FullName (Join-Path $dstRes "font2.ttf")
Copy-Item -Force $fusionTtf.FullName (Join-Path $dstRes "BPreplayBold-unhinted.ttf")
Copy-Item -Force $fusionTtf.FullName (Join-Path $dstRes "BPreplayBold.ttf")
Write-Host "[font_sync] font2.ttf <- $($fusionTtf.Name)"

# ChillRound M
$chillZip = Join-Path $OutDir "ChillRoundM_$ChillRoundVersion.zip"
Get-ReleaseZip "https://github.com/Warren2060/ChillRound/releases/download/$ChillRoundVersion/ChillRoundM_$ChillRoundVersion.zip" $chillZip
$chillExt = Join-Path $OutDir "chill-extract"
Expand-Clean $chillZip $chillExt
$chillFont = Get-ChildItem -Path $chillExt -Recurse -File -Filter "ChillRoundM.ttf" | Select-Object -First 1
if (-not $chillFont) {
  $chillFont = Get-ChildItem -Path $chillExt -Recurse -File -Filter "ChillRoundM.otf" | Select-Object -First 1
}
if (-not $chillFont) { throw "ChillRoundM.ttf/.otf not found in $chillZip" }
Copy-Item -Force $chillFont.FullName (Join-Path $dstRes "font3.ttf")
Write-Host "[font_sync] font3.ttf <- $($chillFont.Name)"

Write-Host "[font_sync] Done."
Get-Item (Join-Path $dstRes "font1.ttf"), (Join-Path $dstRes "font2.ttf"), (Join-Path $dstRes "font3.ttf") | Format-Table Name, Length
