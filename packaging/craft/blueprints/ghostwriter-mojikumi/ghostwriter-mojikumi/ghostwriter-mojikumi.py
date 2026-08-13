# SPDX-FileCopyrightText: 2026 Ghostwriter Mojikumi contributors
# SPDX-License-Identifier: BSD-2-Clause

import subprocess

import info
import utils
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

        # Craft relocates QtWebEngineCore.framework into the outer app's
        # Contents/Frameworks directory. Its generic dylib bundler rewrites
        # every @rpath dependency as if the executable lived directly in
        # Contents/MacOS, but QtWebEngineProcess is a deeply nested helper app.
        # Preserve the helper's relationship to the outer Frameworks folder
        # with @loader_path before the generic pass sees these references.
        helper = (
            self.archiveDir()
            / "lib/QtWebEngineCore.framework/Versions/A/Helpers"
            / "QtWebEngineProcess.app/Contents/MacOS/QtWebEngineProcess"
        )
        if not helper.is_file():
            CraftCore.log.error(f"Missing macOS Qt WebEngine helper: {helper}")
            return False

        output = subprocess.check_output(
            ["otool", "-L", str(helper)],
            text=True,
        )
        rpath_dependencies = []
        for line in output.splitlines()[1:]:
            dependency = line.strip().split(" (compatibility version", 1)[0]
            if dependency.startswith("@rpath/"):
                rpath_dependencies.append(dependency)

        if not rpath_dependencies:
            CraftCore.log.error(
                f"No @rpath dependencies found in macOS Qt WebEngine helper: {helper}"
            )
            return False

        # Contents/MacOS -> helper Contents -> helper .app -> Helpers -> A ->
        # Versions -> QtWebEngineCore.framework -> outer Contents/Frameworks.
        loader_frameworks = "@loader_path/../../../../../../../"
        command = ["install_name_tool"]
        for dependency in rpath_dependencies:
            command += [
                "-change",
                dependency,
                loader_frameworks + dependency.removeprefix("@rpath/"),
            ]
        command.append(str(helper))
        return utils.system(command)

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
