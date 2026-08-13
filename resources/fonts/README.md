<!--
SPDX-FileCopyrightText: 2026 Mr-Drinking

SPDX-License-Identifier: GPL-3.0-or-later
-->

# Bundled Noto Sans CJK font

`GhostwriterMojikumiCJKsc-Regular.otf` is built from the pinned static OpenType
collection in AOSP `platform/external/noto-fonts`, commit
`aa96a71129acdb7ad8005ab5de269cb506d29655`, where it is stored at
`notosanscjk/NotoSansCJK-Regular.ttc`. The immutable source page is:

https://android.googlesource.com/platform/external/noto-fonts/+/aa96a71129acdb7ad8005ab5de269cb506d29655/notosanscjk/NotoSansCJK-Regular.ttc

The source is Noto CJK 2.004 with AOSP's `chws` table fix. Its SHA-256 is
`39fb47c543da50618ab99e8b9e5529e54566bdbef41719308165975f627d5c93`, its
Git blob ID is `31ab084552348c8e904eec503021544b3f4fd43b`, and its size is 19,474,972
bytes.

`tools/fonts/rename_noto_cjk.py` produces the bundled OFL Modified Version by
extracting the complete proportional Simplified Chinese face (index 2) without
Unicode subsetting, then changing name IDs 1 (family), 3 (unique ID), 4 (full
name), and 6 (PostScript name), plus the corresponding CFF names. This
prevents an installed same-family Noto font from replacing the application
font. Glyphs and OpenType layout tables are not modified. Rebuild with:

```shell
python tools/fonts/rename_noto_cjk.py \
    Android-NotoSansCJK-Regular.ttc \
    resources/fonts/GhostwriterMojikumiCJKsc-Regular.otf
```

The generated file contains 65,535 glyphs and 44,776 mapped code points. It is
16,435,576 bytes and has SHA-256
`52e34b390a05ceb19f22e80f5bf7fed76b1f16aebc9965451ee6a5e433575888`.
Its PostScript name is
`GhostwriterMojikumiCJKsc-Regular` and family name is
`Ghostwriter Mojikumi CJK SC`.

Copyright © 2014-2021 Adobe (http://www.adobe.com/). The modified font is
distributed under the SIL Open Font License 1.1; see
`LICENSES/OFL-1.1.txt` and the corresponding entry in `.reuse/dep5`.
