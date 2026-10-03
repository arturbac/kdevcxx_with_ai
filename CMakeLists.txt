cmake_minimum_required(VERSION 3.25)

include(FeatureSummary)
include(cmake/CPM.cmake)

project(kdevcxx_with_ai LANGUAGES CXX
    HOMEPAGE_URL "https://github.com/arturbac/kdevcxx_with_ai"
    DESCRIPTION "KDevelop client for Claude Code"
    VERSION 1.0.0
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Prevent in-source builds
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR
        "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
    )
endif()

# KDE settings are global so the tests get the same Qt defines as the code they test
find_package(ECM "6.6.0" CONFIG REQUIRED)

list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs6)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets WebSockets Test)
find_package(KF6 REQUIRED COMPONENTS
    Config
    CoreAddons
    I18n
    Parts
    TextEditor
    XmlGui
)
find_package(KDevPlatform 6.0.0 REQUIRED)

set(CMAKE_AUTOMOC ON)

add_subdirectory(src)

enable_testing()
include(CTest)
include(cmake/add_engine_ut.cmake)
add_subdirectory(unittests)

include(cmake/spdx_check.cmake)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
