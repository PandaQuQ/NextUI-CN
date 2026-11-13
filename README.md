<div align="center">

  <img width=128px src="github/logo_outline.png">

  <h1>NextUI-CN</h1>
  <h3>NextUI 中文本地化分支</h3>

</div>

<hr>

<h4 align="center">
  <a href="https://nextui.loveretro.games/usage/#getting-started" target="_blank">安装指南</a>
  ·
  <a href="https://nextui.loveretro.games/docs/" target="_blank">文档</a>
  ·
  <a href="https://discord.gg/HKd7wqZk3h" target="_blank">Discord</a>
</h4>

<div align="center"><p>
    <a href="https://github.com/PandaQuQ/NextUI-CN/releases/latest">
      <img alt="最新版本" src="https://img.shields.io/github/v/release/PandaQuQ/NextUI-CN?style=for-the-badge&logo=starship&include_prerelease=false&color=C9CBFF&logoColor=D9E0EE&labelColor=302D41&sort=semver" />
    </a>
    <a href="https://github.com/PandaQuQ/NextUI-CN/pulse">
      <img alt="最近提交" src="https://img.shields.io/github/last-commit/PandaQuQ/NextUI-CN?style=for-the-badge&logo=starship&color=8bd5ca&logoColor=D9E0EE&labelColor=302D41"/>
    </a>
    <a href="https://github.com/PandaQuQ/NextUI-CN/blob/main/LICENSE">
      <img alt="许可证" src="https://img.shields.io/github/license/PandaQuQ/NextUI-CN?style=for-the-badge&logo=starship&color=ee999f&logoColor=D9E0EE&labelColor=302D41" />
    </a>
    <a href="https://github.com/PandaQuQ/NextUI-CN/stargazers">
      <img alt="星标" src="https://img.shields.io/github/stars/PandaQuQ/NextUI-CN?style=for-the-badge&logo=starship&color=c69ff5&logoColor=D9E0EE&labelColor=302D41" />
    </a>
    <a href="https://github.com/PandaQuQ/NextUI-CN/issues">
      <img alt="问题" src="https://img.shields.io/github/issues/PandaQuQ/NextUI-CN?style=for-the-badge&logo=bilibili&color=F5E0DC&logoColor=D9E0EE&labelColor=302D41" />
    </a>
</div>

---

> **这是一个专注于中文本地化的 NextUI 分支**
>
> - 上游项目：<https://github.com/LoveRetro/NextUI>
> - 本分支目标：尽量以“薄补丁”的方式提供完整中文界面，同时减少与上游同步时的冲突。

NextUI 是一个基于 MinUI 的自定义固件（CFW），通过重建模拟器引擎，为 TrimUI Brick 和 Smart Pro 带来大量新增功能与优化。

