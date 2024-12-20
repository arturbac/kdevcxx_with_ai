cmake_minimum_required(VERSION 3.21)

include(FeatureSummary)
include(cmake/CPM.cmake)

project(kdevcxx_with_ai LANGUAGES CXX
    HOMEPAGE_URL "http://github/arturbac/kdevcxx_with_ai"
    DESCRIPTION "KDevelop and Kate OpenAI API plugin"
    VERSION 0.1.4
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BUILD_SHARED_LIBS OFF)

# Prevent in-source builds
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR
        "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
    )
endif()

add_compile_options("-fPIC")

add_subdirectory(src)

#----------------------------------------------------------------
# Define the Target
#----------------------------------------------------------------
# Define the library/executable target first
#add_custom_target(${PROJECT_NAME})

#----------------------------------------------------------------
# Dependency Management
#----------------------------------------------------------------
# Helper function for optional CPM-based fallback
function(find_or_add_package name github_repo tag options)
    find_package(${name} QUIET)
    if(NOT ${name}_FOUND)
        CPMAddPackage(
            NAME ${name}
            GITHUB_REPOSITORY ${github_repo}
            GIT_TAG ${tag}
            OPTIONS ${options}
        )
    endif()
endfunction()

# simple_enum
find_or_add_package(simple_enum "arturbac/simple_enum" v0.6.2 "")
if(TARGET simple_enum)
    target_compile_options(simple_enum INTERFACE "-Wno-c++23-extensions")
endif()

# small_vectors
find_or_add_package(small_vectors "arturbac/small_vectors" v3.1.1 "")
if(TARGET small_vectors)
    target_compile_options(small_vectors INTERFACE "-Wno-c++23-extensions")
endif()

# stralgo
find_or_add_package(stralgo "arturbac/stralgo" v1.2.1 "STRALGO_ENABLE_UNIT_TESTS=OFF")
if(TARGET stralgo)
    target_compile_options(stralgo INTERFACE "-Wno-c++23-extensions")
endif()

# glaze
find_or_add_package(glaze "stephenberry/glaze" v2.2.1 "")

# fmt
find_or_add_package(fmt "fmtlib/fmt" 10.2.1 "")

# spdlog
find_or_add_package(spdlog "gabime/spdlog" v1.13.0 "")

# Boost
find_package(Boost 1.66.0 REQUIRED COMPONENTS system thread)

# OpenSSL
find_package(OpenSSL REQUIRED)

#----------------------------------------------------------------
# Project Structure
#----------------------------------------------------------------
include(cmake/building_config.cmake)
add_subdirectory(ai_processing)
enable_testing()
include(CTest)
include(cmake/add_engine_ut.cmake)
add_subdirectory(unittests)

include(cmake/spdx_check.cmake)

#----------------------------------------------------------------
# Logging Categories (Optional, Uncomment if needed)
#----------------------------------------------------------------
# ecm_qt_install_logging_categories(
#     EXPORT CXX_WITH_GPT
#     FILE cxx_with_gpt.categories
#     DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
# )

#----------------------------------------------------------------
# Feature Summary
#----------------------------------------------------------------
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
