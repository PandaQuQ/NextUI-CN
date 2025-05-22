# 关于 NextUI pak

Pak 其实就是一个带有“.pak”扩展名的文件夹，里面包含一个名为“launch.sh”的 shell 脚本。

Pak 分为两类：模拟器和工具。模拟器 pak 存放在 Emus 文件夹，工具 pak 存放在 Tools 文件夹。这两个文件夹都位于 SD 卡根目录。额外的 pak 不应放在 SD 卡根目录下的隐藏“.system”文件夹中，因为每次用户更新 NextUI 时，这个文件夹都会被删除并替换。

Pak 是针对不同平台的。在 Emus 和 Tools 文件夹中，你会看到（或需要创建）平台文件夹。有些平台文件夹以目标设备命名（如“rgb30”代表 Powkiddy RGB30），有些用设备内部名称（如“tg5040”代表 Trimui Smart Pro），还有些用简短的别名（如“trimui”代表 Trimui Model S），所有名称都为小写。具体支持的平台文件夹名称请参考 extras bundle 获取最新信息。

部分平台下有多个具有独特特性的设备。NextUI 通过 `DEVICE` 环境变量区分这些设备。例如“rg35xxplus”平台下有“cube”（RG CubeXX）和“wide”（RG34xx）两个独特设备，还支持“hdmi”用于 HDMI 输出。Pak 可以选择使用或忽略该环境变量。

# 模拟器 pak 的类型

模拟器 pak 主要有三种类型，选择哪种取决于你的目标和对 NextUI 集成的需求。

第一种类型复用 NextUI 基础安装中自带的 libretro core。这种方式可以利用已知可用的核心，同时允许自定义默认选项并分离用户配置。例如，extra 里的 GG.pak 就使用了默认的 picodrive core。

第二种类型自带自己的 libretro core。这让你可以支持全新的系统，同时享受 NextUI 的标准特性，如菜单内恢复、快速存档和自动恢复、一致的游戏内菜单和操作体验。例如，extra 里的 MGBA.pak 就自带了 mgba core。

第三种类型则直接启动自带的独立模拟器。这种方式有时能让硬件性能发挥到极致，但缺点是无法与 NextUI 集成：没有菜单内恢复、没有快速存档和自动恢复、没有统一的游戏内菜单和操作体验。有些情况下，MENU（以及可能的 POWER）键可能无法正常工作。此类 pak 应作为最后手段。例如，社区开发的 NDS.pak 就属于这种类型，仅支持 NextUI 支持的部分平台。

无论哪种类型，请务必让用户知晓，@shauninman 无法为第三方 pak 提供支持。如果某个主机或核心未被纳入 NextUI 的基础或额外包，通常是有原因的：要么核心集成度不够（如街机核心对 rom 文件名和 rom 集有严格要求），要么 bug 太多（如无法可靠地从存档恢复），要么在某设备上性能不佳，或仅仅是我对该主机不熟悉或不感兴趣。

# 命名你的模拟器 pak

NextUI 会根据 rom 父文件夹名称末尾括号内的标签，将 rom 映射到 pak（如“/Roms/Game Boy (GB)/Dr. Mario (World).gb”会启动“GB.pak”）。标签应全部大写。选择标签时，优先参考 Retroarch 或 EmulationStation 等其他前端常用缩写（如 FC 代表 Famicom/NES，MD 代表 MegaDrive/Genesis）。如该标签已被其他 pak 占用，可用核心名（如 MGBA），或核心名缩写（如 PKM 代表 pokemini），或截断（如 SUPA 代表 mednafen_supafaust）。

# 启动你的核心

以下是“launch.sh”示例：

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

这样会用 NextUI 自带的“picodrive_libretro.so”核心打开指定 rom。若需更换核心，只需将 `EMU_EXE` 改为其他核心名（去掉“_libretro.so”）。如果核心随 pak 一起打包，则在 `EMU_EXE` 行后加上：

	CORES_PATH=$(dirname "$0")

井号分割线以下内容无需修改，都是通用模板：自动提取 pak 标签、创建 bios 和存档文件夹、设置 `HOME` 环境变量为“/.userdata/[platform]/”，启动游戏，并将 minarch 及核心输出日志写入“/.userdata/[platform]/logs/[TAG].txt”。

就是这样！你可以尝试用原厂固件、其他兼容设备的核心，甚至自己编译核心。

哦，如果你要为 Anbernic 的 RG*XX 系列做 pak，最后一行的 ` &> "$LOGS_PATH/$EMU_TAG.txt"` 需改为 ` > "$LOGS_PATH/$EMU_TAG.txt" 2>&1`，因为该系列默认 shell 比较特殊。

# 选项默认值与按键绑定

将新 pak 和一些 rom 拷贝到 SD 卡，启动游戏。按 MENU 键进入选项，配置前端、模拟器和按键。NextUI 的标准做法是只绑定原主机物理手柄上有的按键（如不加速、不绑定核心特有功能如调色盘或磁盘切换），让玩家自行深入设置，快捷键同理。最后选择“保存更改 > 保存到主机”。退出后将 SD 卡插回电脑。

在 SD 卡根目录的隐藏“.userdata”文件夹下，你会找到平台文件夹，里面有“[TAG]-[core]”文件夹。将其中的“minarch.cfg”复制到 pak 文件夹并重命名为“default.cfg”。打开“default.cfg”，删除未自定义的选项。以“-”开头的选项会被设定且隐藏，适合禁用某些不可用或性能不佳的功能（如超频、放大）。文件底部是按键绑定，例如 MGBA.pak 的配置：

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

`bind ` 后到 `=` 之间是控制菜单中显示的按键标签，建议统一风格（如用“Up”而不是“D-pad up”，“A Button”而不是“A”）。`=` 后到冒号前是按键映射，全部大写。肩键和摇杆键都带数字（如“L1”而不是“L”）。如无需默认绑定，用“NONE”。自定义或移除绑定时，建议在冒号后保留核心默认映射。例如：

	bind More Sun = L3

改为：

	bind More Sun = NONE:L3

# 亮度与音量

有些二进制程序会在每次启动时重置亮度（如 40xxH 原厂固件的 DinguxCommander）或音量（如 ppssppSDL）。为保持与 NextUI 全局设置同步，可用 syncsettings.elf。它会等待一秒后恢复 NextUI 当前亮度和音量。大多数情况下，直接后台启动即可：

	syncsettings.elf &
	./DinguxCommander

如果程序初始化超过一秒，可让它循环运行直到主程序退出：

	while :; do
	    syncsettings.elf
	done &
	LOOP_PID=$!
	
	./PPSSPPSDL --pause-menu-exit "$ROM_PATH"
	
	kill $LOOP_PID

# 注意事项

NextUI 目前仅支持 RGB565 像素格式，不支持 OpenGL libretro API。理论上可以用原厂固件的 retroarch 替代 NextUI 的 minarch 运行部分核心，但具体实现请自行探索。