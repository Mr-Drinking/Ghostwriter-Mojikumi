<!--
SPDX-FileCopyrightText: 2014-2024 Megan Conkle <megan.conkle@kdemail.net>
SPDX-FileCopyrightText: 2022-present KDE
SPDX-FileCopyrightText: 2026 Mr-Drinking

SPDX-License-Identifier: GPL-3.0-or-later
-->

[English](README.md) | [简体中文](README.zh-CN.md)

# Ghostwriter Mojikumi — 非官方分支

Ghostwriter Mojikumi 是 KDE 专注写作 Markdown 编辑器 ghostwriter 的独立
分支。它在编辑器和实时预览中加入了 CJK 标点挤压（mojikumi），并为这两个
视图内置了一款家族名固定的 CJK 字体。

本仓库**不是 KDE 官方版本**。它以 ghostwriter `release/26.08` 分支中面向
26.08.0 的快照提交 `db9690507e9ba9194af4ee0dbad66dc4b1507389`
为基础，早于 KDE 发布 26.08.0 标签；本分支的修改于 2026-08-13 首次发布。

## 本分支的改动

- Qt 编辑器会对连续的 CJK 标点应用上下文挤压，并收紧每个视觉行行首符合
  条件的全角开标点。
- 基于 Chromium 的实时预览会应用对应的 CSS Text 标点间距规则。围栏式和
  缩进式代码块在两个视图中同样应用标点挤压，不会被排除。
- 标点挤压只改变显示，不会改写 Markdown 源文本，也不会增加撤销历史记录。

编辑器和预览使用不同的排版引擎。因此，在极窄的换行临界宽度附近，即使
两边启用了相同的标点间距策略，Qt 和 Chromium 的最终软换行位置仍可能
不同。这是已知的引擎边界差异，不代表文档文本发生了变化。

## 下载与支持

打包工作流面向以下产物：

- Windows x86_64：便携版 ZIP；
- macOS：Intel 与 Apple 芯片版 DMG；
- Linux x86_64：AppImage；
- Linux x86_64：由仓库中的 Flatpak 清单与 CI 工作流从当前源码检出构建的
  Flatpak 单文件包，使用 KDE 6.11 运行时和 Qt WebEngine BaseApp。该包是否
  出现在 [GitHub Releases 页面](https://github.com/Mr-Drinking/Ghostwriter-Mojikumi/releases)
  取决于发布工作流是否成功；本项目尚未上架 Flathub。

将下载的 Flatpak 单文件包安装到当前用户：

```sh
flatpak install --user ./ghostwriter-mojikumi-linux-x86_64.flatpak
```

这些是非官方、未签名的开发构建。Windows 和 macOS 可能显示“未知发布者”或
“无法验证开发者”等警告；绕过操作系统保护前，请先核对下载来源与校验和。

请在本仓库的 [Issue 跟踪器](https://github.com/Mr-Drinking/Ghostwriter-Mojikumi/issues)
报告本分支特有的问题，不要向 KDE ghostwriter 维护者报告。

## 字体行为

编辑器和预览默认使用内置的 `Ghostwriter Mojikumi CJK SC` 字体家族。它是
基于 AOSP Noto Sans CJK、以 OFL-1.1“修改版本”形式发布的字体。内置文件是
未做 Unicode 子集化的完整 SC 字体面，仅更改了命名元数据，以获得本项目专用
的字体家族名。

选择其他字体时，编辑器、预览正文和预览代码字体会一并更改。每次运行中
首次选择非内置字体时，程序会显示兼容性警告：字体回退字形或不同的 OpenType
支持可能改变标点间距，因此无法保证得到等同的标点挤压效果。

字体的确切来源、转换方式、哈希值与许可证声明详见
[`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md)。

## 构建

项目保留了上游的 `ghostwriter` CMake 目标名和可执行文件名，以免破坏兼容
工具。构建需要 CMake、Qt 6.11 或更高版本、Extra CMake Modules 和 KDE
Frameworks 6；如缺少特定平台的依赖，CMake 会给出提示。

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

随后可运行安装后的应用，或构建生成的 `ghostwriter` 可执行文件。分发产物的
配置请参阅本仓库中的各平台打包配置。

## 上游与署名

本应用仍以 [KDE ghostwriter](https://invent.kde.org/office/ghostwriter) 为
基础。原作者为 Megan Conkle，并有 ghostwriter 社区成员参与贡献。上游官网为
<https://ghostwriter.kde.org>。

本分支的身份、修改历史与支持范围汇总于 [`ABOUT.md`](ABOUT.md)。

## 许可证

应用以 GPL-3.0-or-later 许可证分发。内置字体、图标和第三方库分别保留各自
兼容的许可证。详见 [`COPYING`](COPYING)、[`LICENSES`](LICENSES)、
[`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md)，以及相应第三方目录中的
许可证文件。
