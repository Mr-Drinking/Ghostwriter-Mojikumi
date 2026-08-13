<!--
SPDX-FileCopyrightText: 2026 Mr-Drinking

SPDX-License-Identifier: GPL-3.0-or-later
-->

# About Ghostwriter Mojikumi

**Product name:** Ghostwriter Mojikumi

**Application ID:** `io.github.mr_drinking.ghostwriter-mojikumi`

**Release:** `26.08.0`

**Repository and homepage:**
<https://github.com/Mr-Drinking/Ghostwriter-Mojikumi>

**Support:**
<https://github.com/Mr-Drinking/Ghostwriter-Mojikumi/issues>

This is an independently maintained modified version. It is not produced,
endorsed, or supported by KDE or the upstream ghostwriter maintainers.

## Provenance and changes

Ghostwriter Mojikumi is based on commit
`db9690507e9ba9194af4ee0dbad66dc4b1507389`, a snapshot from KDE
ghostwriter's `release/26.08` branch targeting 26.08.0, before KDE published a
26.08.0 tag. The Mojikumi changes were first made and identified on 2026-08-13.
They add CJK mojikumi in the editor and preview,
select a common regional font for both views, warn when another font may change punctuation
spacing, and give distributable metadata an independent application identity.

The internal CMake target, executable name, source namespace, settings keys,
and inherited icon asset remain `ghostwriter` for compatibility. Those
technical names do not indicate an official KDE build.

## Copyright and license

Copyright in the upstream application remains with Megan Conkle, KDE, and all
other upstream contributors as recorded in the source history and file
headers. Ghostwriter Mojikumi modifications are copyright 2026 Mr-Drinking.

The application is licensed under GPL-3.0-or-later. Separately bundled works
retain their respective licenses; see [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md),
[`LICENSES`](LICENSES), and third-party directories.
