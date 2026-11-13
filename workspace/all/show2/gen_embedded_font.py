#!/usr/bin/env python3
"""Subset 寒蝉全圆体 Bold into a small install-splash font and emit embedded_font_cjk.h."""
from pathlib import Path
from fontTools.subset import Options, Subsetter
from fontTools.ttLib import TTFont

ROOT = Path(__file__).resolve().parent
# Prefer the NextCommander-CN checkout; fall back to a sibling path.
CANDIDATES = [
    Path("/workspace/NextCommander-CN/res/ChillRoundFBold.ttf"),
    ROOT.parent.parent.parent.parent / "NextCommander-CN/res/ChillRoundFBold.ttf",
]
SRC = next((p for p in CANDIDATES if p.is_file()), None)
if SRC is None:
    raise SystemExit("ChillRoundFBold.ttf not found; clone PandaQuQ/NextCommander-CN")

DST_TTF = ROOT / "ChillRound-Bold-install.ttf"
DST_H = ROOT / "embedded_font_cjk.h"

chars = set(chr(c) for c in range(0x20, 0x7F))
chars.update("…—–·、。，．！？：；「」『』【】《》（）")
chars.update("安装中升级解压正在提取完成系统文件请稍候开机关机重启")
chars.update("复制删除移动取消确定返回退出错误成功等待加载准备清理运行脚本完成")
text = "".join(sorted(chars))

opt = Options()
opt.layout_features = ["*"]
opt.name_IDs = ["*"]
opt.name_languages = ["*"]
opt.notdef_outline = True
opt.recommended_glyphs = True
opt.hinting = True
subsetter = Subsetter(options=opt)
font = TTFont(SRC)
subsetter.populate(text=text)
subsetter.subset(font)
font.save(DST_TTF)
font.close()

data = DST_TTF.read_bytes()
lines = [
    "/* Auto-generated subset of 寒蝉全圆体 Bold for show2 install/update text.",
    " * Regenerate with workspace/all/show2/gen_embedded_font.py",
    " */",
    "unsigned char embedded_cjk_font_ttf[] = {",
]
row = []
for b in data:
    row.append(f"0x{b:02x}")
    if len(row) == 12:
        lines.append("  " + ", ".join(row) + ",")
        row = []
if row:
    lines.append("  " + ", ".join(row) + ",")
lines.append("};")
lines.append(f"unsigned int embedded_cjk_font_ttf_len = {len(data)};")
DST_H.write_text("\n".join(lines) + "\n")
print(f"wrote {DST_TTF} ({DST_TTF.stat().st_size} bytes)")
print(f"wrote {DST_H}")
