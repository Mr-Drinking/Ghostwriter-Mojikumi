# SPDX-FileCopyrightText: 2026 Ghostwriter Mojikumi contributors
# SPDX-License-Identifier: BSD-2-Clause

"""Apply the small set of deterministic Craft settings used by CI."""

from __future__ import annotations

import argparse
import configparser
from pathlib import Path


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--settings", type=Path, required=True)
    parser.add_argument("--expected-abi", required=True)
    parser.add_argument("--blueprints", type=Path, required=True)
    parser.add_argument("--destination", type=Path, required=True)
    parser.add_argument("--package-type", required=True)
    parser.add_argument("--archive-type", default="")
    args = parser.parse_args()

    settings = configparser.ConfigParser(interpolation=None)
    settings.optionxform = str
    with args.settings.open(encoding="utf-8") as handle:
        settings.read_file(handle)

    actual_abi = settings["General"]["ABI"]
    if actual_abi != args.expected_abi:
        raise SystemExit(
            f"Craft bootstrap selected ABI {actual_abi!r}; "
            f"runner requires {args.expected_abi!r}"
        )

    args.destination.mkdir(parents=True, exist_ok=True)

    settings["General"]["KFHostToolingVersion"] = "6"
    settings["Blueprints"]["Locations"] = str(args.blueprints.resolve())
    settings["BlueprintVersions"]["EnableDailyUpdates"] = "False"
    # KDE's hosted Craft binary cache is published as RelWithDebInfo.  Keep
    # build and package actions on that exact type so the collection packager
    # reads the same dependency image directories that Craft downloaded.
    settings["Compile"]["BuildType"] = "RelWithDebInfo"
    settings["Compile"]["UseNinja"] = "True"
    settings["ContinuousIntegration"]["Enabled"] = "True"
    settings["ContinuousIntegration"]["OutputOnFailure"] = "True"
    settings["Packager"]["Destination"] = str(args.destination.resolve())
    settings["Packager"]["PackageType"] = args.package_type
    settings["Packager"]["PackageSrc"] = "False"
    settings["Packager"]["PackageDebugSymbols"] = "False"
    settings["Packager"]["RepositoryUrl"] = "https://files.kde.org/craft/Qt6/"
    settings["Packager"]["UseCache"] = "True"
    settings["CodeSigning"]["Enabled"] = "False"
    if args.archive_type:
        settings["Packager"]["7ZipArchiveType"] = args.archive_type

    with args.settings.open("w", encoding="utf-8", newline="\n") as handle:
        settings.write(handle)


if __name__ == "__main__":
    main()
