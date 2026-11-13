#!/usr/bin/env python3
"""Report CJK coverage of the bundled NextUI-CN fonts.

Checks each of font1/font2/font3 against:
  - ASCII printable (U+0020–U+007E)
  - GB2312 level-1 (3755 chars) and level-2 (3008 chars)
  - Every character in workspace/i18n/locales/zh_CN.lang values
  - Chinese / punctuation used in install/update boot.sh screens
"""
from __future__ import annotations

import sys
from pathlib import Path

from fontTools.ttLib import TTFont

ROOT = Path(__file__).resolve().parents[2]
RES = ROOT / "skeleton/SYSTEM/res"
LANG = ROOT / "workspace/i18n/locales/zh_CN.lang"
BOOT_SH = [
    ROOT / "workspace/tg5040/install/boot.sh",
    ROOT / "workspace/tg5050/install/boot.sh",
    ROOT / "workspace/h700/install/boot.sh",
]
FONTS = {
    "font1.ttf": "梦源黑体 CN W10",
    "font2.ttf": "缝合像素 12px zh_hans",
    "font3.ttf": "寒蝉半圆体",
}


def gb2312_level(level: int) -> str:
    # Level 1: rows 16–55; level 2: rows 56–87; 94 cells each.
    if level == 1:
        rows = range(16, 56)
    elif level == 2:
        rows = range(56, 88)
    else:
        raise ValueError(level)
    chars = []
    for row in rows:
        for col in range(1, 95):
            try:
                chars.append(bytes([0xA0 + row, 0xA0 + col]).decode("gb2312"))
            except UnicodeDecodeError:
                pass
    return "".join(chars)


def chars_from_lang(path: Path) -> str:
    out = []
    for raw in path.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        val = line.split("=", 1)[1]
        out.append(val)
    return "".join(out)


def chars_from_boot(paths: list[Path]) -> str:
    out = []
    for p in paths:
        if not p.is_file():
            continue
        text = p.read_text(encoding="utf-8")
        out.append("".join(ch for ch in text if ord(ch) > 127))
    return "".join(out)


def unique(s: str) -> str:
    seen = set()
    chars = []
    for ch in s:
        if ch not in seen:
            seen.add(ch)
            chars.append(ch)
    return "".join(chars)


def cmap_of(path: Path) -> set[int]:
    font = TTFont(path)
    cmap = font.getBestCmap() or {}
    font.close()
    return set(cmap)


def missing(sample: str, cmap: set[int]) -> list[str]:
    return [ch for ch in unique(sample) if ord(ch) not in cmap]


def main() -> int:
    ascii_chars = "".join(chr(c) for c in range(0x20, 0x7F))
    gb1 = gb2312_level(1)
    gb2 = gb2312_level(2)
    lang = chars_from_lang(LANG) if LANG.is_file() else ""
    boot = chars_from_boot(BOOT_SH)
    sets = [
        ("ASCII printable", ascii_chars),
        ("GB2312 level-1", gb1),
        ("GB2312 level-2", gb2),
        ("zh_CN.lang values", lang),
        ("boot.sh CJK/text", boot),
    ]
    print(f"{'font':<10} {'label':<22} {'size':>8} {'cmap':>7} {'cjk':>6}")
    rc = 0
    reports = []
    for fname, label in FONTS.items():
        path = RES / fname
        if not path.is_file():
            print(f"{fname:<10} MISSING")
            rc = 1
            continue
        cmap = cmap_of(path)
        cjk = sum(1 for cp in cmap if 0x4E00 <= cp <= 0x9FFF)
        print(f"{fname:<10} {label:<22} {path.stat().st_size/1048576:7.2f}M {len(cmap):7d} {cjk:6d}")
        reports.append((fname, label, cmap))

    print()
    for set_name, sample in sets:
        n = len(unique(sample))
        print(f"== {set_name} ({n} unique chars) ==")
        # Required sets: ASCII, GB2312 level-1, translations, boot text.
        required = set_name in ("ASCII printable", "GB2312 level-1", "zh_CN.lang values", "boot.sh CJK/text")
        for fname, label, cmap in reports:
            miss = missing(sample, cmap)
            status = "OK" if not miss else f"MISSING {len(miss)}"
            print(f"  {fname}: {n - len(miss)}/{n}  {status}")
            if miss:
                if required:
                    rc = 1
                show = "".join(miss[:80])
                extra = "" if len(miss) <= 80 else f" … +{len(miss)-80} more"
                print(f"    {show}{extra}")
        print()
    return rc


if __name__ == "__main__":
    sys.exit(main())
