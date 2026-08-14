# SPDX-FileCopyrightText: 2026 Ghostwriter Mojikumi contributors
# SPDX-License-Identifier: BSD-2-Clause

import os
from pathlib import Path

import info
import utils
from CraftCore import CraftCore
from Package.CMakePackageBase import CMakePackageBase
from Utils import CodeSign


class subinfo(info.infoclass):
    def setTargets(self):
        # CI replaces the source with this repository checkout through srcDir.
        self.svnTargets["26.08.0"] = (
            "[git]https://github.com/Mr-Drinking/Ghostwriter-Mojikumi.git|main|"
        )
        self.defaultTarget = "26.08.0"
        self.displayName = "Ghostwriter Mojikumi"
        self.description = "A distraction-free Markdown editor with CJK mojikumi"
        self.webpage = "https://github.com/Mr-Drinking/Ghostwriter-Mojikumi"

    def setDependencies(self):
        self.buildDependencies["virtual/base"] = None
        self.buildDependencies["kde/frameworks/extra-cmake-modules"] = None

        if CraftCore.compiler.isLinux:
            self.buildDependencies["dev-utils/linuxdeploy"] = None
            # QtWayland's Craft image also contains the compositor library,
            # whose libwayland-server dependency must be available while
            # linuxdeploy resolves and collects the AppImage runtime.
            self.runtimeDependencies["libs/wayland"] = None

        # Craft's public Qt package paths are virtual aliases that resolve to
        # their Qt 6 implementation in a Qt6 Craft root.
        self.runtimeDependencies["libs/qt/qtbase"] = None
        self.runtimeDependencies["libs/qt/qtsvg"] = None
        self.runtimeDependencies["libs/qt/qttranslations"] = None
        self.runtimeDependencies["libs/qt/qtwebchannel"] = None
        self.runtimeDependencies["libs/qt/qtwebengine"] = None

        self.runtimeDependencies["kde/frameworks/tier1/kconfig"] = None
        self.runtimeDependencies["kde/frameworks/tier1/kcoreaddons"] = None
        self.runtimeDependencies["kde/frameworks/tier1/sonnet"] = None
        self.runtimeDependencies["kde/frameworks/tier1/kwidgetsaddons"] = None
        self.runtimeDependencies["kde/frameworks/tier3/kconfigwidgets"] = None
        self.runtimeDependencies["kde/frameworks/tier3/kxmlgui"] = None


class Package(CMakePackageBase):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        build_testing = "ON" if CraftCore.compiler.isLinux else "OFF"
        self.subinfo.options.configure.args += [
            f"-DBUILD_TESTING={build_testing}",
            "-DQT_MAJOR_VERSION=6",
        ]

    def internalCreatePackage(self, defines):
        if not super().internalCreatePackage(defines):
            return False
        if not CraftCore.compiler.isMacOS:
            return True

        app_path = self.getMacAppPath(defines)
        if not app_path:
            return False

        # MacBasePackager treats every Mach-O as though it were launched from
        # the outer Contents/MacOS directory. That is not true for the nested
        # QtWebEngineProcess helper, and @executable_path is inherited by all
        # frameworks loaded into that process. Convert every package-internal
        # framework reference to the equivalent path relative to the Mach-O
        # that owns it. This covers the helper and all of its transitive Qt,
        # KF, ICU and GLib dependencies without introducing directory cycles.
        contents = app_path / "Contents"
        frameworks = contents / "Frameworks"
        old_prefix = "@executable_path/../Frameworks/"
        changed_binary_count = 0
        changed_reference_count = 0

        binaries = list(
            utils.filterDirectoryContent(
                contents,
                whitelist=lambda entry, root: utils.isBinary(entry.path),
                blacklist=lambda entry, root: True,
            )
        )
        for binary_name in binaries:
            binary = Path(binary_name)
            changes = []
            loader_frameworks = os.path.relpath(frameworks, binary.parent).replace(
                os.sep, "/"
            )
            for dependency in utils.getLibraryDeps(str(binary)):
                if dependency.startswith(old_prefix):
                    changes.append(
                        (
                            dependency,
                            f"@loader_path/{loader_frameworks}/"
                            + dependency.removeprefix(old_prefix),
                        )
                    )

            if not changes:
                continue
            command = ["install_name_tool"]
            for old_reference, new_reference in changes:
                command += ["-change", old_reference, new_reference]
            command.append(str(binary))
            with utils.makeTemporaryWritable(binary):
                if not utils.system(command):
                    return False
            changed_binary_count += 1
            changed_reference_count += len(changes)

        if changed_reference_count == 0:
            CraftCore.log.error(
                "No macOS package-internal framework references were rewritten"
            )
            return False

        CraftCore.log.info(
            "Rewrote %d macOS framework references across %d binaries",
            changed_reference_count,
            changed_binary_count,
        )

        # install_name_tool invalidates the signatures produced by Craft's
        # generic bundler, so sign the complete nested bundle hierarchy again.
        return CodeSign.signMacApp(app_path)

    def createPackage(self):
        self.defines["appname"] = "ghostwriter"
        self.defines["productname"] = "Ghostwriter Mojikumi"
        self.defines["display_name"] = "Ghostwriter Mojikumi"
        self.defines["description"] = self.subinfo.description
        self.defines["website"] = self.subinfo.webpage
        self.defines["license"] = self.sourceDir() / "COPYING"
        self.defines["desktopFile"] = "io.github.mr_drinking.ghostwriter-mojikumi"

        if CraftCore.compiler.isWindows:
            self.defines["company"] = "Mr-Drinking"
            self.defines["icon"] = self.sourceDir() / "resources/windows/ghostwriter.ico"
            # Craft's automatic shortcut generated from ``executable`` leaves
            # IconLocation empty.  Define the shortcut ourselves so Windows'
            # Start menu always has an explicit, installed icon to resolve.
            self.defines["shortcuts"] = [
                {
                    "name": "Ghostwriter Mojikumi",
                    "target": "bin/ghostwriter.exe",
                    "icon": r"$INSTDIR\ghostwriter.ico",
                    "description": self.subinfo.description,
                }
            ]
        else:
            self.defines["executable"] = "bin/ghostwriter"

        return super().createPackage()
