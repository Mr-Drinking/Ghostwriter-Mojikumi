# SPDX-FileCopyrightText: 2026 Mr-Drinking
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)

project(mojikumitest VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 6.11 REQUIRED COMPONENTS Core Gui Test Widgets)

qt_standard_project_setup()

set(MOJIKUMI_TEST_FONT
    "${CMAKE_CURRENT_SOURCE_DIR}/../../resources/fonts/NotoSansCJK-Regular.ttc")

if(NOT EXISTS "${MOJIKUMI_TEST_FONT}")
    message(FATAL_ERROR
        "The mojikumi regression tests require the bundled font at "
        "${MOJIKUMI_TEST_FONT}")
endif()

ecm_add_test(
    mojikumitest.cpp
    ../../src/editor/mojikumidecorator.h
    ../../src/editor/mojikumidecorator.cpp
    ../../src/settings/bundledfont.h
    LINK_LIBRARIES
        PRIVATE
            Qt::Core
            Qt::Gui
            Qt::Test
            Qt::Widgets
    TEST_NAME mojikumitest
)

target_compile_definitions(mojikumitest PRIVATE
    MOJIKUMI_TEST_FONT_PATH="${MOJIKUMI_TEST_FONT}")

target_include_directories(mojikumitest PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src")

set_tests_properties(mojikumitest PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

enable_testing(true)
