# 关于 NextUI 的 pak

pak 本质上就是一个以 “.pak” 结尾的文件夹，其中包含一个名为 “launch.sh” 的 shell 脚本。

pak 分两类：模拟器（Emulator）和工具（Tool）。模拟器 pak 放在 Emus 目录下，工具 pak 放在 Tools 目录下。这两个目录都位于 SD 卡根目录。

额外（第三方）pak 不要放进 SD 卡根目录下的隐藏目录 “.system”。因为用户每次更新 NextUI 时，这个目录都会被删除并重新写入。

pak 是按平台区分的。在 Emus 和 Tools 目录中，你会看到（或需要创建）不同平台对应的子目录。平台目录名有几种来源：

- 有的直接用设备型号（例如 Powkiddy RGB30 的平台目录是 “rgb30”）
- 有的用设备内部代号（例如 Trimui Smart Pro 的平台目录是 “tg5040”）
- 还有的使用一个约定俗成的短名（例如 Trimui Model S 使用 “trimui”）

它们都必须是全小写。最新支持的平台目录名以 extras 包为准。

有些平台下面还会有多台具有不同特性的设备。NextUI 使用环境变量 `DEVICE` 来区分这些设备。在 tg5040 上取值为 `brick`、`brickpro` 或 `smartpro`；在 h700 上则是具体的 SKU 编号，例如 `rg40xxv`、`rg34xxsp` 或 `rgcubexx`（完整列表见 Wiki 中的 H700 Pak 移植页面）。pak 可以选择使用或忽略这个环境变量。

# 模拟器 pak 的类型

模拟器 pak 大体分为三种类型，选择哪一种取决于你的目标，以及你希望和 NextUI 集成到什么程度。

第一种：复用 NextUI 基础安装中自带的 libretro核心。优点是核心已知可用，同时你还能自定义默认选项并将用户配置隔离开。例子：额外的 GG.pak 使用默认的 picodrive核心。

第二种：在 pak 内自带一个 libretro核心。这样你可以支持全新的系统，同时继续享受 NextUI 的标准功能，比如从菜单恢复、快速存档与自动恢复、统一的游戏内菜单/行为/选项等。例子：额外的 MGBA.pak 会捆绑自己的 mgba核心。

第三种：启动 pak 内自带的独立模拟器（standalone emulator）。这可能比 libretro核心榨出更好的性能，但缺点是无法与 NextUI 集成：不能从菜单恢复、没有快速存档与自动恢复、游戏内菜单/行为/选项不一致。在某些情况下，MENU（以及如果存在的话 POWER）按键可能无法按预期工作，甚至完全无效。这种 pak 应作为最后手段。例子：社区开发的 NDS.pak，只支持 NextUI 的少数平台。

无论哪种类型，都请明确告知你的用户：我（@shauninman）无法为第三方 pak 提供支持。如果我把某个主机或核心排除在 NextUI 的基础/额外包之外，通常是有原因的：例如核心的集成质量不足（比如街机核心往往要求 ROM 文件名很“玄学”，且只有某些 ROM 集才能工作）、bug 太多（例如无法稳定从即时存档恢复）、在特定设备上性能太差，或只是我本人对某个平台没有兴趣/不熟悉。

# 为模拟器 pak 命名

NextUI 会根据 ROM 所在父目录名称末尾括号里的 tag，将 ROM 映射到对应 pak（例如 “/Roms/Game Boy (GB)/Dr. Mario (World).gb” 会启动 “GB.pak”）。tag 必须全大写。

选择 tag 时，建议从其他前端（如 RetroArch 或 EmulationStation）常用的缩写开始（例如 FC 表示 Famicom/Nintendo，MD 表示 MegaDrive/Genesis）。如果这个 tag 已被其它 pak 占用：

- 如果核心名很短，可以直接用核心名（例如 MGBA）
- 或使用缩写（例如 PKM 表示 pokemini）
- 或对核心名做截断（例如 SUPA 表示 mednafen_supafaust）

# 启动核心

下面是一个 “launch.sh” 示例：

	#!/bin/sh
	
	EMU_EXE=picodrive
	
	###############################
	
	EMU_TAG=$(basename "$(dirname "$0")" .pak)
	ROM="$1"
	mkdir -p "$BIOS_PATH/$EMU_TAG"
	mkdir -p "$SAVES_PATH/$EMU_TAG"
	mkdir -p "$CHEATS_PATH/$EMU_TAG"
	HOME="$USERDATA_PATH"
	cd "$HOME"
	minarch.elf "$CORES_PATH/${EMU_EXE}_libretro.so" "$ROM" &> "$LOGS_PATH/$EMU_TAG.txt"

