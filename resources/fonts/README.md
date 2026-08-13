<!--
SPDX-FileCopyrightText: 2026 Mr-Drinking

SPDX-License-Identifier: GPL-3.0-or-later
-->

# Bundled Noto Sans CJK collection

`NotoSansCJK-Regular.ttc` is the complete, byte-for-byte AOSP static OpenType
collection from `platform/external/noto-fonts`, commit
`aa96a71129acdb7ad8005ab5de269cb506d29655`, path
`notosanscjk/NotoSansCJK-Regular.ttc`:

https://android.googlesource.com/platform/external/noto-fonts/+/aa96a71129acdb7ad8005ab5de269cb506d29655/notosanscjk/NotoSansCJK-Regular.ttc

It is Noto CJK 2.004 with AOSP's `chws` table fix. The file is 19,474,972
bytes; SHA-256 is
`39fb47c543da50618ab99e8b9e5529e54566bdbef41719308165975f627d5c93`;
Git blob ID is `31ab084552348c8e904eec503021544b3f4fd43b`.

The collection has proportional JP, KR, SC, TC, and HK faces followed by five
corresponding Mono faces. Each face has 65,535 glyphs. The proportional faces
contain both `chws` and `halt`; the Mono faces contain `halt` but not `chws`.
Ghostwriter Mojikumi therefore selects a proportional regional face for both
the editor and preview so their contextual punctuation behavior stays aligned.

Copyright © 2014-2021 Adobe (http://www.adobe.com/). The unchanged collection
is distributed under SIL Open Font License 1.1; see `LICENSES/OFL-1.1.txt`,
`THIRD_PARTY_NOTICES.md`, and `.reuse/dep5`.
