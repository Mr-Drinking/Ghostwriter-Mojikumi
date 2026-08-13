# SPDX-FileCopyrightText: 2026 Ghostwriter Mojikumi contributors
# SPDX-License-Identifier: BSD-2-Clause

"""Perform release-package content and command-line smoke checks."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import plistlib
import subprocess
import sys


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
    run_version(executable, expected_version, executable.parent)


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
        executable_name = plistlib.load(info_file).get("CFBundleExecutable")
    if not executable_name:
        raise RuntimeError(f"CFBundleExecutable is missing from {info_path}")

    executable = application / "Contents" / "MacOS" / executable_name
    if not executable.is_file() or not os.access(executable, os.X_OK):
        raise RuntimeError(f"Missing executable application entry point: {executable}")
    print(f"Found application entry point: {executable}")

    entries = all_entries(application)
    require_webengine_payload(entries, "macos")
    run_version(executable, expected_version, application)


def verify_linux(root: Path, expected_version: str) -> None:
    app_run = root / "AppRun"
    if not app_run.is_file() or not os.access(app_run, os.X_OK):
        raise RuntimeError(f"Missing executable AppRun: {app_run}")
    print(f"Found AppRun: {app_run}")

    entries = all_entries(root)
    require("ghostwriter executable", named_files(entries, "ghostwriter"))
    require_webengine_payload(entries, "linux")
    run_version(app_run, expected_version, root, qpa_platform="offscreen")


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
