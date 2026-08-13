# SPDX-FileCopyrightText: 2026 Ghostwriter Mojikumi contributors
# SPDX-License-Identifier: BSD-2-Clause

import info
from CraftCore import CraftCore
from Package.CMakePackageBase import CMakePackageBase


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

    def preArchive(self):
        if not CraftCore.compiler.isMacOS:
            return True

        # Craft correctly relocates every framework into the outer app's
        # Contents/Frameworks directory, but its generic fix-up gives binaries
        # loaded by the nested QtWebEngineProcess helper paths relative to that
        # helper's executable. Restore the conventional nested-bundle view by
        # linking helper Contents/Frameworks back to the outer Frameworks dir.
        helper_contents = (
            self.archiveDir()
            / "lib/QtWebEngineCore.framework/Versions/A/Helpers"
            / "QtWebEngineProcess.app/Contents"
        )
        if not helper_contents.is_dir():
            CraftCore.log.error(
                f"Missing macOS Qt WebEngine helper bundle: {helper_contents}"
            )
            return False

        frameworks_link = helper_contents / "Frameworks"
        if frameworks_link.exists() or frameworks_link.is_symlink():
            CraftCore.log.error(
                f"Unexpected existing Qt WebEngine helper framework path: {frameworks_link}"
            )
            return False

        # Six parent traversals from helper Contents arrive at archive/lib,
        # which MacBasePackager later moves to outer Contents/Frameworks.
        frameworks_link.symlink_to("../../../../../..", target_is_directory=True)
        return frameworks_link.is_symlink()

    def createPackage(self):
        self.defines["appname"] = "ghostwriter"
        self.defines["productname"] = "Ghostwriter Mojikumi"
        self.defines["display_name"] = "Ghostwriter Mojikumi"
        self.defines["description"] = self.subinfo.description
        self.defines["website"] = self.subinfo.webpage
        self.defines["license"] = self.sourceDir() / "COPYING"
        self.defines["desktopFile"] = "io.github.mr_drinking.ghostwriter-mojikumi"

        if CraftCore.compiler.isWindows:
            self.defines["executable"] = "bin/ghostwriter.exe"
            self.defines["company"] = "Mr-Drinking"
            self.defines["icon"] = self.sourceDir() / "resources/windows/ghostwriter.ico"
            self.defines["shortcuts"] = [
                {
                    "name": "Ghostwriter Mojikumi",
                    "target": "bin/ghostwriter.exe",
                    "description": self.subinfo.description,
                }
            ]
        else:
            self.defines["executable"] = "bin/ghostwriter"

        return super().createPackage()
