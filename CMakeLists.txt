cmake_minimum_required(VERSION 3.31)

include(FeatureSummary)
include(cmake/CPM.cmake)

project(kdevcxx_with_ai LANGUAGES CXX
    HOMEPAGE_URL "https://github.com/arturbac/kdevcxx_with_ai"
    DESCRIPTION "KDevelop client for Claude Code"
    VERSION 1.0.1
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# no C++ modules; CMake 3.28+ would otherwise need clang-scan-deps for every C++20 target
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

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
# KDECompilerSettings disables exceptions, but the STL throws (bad_alloc, format_error, ...) and must be handled
kde_enable_exceptions()

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

#----------------------------------------------------------------
# core dependencies (no Qt): during development my libraries track master
#----------------------------------------------------------------
# KDECMakeSettings builds shared libraries by default, the plugin must not depend on an extra .so
set(BUILD_SHARED_LIBS OFF)
CPMAddPackage(
    NAME              small_vectors
    GITHUB_REPOSITORY arturbac/small_vectors
    GIT_TAG           master
    EXCLUDE_FROM_ALL  YES
    SYSTEM            YES
    OPTIONS           "SMALL_VECTORS_EXPORT_CMAKE_TARGETS OFF"
)
set_target_properties(small_vectors PROPERTIES POSITION_INDEPENDENT_CODE ON)
# stralgo links the namespaced name, which only small_vectors' install config defines
add_library(small_vectors::small_vectors ALIAS small_vectors)
unset(BUILD_SHARED_LIBS)
CPMAddPackage(
    NAME              stralgo
    GITHUB_REPOSITORY arturbac/stralgo
    GIT_TAG           master
    EXCLUDE_FROM_ALL  YES
    SYSTEM            YES
    OPTIONS           "STRALGO_ENABLE_UNIT_TESTS OFF" "STRALGO_EXPORT_CMAKE_TARGETS OFF"
)
CPMAddPackage(
    NAME              simple_enum
    GITHUB_REPOSITORY arturbac/simple_enum
    GIT_TAG           master
    EXCLUDE_FROM_ALL  YES
    SYSTEM            YES
    OPTIONS           "SIMPLE_ENUM_ENABLE_TESTS OFF" "SIMPLE_ENUM_EXPORT_CMAKE_TARGETS OFF"
)
CPMAddPackage(
    NAME              glaze
    GITHUB_REPOSITORY stephenberry/glaze
    GIT_TAG           v9.0.0
    EXCLUDE_FROM_ALL  YES
    SYSTEM            YES
)

set(CMAKE_AUTOMOC ON)

add_subdirectory(src)

enable_testing()
include(CTest)
include(cmake/add_engine_ut.cmake)
add_subdirectory(unittests)

include(cmake/spdx_check.cmake)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
