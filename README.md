<!--
SPDX-FileCopyrightText: 2014-2024 Megan Conkle <megan.conkle@kdemail.net>
SPDX-FileCopyrightText: 2022-present KDE
SPDX-FileCopyrightText: 2026 Mr-Drinking

SPDX-License-Identifier: GPL-3.0-or-later
-->

# Ghostwriter Mojikumi — Unofficial Fork

Ghostwriter Mojikumi is an independent fork of KDE's distraction-free
Markdown editor. It adds CJK mojikumi (punctuation compression) to both the
editor and live preview and ships a consistently named CJK font for those two
views.

This repository is **not an official KDE release**. It is based on ghostwriter
26.08.0 at commit
`db9690507e9ba9194af4ee0dbad66dc4b1507389`; fork modifications were first
published on 2026-08-13.

## Downloads and support

Windows, macOS, and Linux release artifacts are published on the
[GitHub Releases page](https://github.com/Mr-Drinking/Ghostwriter-Mojikumi/releases).
Please report fork-specific issues in this repository's
[issue tracker](https://github.com/Mr-Drinking/Ghostwriter-Mojikumi/issues),
not to KDE's ghostwriter maintainers.

## Font behavior

The editor and preview use the bundled `Ghostwriter Mojikumi CJK SC` family by
default. It is an OFL-1.1 Modified Version derived from AOSP Noto Sans CJK. The
complete SC face is extracted without Unicode subsetting, and only its naming
metadata is changed to give the bundled font a private family name.
Choosing another font changes both views and shows a warning because fallback
glyphs or different OpenType support can change punctuation spacing.

The exact source, transformation, hashes, and license notice are documented in
[`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md).

## Build

The project retains the upstream `ghostwriter` CMake target and executable to
avoid breaking compatible tooling. It requires CMake, Qt 6, Extra CMake
Modules, and KDE Frameworks 6; CMake reports any missing platform-specific
dependencies.

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
