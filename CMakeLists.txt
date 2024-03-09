cmake_minimum_required(VERSION 3.21)


project(kdevcxx_with_ai LANGUAGES CXX
    HOMEPAGE_URL "http://github/arturbac/kdevcxx_with_ai"
    DESCRIPTION "KDevelop OpenAI API plugin"
   )

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) 

set(BUILD_SHARED_LIBS OFF)
add_compile_options("-fPIC")

include(cmake/CPM.cmake)
#----------------------------------------------------------------
# simple_enum
#----------------------------------------------------------------
CPMAddPackage(
  Name simple_enum
  GITHUB_REPOSITORY arturbac/simple_enum
  GIT_TAG v0.6.0
)
# clang exposes static operators() in c++20 compile mode even when not asked to do so
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
target_compile_options( simple_enum INTERFACE "-Wno-c++23-extensions" )
endif()
find_package(simple_enum REQUIRED)
#----------------------------------------------------------------
# small_vectors
#----------------------------------------------------------------
CPMAddPackage(
  small_vectors
  GITHUB_REPOSITORY arturbac/small_vectors
  GIT_TAG        v3.1.0
  )
find_package(small_vectors REQUIRED)
# clang exposes static operators() in c++20 compile mode even when not asked to do so
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
target_compile_options( simple_enum INTERFACE "-Wno-c++23-extensions" )
endif()
#----------------------------------------------------------------
# stralgo
#----------------------------------------------------------------
CPMAddPackage(
  stralgo
  GITHUB_REPOSITORY arturbac/stralgo
  GIT_TAG        v1.2.0
  OPTIONS "STRALGO_ENABLE_UNIT_TESTS=OFF"
  )
find_package(stralgo REQUIRED)
# clang exposes static operators() in c++20 compile mode even when not asked to do so
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
target_compile_options( simple_enum INTERFACE "-Wno-c++23-extensions" )
endif()
#----------------------------------------------------------------
# glaze
#----------------------------------------------------------------
CPMAddPackage(
  glaze
  GITHUB_REPOSITORY stephenberry/glaze
  GIT_TAG        v2.1.8
  )
find_package(glaze REQUIRED)
#----------------------------------------------------------------
# fmt
#----------------------------------------------------------------
CPMAddPackage(
  NAME fmt
  GITHUB_REPOSITORY fmtlib/fmt
  GIT_TAG 10.2.1
)

find_package(Boost 1.66.0 REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)

include(cmake/building_config.cmake)
add_subdirectory(ai_processing)
enable_testing()
include(CTest)
include(cmake/add_engine_ut.cmake)
add_subdirectory(unittests)
#----------------------------------------------------------------
# kde junk below this like don not add any targets here to this shit thise KDE* cmake things brake normal code build

find_package(ECM "5.68.0" CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(FeatureSummary)

find_package(KDevPlatform 5.2.40 REQUIRED)


add_subdirectory(src)

ecm_qt_install_logging_categories(
    EXPORT CXX_WITH_GPT
    FILE cxx_with_gpt.categories
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

#https://github.com/libcpr/cpr
