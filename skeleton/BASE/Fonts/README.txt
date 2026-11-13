NextUI-CN 自定义字体 / Custom Fonts
----------------------------------------

把你自己的字体文件（.ttf 或 .otf）放进这个文件夹（SD 卡根目录下的 Fonts），
然后在 设置 > 外观 > 字体 里选择“自定义：<文件名>”即可。
选择会保存在设置里（font=custom:<文件名>），重启后依然生效。

说明：
- 文件名中的下划线会显示为空格；最多列出 64 个字体（按文件名排序）。
- 损坏或无法读取的字体文件不会出现在列表中。
- 界面需要中文字形，请选用包含简体中文的字体，否则缺字会显示为方框。
- 如果字体文件被删除、改名或无法加载，系统会自动回退到默认字体（梦源黑体）。
- 本文件夹不会被系统更新覆盖。

Drop your own .ttf / .otf font files into this folder (SD card root /Fonts),
then pick "自定义：<name>" under Settings > Appearance > Font. The choice is saved
as font=custom:<filename> and persists across reboots. If the file is removed or
cannot be loaded, NextUI falls back to the default font
(unloadable files are not listed). Use a font with CJK
(Simplified Chinese) coverage, otherwise missing characters render as boxes.
This folder is never overwritten by updates.
