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
        self.subinfo.options.configure.args += [
            "-DBUILD_TESTING=OFF",
            "-DQT_MAJOR_VERSION=6",
        ]

    def createPackage(self):
        self.defines["appname"] = "ghostwriter"
        self.defines["productname"] = "Ghostwriter Mojikumi"
        self.defines["display_name"] = "Ghostwriter Mojikumi"
        self.defines["description"] = self.subinfo.description
        self.defines["website"] = self.subinfo.webpage
        self.defines["license"] = self.sourceDir() / "COPYING"
        self.defines["desktopFile"] = "io.github.mr_drinking.ghostwritermojikumi"

        if CraftCore.compiler.isWindows:
            self.defines["executable"] = "bin/ghostwriter.exe"
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
