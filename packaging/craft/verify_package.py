# SPDX-FileCopyrightText: 2026 Ghostwriter Mojikumi contributors
# SPDX-License-Identifier: BSD-2-Clause

"""Perform release-package content and command-line smoke checks."""

from __future__ import annotations

import argparse
import hashlib
import os
from pathlib import Path
import plistlib
import struct
import subprocess
import sys
import tempfile
import zlib


MACOS_ICON_SHA256 = (
    "7f2afcdde8231128550f72f4e88dc1640c58e7d3aa383b1662ede609d1320ea6"
)
LINUX_ICON_SHA256 = (
    "9bb90cba581358d578884d994e271e601dc4eff4fa88b0cb2672ec68f7852f37"
)


def all_entries(root: Path) -> list[Path]:
    return list(root.rglob("*"))


def matching(entries: list[Path], predicate) -> list[Path]:
    return [entry for entry in entries if predicate(entry)]


def require(description: str, paths: list[Path]) -> Path:
    if not paths:
        raise RuntimeError(f"Missing {description}")

    print(f"Found {description}: {paths[0]}")
    return paths[0]


def named_files(entries: list[Path], name: str) -> list[Path]:
    expected = name.casefold()
    return matching(
        entries,
        lambda path: path.is_file() and path.name.casefold() == expected,
    )


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def png_alpha_bbox(png: bytes) -> tuple[int, int, int, int, int, int]:
    if not png.startswith(b"\x89PNG\r\n\x1a\n"):
        raise RuntimeError("ICNS 1024px representation is not PNG data")

    width = height = 0
    compressed = bytearray()
    offset = 8
    while offset < len(png):
        if offset + 12 > len(png):
            raise RuntimeError("Truncated PNG chunk in macOS icon")
        length = struct.unpack(">I", png[offset : offset + 4])[0]
        chunk_type = png[offset + 4 : offset + 8]
        payload = png[offset + 8 : offset + 8 + length]
        if len(payload) != length:
            raise RuntimeError("Truncated PNG payload in macOS icon")
        if chunk_type == b"IHDR":
            width, height, depth, colour, compression, filtering, interlace = (
                struct.unpack(">IIBBBBB", payload)
            )
            if (depth, colour, compression, filtering, interlace) != (8, 6, 0, 0, 0):
                raise RuntimeError(
                    "Unsupported PNG format in macOS icon: "
                    f"depth={depth} colour={colour} interlace={interlace}"
                )
        elif chunk_type == b"IDAT":
            compressed.extend(payload)
        elif chunk_type == b"IEND":
            break
        offset += 12 + length

    if width <= 0 or height <= 0 or not compressed:
        raise RuntimeError("Incomplete PNG representation in macOS icon")

    bytes_per_pixel = 4
    stride = width * bytes_per_pixel
    raw = zlib.decompress(compressed)
    if len(raw) != (stride + 1) * height:
        raise RuntimeError("Unexpected PNG scanline size in macOS icon")

    def paeth(left: int, above: int, upper_left: int) -> int:
        estimate = left + above - upper_left
        left_distance = abs(estimate - left)
        above_distance = abs(estimate - above)
        upper_left_distance = abs(estimate - upper_left)
        if left_distance <= above_distance and left_distance <= upper_left_distance:
            return left
        if above_distance <= upper_left_distance:
            return above
        return upper_left

    previous = bytearray(stride)
    min_x, min_y, max_x, max_y = width, height, -1, -1
    cursor = 0
    for y in range(height):
        filter_type = raw[cursor]
        cursor += 1
        row = bytearray(raw[cursor : cursor + stride])
        cursor += stride
        for index in range(stride):
            left = row[index - bytes_per_pixel] if index >= bytes_per_pixel else 0
            above = previous[index]
            upper_left = (
                previous[index - bytes_per_pixel]
                if index >= bytes_per_pixel
                else 0
            )
            if filter_type == 1:
                row[index] = (row[index] + left) & 0xFF
            elif filter_type == 2:
                row[index] = (row[index] + above) & 0xFF
            elif filter_type == 3:
                row[index] = (row[index] + ((left + above) // 2)) & 0xFF
            elif filter_type == 4:
                row[index] = (row[index] + paeth(left, above, upper_left)) & 0xFF
            elif filter_type != 0:
                raise RuntimeError(f"Unsupported PNG filter {filter_type}")

        for x in range(width):
            if row[x * bytes_per_pixel + 3] != 0:
                min_x = min(min_x, x)
                min_y = min(min_y, y)
                max_x = max(max_x, x)
                max_y = max(max_y, y)
        previous = row

    if max_x < min_x or max_y < min_y:
        raise RuntimeError("macOS icon artwork is fully transparent")
    return width, height, min_x, min_y, max_x + 1, max_y + 1


def require_macos_icon_geometry(icon: Path) -> None:
    data = icon.read_bytes()
    if len(data) < 8 or data[:4] != b"icns":
        raise RuntimeError("macOS application icon is not an ICNS file")
    declared_length = struct.unpack(">I", data[4:8])[0]
    if declared_length != len(data):
        raise RuntimeError("macOS ICNS declared length does not match its file size")

    chunks: dict[bytes, bytes] = {}
    offset = 8
    while offset < len(data):
        if offset + 8 > len(data):
            raise RuntimeError("Truncated chunk header in macOS ICNS")
        chunk_type = data[offset : offset + 4]
        length = struct.unpack(">I", data[offset + 4 : offset + 8])[0]
        if length < 8 or offset + length > len(data):
            raise RuntimeError("Invalid chunk length in macOS ICNS")
        chunks[chunk_type] = data[offset + 8 : offset + length]
        offset += length

    expected_chunks = {
        b"ic07",
        b"ic08",
        b"ic09",
        b"ic10",
        b"ic11",
        b"ic12",
        b"ic13",
        b"ic14",
    }
    missing_chunks = expected_chunks - chunks.keys()
    if missing_chunks:
        raise RuntimeError(
            f"macOS ICNS is missing Retina representations: {sorted(missing_chunks)}"
        )

    width, height, left, top, right, bottom = png_alpha_bbox(chunks[b"ic10"])
    if (width, height) != (1024, 1024):
        raise RuntimeError(f"Unexpected largest macOS icon size: {width}x{height}")
    margins = (left, top, width - right, height - bottom)
    if any(margin < 90 or margin > 115 for margin in margins):
        raise RuntimeError(
            f"macOS icon artwork does not occupy the 80.5% safe area: {margins}"
        )
    if max(margins) - min(margins) > 2:
        raise RuntimeError(f"macOS icon artwork is not centered: {margins}")
    print(
        "Verified macOS icon Retina layers and 1024px safe-area margins: "
        f"{margins}"
    )


def run_font_verification(
    executable: Path,
    cwd: Path,
    qpa_platform: str | None = None,
) -> None:
    environment = os.environ.copy()
    if qpa_platform:
        environment.setdefault("QT_QPA_PLATFORM", qpa_platform)
    environment.setdefault("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-gpu")
    with tempfile.TemporaryDirectory(prefix="ghostwriter-mojikumi-smoke-") as data_home:
        if sys.platform == "win32":
            environment["APPDATA"] = str(Path(data_home) / "Roaming")
            environment["LOCALAPPDATA"] = str(Path(data_home) / "Local")
        elif sys.platform == "darwin":
            environment["HOME"] = data_home
        else:
            environment["XDG_CONFIG_HOME"] = str(Path(data_home) / "config")
            environment["XDG_DATA_HOME"] = str(Path(data_home) / "data")
            environment["XDG_CACHE_HOME"] = str(Path(data_home) / "cache")

        for locale in ("zh_CN", "ja_JP", "ko_KR", "zh_TW", "zh_HK"):
            completed = subprocess.run(
                [str(executable), "--verify-bundled-font", locale],
                cwd=cwd,
                env=environment,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                encoding="utf-8",
                errors="replace",
                timeout=90,
                check=False,
            )
            output = completed.stdout.strip()
            print(output)
            if completed.returncode != 0 or "bundled-font-ok" not in output:
                raise RuntimeError(
                    "Bundled font runtime verification failed for "
                    f"{locale} with {completed.returncode}"
                )


def run_translation_verification(
    executable: Path,
    cwd: Path,
    qpa_platform: str | None = None,
) -> None:
    environment = os.environ.copy()
    if qpa_platform:
        environment.setdefault("QT_QPA_PLATFORM", qpa_platform)
    completed = subprocess.run(
        [str(executable), "--verify-translations"],
        cwd=cwd,
        env=environment,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        encoding="utf-8",
        errors="replace",
        timeout=30,
        check=False,
    )
    output = completed.stdout.strip()
    print(output)
    if completed.returncode != 0 or "translations-ok" not in output:
        raise RuntimeError(
            "Packaged translation runtime verification failed with "
            f"{completed.returncode}"
        )


def run_color_scheme_verification(
    executable: Path,
    cwd: Path,
    qpa_platform: str | None = None,
) -> None:
    environment = os.environ.copy()
    if qpa_platform:
        environment.setdefault("QT_QPA_PLATFORM", qpa_platform)
    completed = subprocess.run(
        [str(executable), "--verify-color-schemes"],
        cwd=cwd,
        env=environment,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        encoding="utf-8",
        errors="replace",
        timeout=30,
        check=False,
    )
    output = completed.stdout.strip()
    print(output)
    if completed.returncode != 0 or "color-schemes-ok" not in output:
        raise RuntimeError(
            "Native color-scheme runtime verification failed with "
            f"{completed.returncode}"
        )


def run_preview_background_verification(
    executable: Path,
    cwd: Path,
    qpa_platform: str | None = None,
) -> None:
    environment = os.environ.copy()
    if qpa_platform:
        environment.setdefault("QT_QPA_PLATFORM", qpa_platform)
    environment.setdefault("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-gpu")
    completed = subprocess.run(
        [str(executable), "--verify-preview-backgrounds"],
        cwd=cwd,
        env=environment,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        encoding="utf-8",
        errors="replace",
        timeout=90,
        check=False,
    )
    output = completed.stdout.strip()
    print(output)
    if completed.returncode != 0 or "preview-backgrounds-ok" not in output:
        raise RuntimeError(
            "Live-preview background runtime verification failed with "
            f"{completed.returncode}"
        )


def require_packaged_translations(entries: list[Path]) -> None:
    required_locales = ("en", "ja", "ko", "zh_CN", "zh_TW")
    normalized_files = {
        "/".join(part.casefold() for part in path.parts)
        for path in entries
        if path.is_file() and path.name.casefold() == "ghostwriter_qt.qm"
    }
    for locale in required_locales:
        suffix = (
            f"locale/{locale}/LC_MESSAGES/ghostwriter_qt.qm".casefold()
        )
        if not any(path.endswith(suffix) for path in normalized_files):
            raise RuntimeError(f"Missing packaged {locale} interface translation")
    print(
        "Verified packaged interface translations: "
        + ", ".join(required_locales)
    )


def require_windows_icon(executable: Path) -> None:
    script = r"""
Add-Type -AssemblyName System.Drawing
$icon = [System.Drawing.Icon]::ExtractAssociatedIcon($env:MOJIKUMI_EXE)
if ($null -eq $icon) { throw 'No associated executable icon' }
$bitmap = $icon.ToBitmap()
try {
  $defaultLike = 0
  $ghostLike = 0
  for ($y = 0; $y -lt $bitmap.Height; $y++) {
    for ($x = 0; $x -lt $bitmap.Width; $x++) {
      $pixel = $bitmap.GetPixel($x, $y)
      if ($pixel.A -eq 0) { continue }
      if ($pixel.R -ge 170 -and $pixel.G -le 120 -and $pixel.B -le 120) {
        $ghostLike++
      }
      if ($pixel.B -ge 120 -and $pixel.B -gt ($pixel.R + 20)) {
        $defaultLike++
      }
    }
  }
  if ($ghostLike -lt 25 -or $defaultLike -gt $ghostLike) {
    throw "Executable icon does not match the red ghost artwork (red=$ghostLike blue=$defaultLike)"
  }
  Write-Output "Verified executable icon: $($icon.Width)x$($icon.Height), red=$ghostLike"
} finally {
  $bitmap.Dispose()
  $icon.Dispose()
}
"""
    completed = subprocess.run(
        ["powershell", "-NoProfile", "-NonInteractive", "-Command", script],
        env={**os.environ, "MOJIKUMI_EXE": str(executable)},
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        encoding="utf-8",
        errors="replace",
        timeout=30,
        check=False,
    )
    print(completed.stdout.strip())
    if completed.returncode != 0:
        raise RuntimeError("Windows executable icon verification failed")


def require_webengine_payload(entries: list[Path], platform: str) -> None:
    helper_name = "QtWebEngineProcess.exe" if platform == "windows" else "QtWebEngineProcess"
    require("Qt WebEngine process helper", named_files(entries, helper_name))

    if platform == "windows":
        require("Qt6WebEngineCore.dll", named_files(entries, "Qt6WebEngineCore.dll"))
    elif platform == "macos":
        require(
            "QtWebEngineCore framework",
            matching(entries, lambda path: path.name == "QtWebEngineCore.framework"),
        )
    else:
        require(
            "libQt6WebEngineCore shared library",
            matching(
                entries,
                lambda path: path.name.startswith("libQt6WebEngineCore.so"),
            ),
        )

    require(
        "Qt WebEngine resources",
        named_files(entries, "qtwebengine_resources.pak"),
    )
    if platform == "linux":
        # Craft's Linux Qt WebEngine is built against its shared ICU runtime,
        # so Chromium's standalone icudtl.dat is intentionally not installed.
        require(
            "shared ICU data library",
            matching(
                entries,
                lambda path: path.is_file()
                and path.name.startswith("libicudata.so"),
            ),
        )
    else:
        require("Qt WebEngine ICU data", named_files(entries, "icudtl.dat"))
    require(
        "Qt WebEngine locale pack",
        matching(
            entries,
            lambda path: path.is_file()
            and path.suffix.casefold() == ".pak"
            and any(
                parent.name.casefold() in {"locales", "qtwebengine_locales"}
                for parent in path.parents
            ),
        ),
    )


def run_version(
    executable: Path,
    expected_version: str,
    cwd: Path,
    qpa_platform: str | None = None,
) -> None:
    environment = os.environ.copy()
    if qpa_platform:
        environment.setdefault("QT_QPA_PLATFORM", qpa_platform)
    completed = subprocess.run(
        [str(executable), "--version"],
        cwd=cwd,
        env=environment,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        encoding="utf-8",
        errors="replace",
        timeout=30,
        check=False,
    )
    output = completed.stdout.strip()
    print(output)
    if completed.returncode != 0:
        raise RuntimeError(
            f"{executable} --version exited with {completed.returncode}"
        )
    if expected_version not in output:
        raise RuntimeError(
            f"Expected version {expected_version!r} was not present in output"
        )


def verify_windows(root: Path, expected_version: str) -> None:
    entries = all_entries(root)
    executable = require(
        "ghostwriter.exe",
        named_files(entries, "ghostwriter.exe"),
    )
    require("cmark-gfm runtime", named_files(entries, "cmark-gfm.dll"))
    require(
        "cmark-gfm extensions runtime",
        named_files(entries, "cmark-gfm-extensions.dll"),
    )
    require_webengine_payload(entries, "windows")
    require_packaged_translations(entries)
    require_windows_icon(executable)
    run_version(executable, expected_version, executable.parent)
    run_preview_background_verification(executable, executable.parent)
    run_color_scheme_verification(executable, executable.parent)
    run_translation_verification(executable, executable.parent)
    run_font_verification(executable, executable.parent)


def verify_macos(root: Path, expected_version: str) -> None:
    applications = matching(
        all_entries(root),
        lambda path: path.is_dir()
        and path.suffix.casefold() == ".app"
        and not any(
            parent != root and parent.suffix.casefold() == ".app"
            for parent in path.parents
        ),
    )
    application = require("top-level application bundle", applications)
    info_path = application / "Contents" / "Info.plist"
    if not info_path.is_file():
        raise RuntimeError(f"Missing application Info.plist: {info_path}")

    with info_path.open("rb") as info_file:
        info = plistlib.load(info_file)
    expected_metadata = {
        "CFBundleIdentifier": "io.github.mr_drinking.ghostwriter-mojikumi",
        "CFBundleDisplayName": "Ghostwriter Mojikumi",
        "CFBundleName": "Ghostwriter Mojikumi",
        "CFBundleShortVersionString": expected_version,
        "CFBundleVersion": expected_version,
    }
    for key, expected_value in expected_metadata.items():
        actual_value = info.get(key)
        if actual_value != expected_value:
            raise RuntimeError(
                f"Unexpected {key} in {info_path}: "
                f"{actual_value!r}, expected {expected_value!r}"
            )
    if info.get("NSRequiresAquaSystemAppearance") is True:
        raise RuntimeError(
            "macOS bundle opts out of native dark appearance support"
        )
    print("Verified macOS bundle identity and version metadata")

    executable_name = info.get("CFBundleExecutable")
    if not executable_name:
        raise RuntimeError(f"CFBundleExecutable is missing from {info_path}")

    icon_name = info.get("CFBundleIconFile")
    if not icon_name:
        raise RuntimeError(f"CFBundleIconFile is missing from {info_path}")
    if not str(icon_name).casefold().endswith(".icns"):
        icon_name = f"{icon_name}.icns"
    icon = application / "Contents" / "Resources" / str(icon_name)
    if not icon.is_file():
        raise RuntimeError(f"Missing declared macOS application icon: {icon}")
    print(f"Found declared macOS application icon: {icon}")
    icon_digest = sha256(icon)
    if icon_digest != MACOS_ICON_SHA256:
        raise RuntimeError(f"macOS application icon SHA-256 mismatch: {icon_digest}")
    print(f"Verified macOS application icon SHA-256: {icon_digest}")
    require_macos_icon_geometry(icon)

    executable = application / "Contents" / "MacOS" / executable_name
    if not executable.is_file() or not os.access(executable, os.X_OK):
        raise RuntimeError(f"Missing executable application entry point: {executable}")
    print(f"Found application entry point: {executable}")

    entries = all_entries(application)
    helper = require(
        "Qt WebEngine process helper",
        named_files(entries, "QtWebEngineProcess"),
    )
    helper_dependencies = subprocess.run(
        ["otool", "-L", str(helper)],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        encoding="utf-8",
        errors="replace",
        timeout=30,
        check=False,
    )
    if helper_dependencies.returncode != 0:
        raise RuntimeError(
            "Could not inspect Qt WebEngine helper dependencies: "
            f"{helper_dependencies.stdout}"
        )
    if "@executable_path/../Frameworks/" in helper_dependencies.stdout:
        raise RuntimeError(
            "Qt WebEngine helper still resolves frameworks relative to its nested executable"
        )
    if "@loader_path/../../../../../../../QtWebEngineCore.framework/" not in helper_dependencies.stdout:
        raise RuntimeError(
            "Qt WebEngine helper does not resolve QtWebEngineCore from the outer application bundle"
        )
    print("Verified nested Qt WebEngine helper framework references")

    require_webengine_payload(entries, "macos")
    require_packaged_translations(entries)
    run_version(executable, expected_version, application)
    run_preview_background_verification(executable, application)
    run_color_scheme_verification(executable, application)
    run_translation_verification(executable, application)
    run_font_verification(executable, application)


def verify_linux(root: Path, expected_version: str) -> None:
    app_run = root / "AppRun"
    if not app_run.is_file() or not os.access(app_run, os.X_OK):
        raise RuntimeError(f"Missing executable AppRun: {app_run}")
    print(f"Found AppRun: {app_run}")

    entries = all_entries(root)
    icon_files = matching(
        entries,
        lambda path: path.is_file()
        and path.suffix.casefold() in {".png", ".svg"}
        and "ghostwriter" in path.name.casefold()
        and "icons" in {parent.name.casefold() for parent in path.parents},
    )
    icon = require("Linux desktop icon", icon_files)
    packaged_256_icons = [
        path for path in icon_files
        if path.name.casefold() == "ghostwriter.png"
        and "256x256" in {parent.name.casefold() for parent in path.parents}
    ]
    if packaged_256_icons:
        icon = packaged_256_icons[0]
    icon_digest = sha256(icon)
    if icon_digest != LINUX_ICON_SHA256:
        raise RuntimeError(f"Linux desktop icon SHA-256 mismatch: {icon_digest}")
    print(f"Verified Linux desktop icon SHA-256: {icon_digest}")
    require("ghostwriter executable", named_files(entries, "ghostwriter"))
    require_webengine_payload(entries, "linux")
    require_packaged_translations(entries)
    run_version(app_run, expected_version, root, qpa_platform="offscreen")
    run_preview_background_verification(
        app_run,
        root,
        qpa_platform="offscreen",
    )
    run_translation_verification(app_run, root, qpa_platform="offscreen")
    run_font_verification(app_run, root, qpa_platform="offscreen")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("platform", choices=("windows", "macos", "linux"))
    parser.add_argument("root", type=Path)
    parser.add_argument("expected_version")
    args = parser.parse_args()

    root = args.root.resolve()
    if not root.is_dir():
        parser.error(f"package root does not exist: {root}")

    verifiers = {
        "windows": verify_windows,
        "macos": verify_macos,
        "linux": verify_linux,
    }
    verifiers[args.platform](root, args.expected_version)
    print(f"{args.platform} package smoke test passed")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, RuntimeError, subprocess.SubprocessError) as error:
        print(f"Package verification failed: {error}", file=sys.stderr)
        sys.exit(1)
