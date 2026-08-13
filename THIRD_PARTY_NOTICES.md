<!--
SPDX-FileCopyrightText: 2026 Mr-Drinking

SPDX-License-Identifier: GPL-3.0-or-later
-->

# Third-party notices

This notice complements the per-file SPDX declarations, `.reuse/dep5`, and
license texts under `LICENSES/`. It does not replace notices shipped inside
third-party directories.

## Ghostwriter Mojikumi CJK

The bundled `GhostwriterMojikumiCJKsc-Regular.otf` is a Modified Version under
the SIL Open Font License 1.1. It is derived from the AOSP Noto Sans CJK 2.004
static collection and is not the upstream Noto binary under a new filename.
It is the complete proportional Simplified Chinese face (65,535 glyphs and
44,776 mapped code points), extracted without Unicode subsetting.

- Copyright © 2014-2021 Adobe (<http://www.adobe.com/>).
- License: SIL Open Font License 1.1 (`LICENSES/OFL-1.1.txt`).
- Private SC family: `Ghostwriter Mojikumi CJK SC`.
- Modification: the complete SC face is extracted from the TTC, and only its
  OpenType `name` table records are changed to private
  Ghostwriter Mojikumi family, full-name, unique-ID, and PostScript names. No
  glyph outlines, cmap, GPOS, or other layout tables are intentionally changed.
- Rebuild script: `tools/fonts/rename_noto_cjk.py`.

The input is the AOSP static collection from
`platform/external/noto-fonts`, commit
`aa96a71129acdb7ad8005ab5de269cb506d29655`:

- [immutable source tree](https://android.googlesource.com/platform/external/noto-fonts/+/aa96a71129acdb7ad8005ab5de269cb506d29655/notosanscjk/)
- [source TTC](https://android.googlesource.com/platform/external/noto-fonts/+/aa96a71129acdb7ad8005ab5de269cb506d29655/notosanscjk/NotoSansCJK-Regular.ttc?format=TEXT)
- [AOSP license notice](https://android.googlesource.com/platform/external/noto-fonts/+/aa96a71129acdb7ad8005ab5de269cb506d29655/notosanscjk/NOTICE?format=TEXT)

Reproducibility data:

| Object | SHA-256 |
| --- | --- |
| AOSP `NotoSansCJK-Regular.ttc` input (19,474,972 bytes) | `39fb47c543da50618ab99e8b9e5529e54566bdbef41719308165975f627d5c93` |
| `tools/fonts/rename_noto_cjk.py` | `155ea236ef53520b05c4a5fc0c17273cf5daa196d50068d4192d9d2c0711a8a4` |
| `GhostwriterMojikumiCJKsc-Regular.otf` output (16,435,576 bytes) | `52e34b390a05ceb19f22e80f5bf7fed76b1f16aebc9965451ee6a5e433575888` |

The AOSP file already contains Android's CJK spacing-related changes,
including `chws`/`vchw` support. The fork's additional transformation is
limited to extracting the complete SC face and applying the private naming
described above. Distributions must ship the OFL notice with every copy of the
font and must keep the font under OFL-1.1.

## Other bundled works

The repository also contains or embeds the following upstream works. Their
complete notices and license files remain in their respective directories and
in `LICENSES/`:

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
