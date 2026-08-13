<!--
SPDX-FileCopyrightText: 2026 Mr-Drinking

SPDX-License-Identifier: GPL-3.0-or-later
-->

# Third-party notices

This notice complements the per-file SPDX declarations, `.reuse/dep5`, and
license texts under `LICENSES/`.

## Noto Sans CJK

The bundled `resources/fonts/NotoSansCJK-Regular.ttc` is the complete,
unmodified AOSP Noto Sans CJK 2.004 static collection. It contains 10 faces:
proportional and Mono variants for JP, KR, SC, TC, and HK. Each face contains
65,535 glyphs; no region or Unicode range is subsetted.

- Copyright © 2014-2021 Adobe (<http://www.adobe.com/>).
- License: SIL Open Font License 1.1 (`LICENSES/OFL-1.1.txt`).
- AOSP repository: `platform/external/noto-fonts`.
- Commit: `aa96a71129acdb7ad8005ab5de269cb506d29655`.
- Git blob: `31ab084552348c8e904eec503021544b3f4fd43b`.
- Size: 19,474,972 bytes.
- SHA-256: `39fb47c543da50618ab99e8b9e5529e54566bdbef41719308165975f627d5c93`.
- [immutable source tree](https://android.googlesource.com/platform/external/noto-fonts/+/aa96a71129acdb7ad8005ab5de269cb506d29655/notosanscjk/)
- [source TTC](https://android.googlesource.com/platform/external/noto-fonts/+/aa96a71129acdb7ad8005ab5de269cb506d29655/notosanscjk/NotoSansCJK-Regular.ttc?format=TEXT)
- [AOSP license notice](https://android.googlesource.com/platform/external/noto-fonts/+/aa96a71129acdb7ad8005ab5de269cb506d29655/notosanscjk/NOTICE?format=TEXT)

The proportional faces contain AOSP's `chws`/`vchw` changes plus `halt`.
Ghostwriter Mojikumi registers the same embedded bytes with Qt and copies them,
after hash verification, to an application-private data directory because
Chromium cannot fetch a `qrc:` web font from the preview's `file:` origin.
That runtime copy is byte-identical and is not installed system-wide.

## Other bundled works

- KDE ghostwriter, including work originally developed by Megan Conkle:
  GPL-3.0-or-later.
- ghostwriter application icons by Megan Conkle: CC-BY-SA-4.0.
- cmark-gfm: BSD-2-Clause and MIT; its specification is CC-BY-SA-4.0.
- MathJax: Apache-2.0.
- React: MIT.
- Hunspell components: MPL-1.1, GPL-2.0-or-later, or LGPL-2.1-or-later.
- Font Awesome icons: OFL-1.1.
- Google Material icons: Apache-2.0.
- KDE `poqm` components: LGPL-2.1-or-later.
