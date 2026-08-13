# SPDX-FileCopyrightText: 2026 Ghostwriter Mojikumi contributors
# SPDX-License-Identifier: BSD-2-Clause

"""Build the macOS ICNS from a 1024px render of the project SVG artwork."""

from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image


CANVAS_SIZE = 1024
# Apple's modern rounded-square artwork template occupies roughly 824 pixels
# of the 1024-pixel canvas. Keeping this transparent margin stops the icon from
# appearing oversized beside system and other modern application icons.
ARTWORK_SIZE = 824


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path, help="1024x1024 transparent PNG")
    parser.add_argument("destination", type=Path, help="output .icns path")
    args = parser.parse_args()

    source = Image.open(args.source).convert("RGBA")
    if source.size != (CANVAS_SIZE, CANVAS_SIZE):
        parser.error(f"source must be 1024x1024, got {source.size}")

    artwork = source.resize(
        (ARTWORK_SIZE, ARTWORK_SIZE),
        Image.Resampling.LANCZOS,
    )
    canvas = Image.new("RGBA", (CANVAS_SIZE, CANVAS_SIZE), (0, 0, 0, 0))
    offset = (CANVAS_SIZE - ARTWORK_SIZE) // 2
    canvas.alpha_composite(artwork, (offset, offset))

    bbox = canvas.getchannel("A").getbbox()
    if bbox is None:
        raise RuntimeError("generated icon is fully transparent")
    width = bbox[2] - bbox[0]
    height = bbox[3] - bbox[1]
    if width > ARTWORK_SIZE + 2 or height > ARTWORK_SIZE + 2:
        raise RuntimeError(f"generated artwork exceeds safe area: {bbox}")

    args.destination.parent.mkdir(parents=True, exist_ok=True)
    canvas.save(args.destination, format="ICNS")

    generated = Image.open(args.destination)
    representations = set(generated.info.get("sizes", []))
    physical_sizes = {
        width * scale for width, _height, scale in representations
    }
    expected_sizes = {32, 64, 128, 256, 512, 1024}
    if not expected_sizes.issubset(physical_sizes):
        raise RuntimeError(
            f"ICNS is missing expected physical sizes: {physical_sizes}"
        )

    print(
        f"generated {args.destination} with alpha bbox {bbox} "
        f"and physical sizes {sorted(physical_sizes)}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
