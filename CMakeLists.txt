# SPDX-FileCopyrightText: 2022-2024 Megan Conkle <megan.conkle@kdemail.net>
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)
if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING
            "Default build type: Release" FORCE)
endif()

if (NOT BUILD_TESTING)
    set (BUILD_TESTING OFF)
endif()

message("Generating makefile for ${CMAKE_BUILD_TYPE} build...")

# KF6 only
set(QT_MAJOR_VERSION 6)
set(QT_MIN_VERSION "6.11.0")
set(KF_MAJOR_VERSION 6)
set(KF_DEP_VERSION "6.0.0")
set(KF_MIN_VERSION "6.0.0")

# use C++20 like KF6 itself
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# KDE Application Version, managed by release script
set(RELEASE_SERVICE_VERSION_MAJOR "26")
set(RELEASE_SERVICE_VERSION_MINOR "08")
set(RELEASE_SERVICE_VERSION_MICRO "0")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
set(MOJIKUMI_RELEASE_VERSION "${RELEASE_SERVICE_VERSION}")

project(ghostwriter VERSION ${RELEASE_SERVICE_VERSION} LANGUAGES CXX)

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)


# Append to the module path so modules can be overridden from the command line.
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
remove_definitions(-DQT_NO_KEYWORDS)
remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
include(KDECMakeSettings)
include(ECMInstallIcons)
include(ECMAddAppIcon)
include(ECMAddTests)
include(ECMPoQmTools)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(FeatureSummary)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/poqm")
    ecm_install_po_files_as_qm(poqm)
endif()

find_package(KF${KF_MAJOR_VERSION}DocTools ${KF_MIN_VERSION})
set_package_properties(KF${KF_MAJOR_VERSION}DocTools PROPERTIES DESCRIPTION
    "Tools to generate documentation"
    TYPE OPTIONAL
)

set(CMARK_TESTS OFF)
add_subdirectory(3rdparty/cmark-gfm EXCLUDE_FROM_ALL)
add_subdirectory(src)

if(KF${QT_MAJOR_VERSION}DocTools_FOUND)
    kdoctools_install(po)
    add_subdirectory(doc)
endif()

# Add icon files to the application's source files to have CMake bundle them in the executable.
set(ICONS_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/resources/icons)

set(ICON_FILES
    ${ICONS_FOLDER}/sc-apps-ghostwriter.svg
    ${ICONS_FOLDER}/16-apps-ghostwriter.png
    ${ICONS_FOLDER}/22-apps-ghostwriter.png
    ${ICONS_FOLDER}/32-apps-ghostwriter.png
    ${ICONS_FOLDER}/64-apps-ghostwriter.png
    ${ICONS_FOLDER}/128-apps-ghostwriter.png
    ${ICONS_FOLDER}/256-apps-ghostwriter.png
)

if(WIN32)
    # ECM's generated Windows resource was not embedded by the hosted MSVC
    # build; the resulting executable displayed Windows' generic icon. Use
    # the verified multi-resolution ICO explicitly for the PE resource.
    enable_language(RC)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/windows/ghostwriter.ico
        ${CMAKE_CURRENT_BINARY_DIR}/ghostwriter.ico
        COPYONLY)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/windows/ghostwriter.rc
        ${CMAKE_CURRENT_BINARY_DIR}/ghostwriter.rc
        COPYONLY)
    target_sources(ghostwriter PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/ghostwriter.rc)
elseif(APPLE)
    # The ICNS is attached in src/CMakeLists.txt, in the same directory scope
    # where the application target is created. CMake source-file properties
    # are directory-scoped, so setting MACOSX_PACKAGE_LOCATION here would put
    # CFBundleIconFile in Info.plist without copying the file into the bundle.
else()
    ecm_add_app_icon(ICONS_SOURCES ICONS ${ICON_FILES})
    target_sources(ghostwriter PRIVATE ${ICONS_SOURCES})
endif()

if(BUILD_TESTING)
   # CTest looks for its root test file in the top-level build directory.
   # Enabling it only inside autotest would let `ctest` report success while
   # discovering no tests when Craft invokes it from the build root.
   enable_testing()
   add_subdirectory(autotest)
endif()


# See https://cmake.org/cmake/help/v3.15/prop_tgt/MACOSX_BUNDLE_INFO_PLIST.html
if(APPLE)
    string(TIMESTAMP CURRENT_YEAR "%Y")

    # CMake only substitutes these documented values from target properties
    # (or variables in the directory where the target was created). This
    # target lives in src/, so parent-directory variables silently produced
    # empty plist values in the packaged application. Set every value on the
    # target itself so both Craft macOS architectures receive the same plist.
    set_target_properties(ghostwriter PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST
            ${CMAKE_CURRENT_SOURCE_DIR}/resources/mac/MacOSBundleInfo.plist.in
        MACOSX_BUNDLE_ICON_FILE "ghostwriter.icns"
        MACOSX_BUNDLE_GUI_IDENTIFIER
            "io.github.mr_drinking.ghostwriter-mojikumi"
        MACOSX_BUNDLE_BUNDLE_NAME "Ghostwriter Mojikumi"
        MACOSX_BUNDLE_INFO_STRING
            "Ghostwriter Mojikumi"
        MACOSX_BUNDLE_LONG_VERSION_STRING
            "Ghostwriter Mojikumi ${MOJIKUMI_RELEASE_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${RELEASE_SERVICE_VERSION}"
        # CFBundleVersion must use Apple's numeric syntax.
        MACOSX_BUNDLE_BUNDLE_VERSION "${RELEASE_SERVICE_VERSION}"
        MACOSX_BUNDLE_COPYRIGHT
            "2014-${CURRENT_YEAR} the ghostwriter authors; Mojikumi modifications 2026 Mr-Drinking"
    )
endif()

install(TARGETS ghostwriter ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

# cmark-gfm is added with EXCLUDE_FROM_ALL, which deliberately suppresses all
# install rules declared inside that subdirectory. MSVC cannot link it
# statically, so explicitly install the two runtime DLLs ghostwriter imports.
if(MSVC)
    install(
        TARGETS
            libcmark-gfm
            libcmark-gfm-extensions
        RUNTIME DESTINATION ${KDE_INSTALL_BINDIR}
    )
endif()

ecm_install_icons(
    ICONS ${ICON_FILES}
    DESTINATION ${KDE_INSTALL_ICONDIR}
    THEME hicolor
)

install(
    PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/resources/linux/io.github.mr_drinking.ghostwriter-mojikumi.desktop
    DESTINATION ${KDE_INSTALL_APPDIR}
)

install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/linux/io.github.mr_drinking.ghostwriter-mojikumi.metainfo.xml
    DESTINATION ${KDE_INSTALL_METAINFODIR}
)

install(
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/ABOUT.md
        ${CMAKE_CURRENT_SOURCE_DIR}/THIRD_PARTY_NOTICES.md
    DESTINATION ${KDE_INSTALL_DATADIR}/ghostwriter-mojikumi
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/LICENSES/
    DESTINATION ${KDE_INSTALL_DATADIR}/ghostwriter-mojikumi/LICENSES
)

install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/COPYING
    DESTINATION ${KDE_INSTALL_DATADIR}/ghostwriter-mojikumi
)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES src/**/*.cpp src/**/*.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
