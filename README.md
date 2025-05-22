<div align="center">

  <img width=128px src="github/logo_outline.png">

  <h1>NextUI-CN</h1>
  <h3>NextUI 中文版</h3>

</div>

<hr>

<h4 align="center">
  <a href="https://nextui.loveretro.games/getting-started/" target="_blank">安装指南</a>
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

> **这是一个专注于中文本地化的NextUI分支**  
> 本项目使用寒蝉全园体(ChillRound)作为中文字体，保持与原版更新同步的同时，提供完整的中文界面体验。

基于MinUI的自定义固件，重建了模拟器引擎，为TrimUI Brick和Smart Pro增加了大量功能。

其他设备建议查看[MinUI](https://github.com/shauninman/MinUI)

- 通过重建模拟器引擎核心，修复了MinUI的屏幕撕裂和同步卡顿问题
- 游戏切换菜单（Onion OS风格）由[@frysee](https://github.com/frysee)开发
- 高品质音频，使用[libsamplerate](https://github.com/libsndfile/libsamplerate)先进重采样引擎（每个模拟器可设置质量/性能）
- 极低延迟（平均20毫秒，相当于60fps的1帧）
- 支持着色器！
- 完全基于OpenGL/GPU实现更快性能！
- 原生WiFi支持
- 游戏美术/媒体支持
- 游戏时间追踪！
- 金手指支持
- 集成WiFi功能
- 支持覆盖层！
- 动态CPU速度调节（保持设备冷却并延长电池寿命，同时在需要时提供性能）
- 完全基于GPU的OpenGL实现更快性能
- 自定义启动画面由[@SolvalouArt](https://bsky.app/profile/solvalouart.bsky.social)贡献
- 颜色和字体设置可自定义NextUI界面
- 菜单动画效果
- LED控制，更改颜色、效果、亮度
- LED指示器，低电量、待机、充电（亮度可单独设置）
- 菜单触觉反馈由[@ExonakiDev](https://github.com/exonakidev)开发
- 环境LED模式，LED灯像环境光电视一样工作以增强沉浸感，可按模拟器配置
- 显示控制（色温、亮度、对比度、饱和度、曝光度）
- 支持通过NTP自动时间同步，包括时区和实时时钟(RTC)
- 深度睡眠模式，提供即时开机并避免Brick上的过热错误，由[@zhaofengli](https://github.com/zhaofengli)开发
- 电池监控，包括历史图表和剩余时间预测
- 长游戏名称的滚动动画标题
- 更新和优化的模拟器核心
- 修复震动强度，现在可变并由游戏使用
- FBNeo街机屏幕旋转
- PAL模式
- Next字体支持CJK，适用于日文/中文等ROM名称
- 许多其他小修复和优化

---

## 当前支持的设备：
- Trimui Brick
- Trimui Smart Pro

---

## 未来计划路线图

- 蓝牙和WiFi集成
- 可配置的FN和切换按钮
- 更多与不同模拟器的兼容性测试，并在必要时修复/改进
- 复古成就系统
- 我们可能会在途中想到更多要添加的功能 :D
- 一旦一切符合我的期望，我将考虑将其移植到更多设备
- 清理所有MinUI代码，并剔除所有针对旧设备的内容等

---

## 安装方法

请参考我们的[安装指南](https://nextui.loveretro.games/getting-started/)。

---

## 使用方法

在菜单中：

- 按住`开始键`并按`音量上`或`音量下`调节亮度
- 按住`选择键`并按`音量上`或`音量下`调节色温
- 短按`选择键`打开游戏切换器

在游戏中：

- `菜单键`打开游戏内选项菜单，调整控制、缩放等
- 同时按住`菜单键`和`选择键`打开游戏切换器

深度睡眠：

当设备闲置时，将进入轻度睡眠。轻度睡眠会关闭屏幕并使LED灯闪烁5次。

两分钟后，设备将进入深度睡眠。LED灯将完全关闭。

---

## 关于其他功能？

NextUI支持各种称为Paks的可选扩展。

这些Paks引入了各种新功能和模拟器。

我们的社区精心整理了[最受欢迎的Paks列表](https://nextui.loveretro.games/paks/community-favorites/)和[独立模拟器列表](https://nextui.loveretro.games/paks/standalone-emulators/)。

---

## 鸣谢

[@josegonzalez](https://github.com/josegonzalez)开发的[minui-keyboard](https://github.com/josegonzalez/minui-keyboard/t)

---

## Play Retro对NextUI的评测

[![image](https://github.com/user-attachments/assets/43217a30-e052-4f67-88a9-c4361f82e72f)](https://www.youtube.com/watch?v=m_7ObMHAFK4)

## Retro Handhelds对NextUI的报道

[![image](https://github.com/user-attachments/assets/5fd538ea-285b-46e9-add4-1ef99b2ee9e5)](https://www.youtube.com/watch?v=KlYVmtYDqRI)

---

## 更多信息请访问我们的网站：[nextui.loveretro.games](https://nextui.loveretro.games)