这会使用 NextUI 基础安装中自带的 “picodrive_libretro.so”核心来运行指定 ROM。

如果要用其他核心，只需要把 `EMU_EXE` 改成另一个核心名（不包含 “_libretro.so” 后缀）。如果该核心是随 pak 一起提供的，请在 `EMU_EXE` 这一行之后加上：

	CORES_PATH=$(dirname "$0")

“###############################” 下面的部分不用改。剩余内容都是样板逻辑：从文件夹名解析 pak 的 tag、创建对应的 bios/save 目录、把 `HOME` 环境变量设为 “/.userdata/[platform]/”、启动游戏，并把 minarch 与核心的输出写入 “/.userdata/[platform]/logs/[TAG].txt”。

就这些！你可以自由尝试使用原厂固件里的核心、其它兼容设备的核心，或自行编译。

另外，如果你在为 Anbernic 的 RG*XX 系列做 pak，需要把最后一行末尾从 ` &> "$LOGS_PATH/$EMU_TAG.txt"` 改成 ` > "$LOGS_PATH/$EMU_TAG.txt" 2>&1`，因为它默认的 shell 行为很奇怪。

# 选项默认值与按键绑定

把你新做的 pak 和一些 ROM 拷到 SD 卡，启动一个游戏。按 MENU 键，选择 Options，分别配置 Frontend、Emulator、Controls。

NextUI 的惯例是：只默认绑定原机实体手柄上“确实存在”的控制（例如不默认绑 turbo 键，也不默认绑核心特有的功能如调色板切换、磁盘切换）。玩家如果想折腾可以自行设置，Shortcuts 也是同理。

最后选择 Save Changes > Save for Console。然后退出，把 SD 卡插回电脑。

在 SD 卡根目录的隐藏文件夹 “.userdata” 中，你会看到平台目录；进入对应平台目录后，会有一个 “[TAG]-[core]” 目录。把其中的 “minarch.cfg” 复制到 pak 目录，并改名为 “default.cfg”。打开 “default.cfg”，删除你没有自定义过的选项。

任何带前缀 “-” 的选项名都会被设置并隐藏。这可以用来关闭某些功能（例如超频）——可能在特定平台不可用，或在特定平台表现很差（例如 upscale）。

在文件接近底部的位置，你会看到按键绑定。下面是来自 “MGBA.pak” 的示例：

	bind Up = UP
	bind Down = DOWN
	bind Left = LEFT
	bind Right = RIGHT
	bind Select = SELECT
	bind Start = START
	bind A Button = A
	bind B Button = B
	bind A Turbo = NONE:X
	bind B Turbo = NONE:Y
	bind L Button = L1
	bind R Button = R1
	bind L Turbo = NONE:L2
	bind R Turbo = NONE:R2
	bind More Sun = NONE:L3
	bind Less Sun = NONE:R3

`bind ` 到 `=` 之间的内容，是 Controls 菜单里显示的按键标签。我通常会把这些标签规范化（例如用 “Up” 而不是 “D-pad up”，用 “A Button” 而不是只写 “A”）。

`=` 到可选的 `:` 之间，是按键映射（mapping）。映射必须全大写。肩键和摇杆按键必须带数字（例如用 “L1” 而不是只写 “L”）。如果默认不绑定某个按键，使用 “NONE”。

当你要自定义或移除一个绑定时，应当在 `:` 之后保留核心默认定义的映射。例如上面的例子里，我通过把：

	bind More Sun = L3

改成：

	bind More Sun = NONE:L3

来移除了默认的 “More Sun” 绑定。

# 亮度与音量

有些二进制程序每次启动都会强制重置亮度（例如 40xxH 原厂固件里的 DinguxCommander）或音量（例如 ppssppSDL 几乎到处都这样）。为了与 NextUI 的全局设置保持同步，可以使用 syncsettings.elf：它会等待 1 秒，然后恢复 NextUI 当前的亮度与音量。

大多数情况下，你只需要在启动目标程序之前把它作为守护进程启动：

	syncsettings.elf &
	./DinguxCommander

但如果某个程序初始化时间超过 1 秒，你可能需要让它在程序运行期间持续循环：

	while :; do
	    syncsettings.elf
	done &
	LOOP_PID=$!
	
	./PPSSPPSDL --pause-menu-exit "$ROM_PATH"
	
	kill $LOOP_PID

# 注意事项

NextUI 目前只支持 XRGB8888 和 RGB565 像素格式，并且没有实现 OpenGL 的 libretro API。也许可以用原厂固件的 RetroArch 替代 NextUI 的 minarch 来运行某些 核心，但这就留给读者自行研究了。
