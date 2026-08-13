# SPDX-FileCopyrightText: 2026 Mr-Drinking
#
# SPDX-License-Identifier: GPL-3.0-or-later

"""Build the private-name, complete SC face used by Ghostwriter Mojikumi.

The pinned AOSP input is a ten-face OpenType collection. This script extracts
the complete Simplified Chinese proportional face (index 2) and changes only
its naming metadata. Glyph outlines, cmap, GPOS, and other layout tables are
preserved; this is not a Unicode subset.
"""

from __future__ import annotations

import argparse
import hashlib
import os
import tempfile
from pathlib import Path

from fontTools.ttLib import TTCollection


SOURCE_SHA256 = "39fb47c543da50618ab99e8b9e5529e54566bdbef41719308165975f627d5c93"
SOURCE_FACE_INDEX = 2
FAMILY = "Ghostwriter Mojikumi CJK SC"
POSTSCRIPT_NAME = "GhostwriterMojikumiCJKsc-Regular"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def renamed_value(name_id: int) -> str | None:
    return {
        1: FAMILY,
        3: f"2.004;GWMOJIKUMI;{POSTSCRIPT_NAME}",
        4: f"{FAMILY} Regular",
        6: POSTSCRIPT_NAME,
    }.get(name_id)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()

    actual_sha256 = sha256(args.source)
    if actual_sha256 != SOURCE_SHA256:
        raise SystemExit(
            f"unexpected source SHA-256: {actual_sha256}; expected {SOURCE_SHA256}"
        )

    collection = TTCollection(args.source)
    if len(collection.fonts) != 10:
        raise SystemExit(f"expected 10 faces, found {len(collection.fonts)}")

    font = collection.fonts[SOURCE_FACE_INDEX]
    if len(font.getGlyphOrder()) != 65_535:
        raise SystemExit("the selected SC face is not the expected complete face")

    # Preserve the pinned source font's head.modified value. fontTools
    # otherwise replaces it with the current time on every save, making an
    # otherwise identical font produce a different release hash.
    font.recalcTimestamp = False

    for record in font["name"].names:
        replacement = renamed_value(record.nameID)
        if replacement is not None:
            record.string = replacement.encode(record.getEncoding())

    cff = font["CFF "].cff
    top_dict = cff.topDictIndex[0]
    cff.fontNames = [POSTSCRIPT_NAME]
    top_dict.FullName = f"{FAMILY} Regular"
    top_dict.FamilyName = FAMILY

    args.output.parent.mkdir(parents=True, exist_ok=True)
    descriptor, temporary_name = tempfile.mkstemp(
        dir=args.output.parent, prefix=args.output.name, suffix=".tmp"
    )
    os.close(descriptor)
    temporary_path = Path(temporary_name)
    try:
        font.save(temporary_path, reorderTables=False)
        os.replace(temporary_path, args.output)
    finally:
        temporary_path.unlink(missing_ok=True)

    print(f"{args.output}: {sha256(args.output)}")


if __name__ == "__main__":
    main()