其他设备建议查看 [MinUI](https://github.com/shauninman/MinUI)。

字体说明（本中文分支）：

- 项目使用 [Dream Han Sans CN](https://github.com/Pal3love/dream-han-cjk) 作为中文字体来源。
- 为了避免把字体大文件长期放进仓库，本仓库在 CI 中会在构建时注入字体（见工作流），并将默认字体文件替换为中文可用字体。

## 许可证

上游项目自 2026 年 8 月 15 日起由 GPL 3.0 改为 [PolyForm Noncommercial 1.0.0](https://polyformproject.org/licenses/noncommercial/1.0.0) 许可证。
允许个人、教育及其他非商业用途自由使用，禁止商业用途。
完整条款见 [LICENSE](LICENSE) 与 [NOTICE](NOTICE)，变更经过与理由见上游 [issue #765](https://github.com/LoveRetro/NextUI/issues/765)。

## 功能亮点

（来自上游 README，并做中文化整理）

- 通过重建模拟器引擎核心，修复 MinUI 的屏幕撕裂和同步卡顿问题
- 游戏切换菜单（Onion OS 风格）由 [@frysee](https://github.com/frysee) 开发
- 高品质音频：使用 [libsamplerate](https://github.com/libsndfile/libsamplerate) 进行高级重采样（每个模拟器可设置质量/性能）
- 更低延迟（平均 20ms，约等于 60fps 的 1 帧）
- 着色器（Shaders）
- 完全基于 OpenGL/GPU，性能更快
- 原生 WiFi 支持 / 集成 WiFi
- 蓝牙音频
- 游戏封面/媒体支持
- 游戏时间追踪（Game time tracker）
- 金手指（Cheats）
- 覆盖层（Overlays）
- 动态 CPU 速度调节（更冷、更省电，同时按需提升性能）
- 自定义启动画面由 [@SolvalouArt](https://bsky.app/profile/solvalouart.bsky.social) 贡献
- 颜色与字体设置：自定义 NextUI 界面
- 菜单动画
- LED 控制：颜色、效果、亮度
- LED 指示：低电量、待机、充电（亮度可分别设置）
- 菜单触觉反馈由 [@ExonakiDev](https://github.com/exonakidev) 开发
- 环境 LED 模式：模拟“氛围灯”，可按模拟器配置
- 显示控制（色温、亮度、对比度、饱和度、曝光度）
- 支持 NTP 自动时间同步（含时区）与实时时钟（RTC）
- 深度睡眠：实现即时开机并避免 Brick 的过热 bug，由 [@zhaofengli](https://github.com/zhaofengli) 开发
- 电池监控：历史曲线与剩余时间预测
- 长游戏名滚动标题
- 更新/优化模拟器核心
- 震动强度修复：现在可变并由游戏驱动
- FBNeo 街机屏幕旋转
- PAL 模式
- Next 字体支持中日韩（CJK）游戏名显示
- 许多其他小修复和优化

---

## 当前支持的设备
- Trimui Brick
- Trimui Brick Pro
- Trimui Smart Pro
- Trimui Smart Pro S
- Anbernic H700 Devices

---

## 路线图（参考）

> 说明：以下内容来自本仓库 main 分支历史文档，可能会随上游变化而过时；请以官网与上游仓库为准。

- 蓝牙和 WiFi 集成
- 可配置的 FN 和切换按钮
- 更多与不同模拟器的兼容性测试，并在必要时修复/改进
- 复古成就系统
- 其他优化与清理（例如清理旧设备相关代码等）

---

## 安装

请参考 [安装指南](https://nextui.loveretro.games/usage/#getting-started)。

---

## 使用方法

在菜单中：

- 按住 `start` 并按 `volume up` / `volume down` 调节亮度
- 按住 `select` 并按 `volume up` / `volume down` 调节色温
- 短按 `select` 打开游戏切换器

在游戏中：

- `Menu` 打开游戏内选项菜单（控制、缩放等）
- 同时按住 `menu` + `select` 打开游戏切换器

深度睡眠：

设备闲置后会进入轻度睡眠：屏幕关闭，LED 闪烁 5 次。

两分钟后进入深度睡眠：LED 将完全关闭。

---

## 关于其他功能？

NextUI 支持各种由社区成员维护的可选扩展（Paks）。

官方 Wiki 提供了[全部可用 Paks 列表](https://nextui.loveretro.games/pak-store/)，可以直接在设备上通过 Pak Store 下载安装。

---

## 鸣谢

- [josegonzalez](https://github.com/josegonzalez) for [minui-keyboard](https://github.com/josegonzalez/minui-keyboard)
- [knulli-cfw](https://github.com/knulli-cfw)/[Mikhailzrick](https://github.com/Mikhailzrick) for [fan-control-daemon](https://github.com/knulli-cfw/knulli-linux/commit/5c1b4724330b59cbe8af79d0a7f1dac86f1c221a)
- [Kenney](www.kenney.nl) for [Input Prompts](https://kenney.nl/assets/input-prompts)

---

## Play Retro 对 NextUI 的评测

[![image](https://github.com/user-attachments/assets/43217a30-e052-4f67-88a9-c4361f82e72f)](https://www.youtube.com/watch?v=m_7ObMHAFK4)

## Retro Handhelds 对 NextUI 的报道

[![image](https://github.com/user-attachments/assets/5fd538ea-285b-46e9-add4-1ef99b2ee9e5)](https://www.youtube.com/watch?v=KlYVmtYDqRI)

---

## 更多信息请访问官网：[nextui.loveretro.games](https://nextui.loveretro.games)
# Build status
