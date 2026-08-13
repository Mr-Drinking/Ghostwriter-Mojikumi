<!--
SPDX-FileCopyrightText: 2014-2024 Megan Conkle <megan.conkle@kdemail.net>
SPDX-FileCopyrightText: 2022-present KDE
SPDX-FileCopyrightText: 2026 Mr-Drinking

SPDX-License-Identifier: GPL-3.0-or-later
-->

[English](README.md) | [简体中文](README.zh-CN.md)

# Ghostwriter Mojikumi — Unofficial Fork

Ghostwriter Mojikumi is an independent fork of KDE's distraction-free
Markdown editor. It adds CJK mojikumi (punctuation compression) to both the
editor and live preview and ships the complete AOSP Noto Sans CJK regional
collection for those two views.

This repository is **not an official KDE release**. It is based on commit
`db9690507e9ba9194af4ee0dbad66dc4b1507389`, a snapshot from ghostwriter's
`release/26.08` branch targeting 26.08.0, before KDE published a 26.08.0 tag.
Fork modifications were first published on 2026-08-13.

## What the fork changes

- The Qt editor applies contextual CJK punctuation compression and trims
  eligible fullwidth opening punctuation at the beginning of visual lines.
- The Chromium-powered live preview applies the corresponding CSS Text
  spacing behavior. Fenced and indented code blocks are included in both
  views rather than exempted from mojikumi.
- Mojikumi is presentation-only: it does not rewrite Markdown source or add
  undo-history entries.

The editor and preview use different layout engines. Near an extremely narrow
wrap-width threshold, Qt and Chromium can therefore choose different final
soft-wrap positions even though the same spacing policy is active. This is a
known engine boundary difference, not a change to the document text.

## Downloads and support

The packaging workflows target the following artifacts:

- Windows x86_64: portable ZIP and NSIS installer;
- macOS: Intel and Apple silicon DMGs;
- Linux x86_64: AppImage;
- Linux x86_64: a Flatpak bundle built from this checkout by the repository's
  Flatpak manifest and CI workflow. It uses the KDE 6.11 runtime and Qt
  WebEngine BaseApp. Whether the bundle appears on the
  [GitHub Releases page](https://github.com/Mr-Drinking/Ghostwriter-Mojikumi/releases)
  depends on a successful release workflow; this project is not listed on
  Flathub.

To install a downloaded Flatpak bundle for the current user:

```sh
flatpak install --user ./ghostwriter-mojikumi-linux-x86_64.flatpak
```

These are unofficial, unsigned development artifacts. Windows and macOS may
show an unknown-publisher or unidentified-developer warning; verify the
download source and checksums before bypassing operating-system protections.

Please report fork-specific issues in this repository's
[issue tracker](https://github.com/Mr-Drinking/Ghostwriter-Mojikumi/issues),
not to KDE's ghostwriter maintainers.

## Font behavior

The editor and preview use the same bundled proportional face by default. The
app chooses `Noto Sans CJK JP`, `KR`, `SC`, `TC`, or `HK` from the configured
interface language (other languages default to SC). The bundled TTC is the
complete, unmodified AOSP Noto Sans CJK static collection under OFL-1.1; its
five proportional faces contain `chws` and `halt`. The Mono faces are not used
because they lack `chws` and therefore cannot provide equivalent behavior.
Choosing another font changes the editor, preview body, and preview code fonts
together. The first non-bundled selection in a session shows a compatibility
warning: fallback glyphs or different OpenType support can change punctuation
spacing, so equivalent mojikumi is not guaranteed.

The exact source, hash, and license notice are documented in
[`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md).

## Build

The project retains the upstream `ghostwriter` CMake target and executable to
avoid breaking compatible tooling. It requires CMake, Qt 6.11 or later, Extra
CMake Modules, and KDE Frameworks 6; CMake reports any missing
platform-specific dependencies.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Run the installed application or the produced `ghostwriter` executable. See
the platform packaging configuration in this repository for distributable
artifacts.

## Upstream and attribution

The application remains based on
[KDE ghostwriter](https://invent.kde.org/office/ghostwriter), originally
developed by Megan Conkle with contributions from the ghostwriter community.
Upstream's homepage is <https://ghostwriter.kde.org>.

Fork identity, modification history, and support boundaries are summarized in
[`ABOUT.md`](ABOUT.md).

## Licensing

The application is distributed under GPL-3.0-or-later. Bundled fonts, icons,
and third-party libraries retain their own compatible licenses. See
[`COPYING`](COPYING), [`LICENSES`](LICENSES),
[`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md), and the license files in
the relevant third-party directories.
