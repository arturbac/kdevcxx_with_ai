include(${CMAKE_CURRENT_LIST_DIR}/check_term_color.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/compile_flag_test.cmake)
check_term_color()

message("testing common flags for all projects")
if(CXX_ENABLE_SANITIZE)

  set(CMAKE_REQUIRED_LIBRARIES "-fsanitize=address" )
  check_cxx_compiler_flag("-fsanitize=address" CXX_SANITIZE_ADDRESS)
  if(CXX_SANITIZE_ADDRESS)
    set(LINKING_DEPS_REQUIRED -fsanitize=address -fsanitize=undefined)
  endif()
  check_cxx_flag("-fsanitize-address-use-after-scope")
  check_cxx_flag("-fsanitize=undefined")
  set(CMAKE_REQUIRED_LIBRARIES "" )
  set(BASIC_LINK_FLAGS ${CUSTOM_CXX_FLAGS})
  
endif()
if(${TERM_COLOR})
check_cxx_flag_test("-fdiagnostics-color=always" CXX_DIAGNOSTICS_COLOR_ALWAYS )
endif()

option(BUILD_WITH_SYMBOLS_VISIBLE "functions symbols to be visible" ON )
add_feature_info("Unit tests" BUILD_WITH_SYMBOLS_VISIBLE "functions symbols to be visible")

if(BUILD_WITH_SYMBOLS_VISIBLE)
  check_cxx_flag("-fvisibility=default")
else(BUILD_WITH_SYMBOLS_VISIBLE)
  check_cxx_flag("-fvisibility=hidden")
  check_cxx_flag("-fvisibility-inlines-hidden")
endif(BUILD_WITH_SYMBOLS_VISIBLE)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
  # check_cxx_flag("-fuse-ld=lld")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=lld")
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fuse-ld=lld")
  if(NOT (CMAKE_BUILD_TYPE STREQUAL "Debug") )
    # check_cxx_flag("-flto=thin")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto=thin")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -flto=thin")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -flto=thin")
  endif()
endif()

set( BUILD_CXX_FLAGS ${CUSTOM_CXX_FLAGS})
unset(CUSTOM_CXX_FLAGS)

set(flags_to_check
  "-fPIC"
  # Assume that the destructor of a thrown exception object will not throw. The generated code for catch handlers will be smaller.
  # A throw expression of a type with a potentially-throwing destructor will lead to an error.
  "-fassume-nothrow-exception-dtor"
  "-Weverything"
  "-Wsuggest-override"
  "-Wundefined-bool-conversion"
  "-Wanon-enum-enum-conversion"
  "-Wambiguous-macro"

  "-Wno-switch-default"
  "-Wno-c++1z-extensions"
  "-Wno-c++98-compat"
  "-Wno-c++98-compat-pedantic"
  "-Wno-c++20-compat"
  "-Wno-gnu-anonymous-struct"
  "-Wno-nested-anon-types"
  "-Wno-ctad-maybe-unsupported"
  "-Wno-c++23-extensions"
  "-Wno-documentation"
  "-Wno-documentation-unknown-command"
  # "-Wno-undefined-func-template"
  "-Wno-padded"
  "-Wno-shorten-64-to-32"
  # "-Wno-date-time"
  "-Wno-float-equal"
  "-Wno-unused-command-line-argument"
  "-Wno-unsafe-buffer-usage"

  "-Werror=address-of-temporary"
  "-Werror=narrowing"
  "-Werror=return-type"
  "-Werror=implicit-fallthrough"
  "-Werror=strict-aliasing"
  "-Werror=shift-count-overflow"
  "-Werror=sequence-point"

  "-Werror=uninitialized"
  "-Werror=sign-compare"
  "-Werror=suggest-destructor-override"
  "-Werror=vla"
  "-Werror=pointer-to-int-cast"
  "-Werror=pointer-to-enum-cast"
  "-Werror=void-pointer-to-enum-cast"
  "-Werror=void-pointer-to-int-cast"
  "-Werror=uninitialized-const-reference"
  # 
  "-Werror=sizeof-array-div"
  "-Werror=writable-strings"
  "-Werror=bool-conversion"
  "-Wpointer-bool-conversion"

  "-Werror=lifetime"
  "-Werror=dangling"
  "-Werror=builtin-macro-redefined"
  # 
  "-Werror=address"
  "-Werror=tautological-pointer-compare"
  "-Werror=string-compare"
  "-Werror=address-of-packed-member"
  "-Werror=align-mismatch"
  "-Werror=ambiguous-delete"
  "-Werror=ambiguous-ellipsis"
  "-Werror=ambiguous-member-template"
  "-Werror=ambiguous-reversed-operator"
  "-Werror=argument-undefined-behaviour"

  "-Werror=array-bounds"
  "-Werror=array-bounds-pointer-arithmetic"
  "-Werror=assign-enum"
  "-Werror=atomic-memory-ordering"
  "-Werror=ignored-attributes"
  "-Werror=unknown-attributes"
  "-Werror=comma"
  "-Werror=conditional-uninitialized"
  "-Werror=null-pointer-subtraction"
  "-Werror=null-pointer-arithmetic"
  "-Werror=over-aligned"
  "-Werror=redundant-move"
  "-Werror=redundant-parens"
  "-Werror=return-stack-address"
  "-Werror=self-assign"
  "-Werror=self-move"
  "-Werror=undefined-bool-conversion"
  "-Werror=undefined-reinterpret-cast"
  "-Werror=unevaluated-expression"
  "-Werror=uninitialized"
  "-Werror=unused-result"
)

set(check_cxx_flagssupported_cxx_flags "")
check_cxx_flags("${flags_to_check}")
set(BASIC_CXX_FLAGS ${BASIC_CXX_FLAGS} ${check_cxx_flagssupported_cxx_flags})
unset(check_cxx_flagssupported_cxx_flags)



set(flags_to_check
    "-Wno-reserved-macro-identifier"
    "-Wno-undef"
    "-Wno-reserved-identifier"
    "-Wno-extra-semi-stmt"
    "-Wno-extra-semi"
    "-Wno-old-style-cast"
    "-Wno-zero-as-null-pointer-constant"
    "-Wno-undefined-func-template"
    "-Wno-sign-conversion"
    "-Wno-implicit-int-conversion"
    "-Wno-implicit-int-float-conversion"
    "-Wno-shadow-uncaptured-local"
    "-Wno-shadow"
    "-Wno-reorder-ctor"
    "-Wno-float-conversion"
    "-Wno-double-promotion"
    "-Wno-implicit-float-conversion"
    "-Wno-missing-braces"
    "-Wno-switch-enum"
    "-Wno-invalid-utf8"
    "-Wno-newline-eof"
    "-Wno-reserved-id-macro"
)
message(STATUS "testing flags off for kde")
set(check_cxx_flagssupported_cxx_flags "")
check_cxx_flags("${flags_to_check}")
set(AUTOMAPA_OFF_CXX_FLAGS ${check_cxx_flagssupported_cxx_flags})
unset(check_cxx_flagssupported_cxx_flags)

set(flags_to_check
  "-Werror=overloaded-virtual"
  "-Werror=implicit-int-conversion"
  "-Werror=reorder"
  "-Werror=zero-as-null-pointer-constant"
  "-Werror=old-style-cast"
  "-Werror=sign-conversion"
  "-Werror=shadow"
  "-Werror=odr"
  "-Werror=unreachable-code-aggressive"
  "-Werror=unused-comparison"
  "-Werror=unused-function"
  "-Werror=unused-local-typedef"
  "-Werror=exceptions"
  "-Werror=missing-prototypes"
  "-Werror=reserved-id-macro"
  "-Wreserved-macro-identifier"
  "-Werror=undef"
  "-Werror=reserved-identifier"
  "-Werror=extra-semi-stmt"
  "-Werror=extra-semi"
  "-Werror=old-style-cast"
  "-Werror=zero-as-null-pointer-constant"
  "-Werror=undefined-func-template"
  "-Werror=sign-conversion"
  "-Werror=implicit-int-conversion"
  "-Werror=implicit-int-float-conversion"
  "-Werror=shadow-uncaptured-local"
  "-Werror=reorder-ctor"
  "-Werror=float-conversion"
  "-Wdouble-promotion"
  "-Werror=implicit-float-conversion"
  "-Werror=missing-braces"
  "-Werror=switch-enum"
  "-Werror=weak-vtables"
  "-Werror=unused-function"
  "-Werror=unused-result"
  )
message(STATUS "testing additional strict flags for this project")
set(check_cxx_flagssupported_cxx_flags "")
check_cxx_flags("${flags_to_check}")
set(STRICT_CXX_FLAGS ${check_cxx_flagssupported_cxx_flags})
unset(check_cxx_flagssupported_cxx_flags)


set( BUILD_SYSTEM ${CMAKE_SYSTEM})
if( __LINUX__ AND NOT CMAKE_CROSSCOMPILING  )
  execute_process( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE TARGET_ARCHITECTURE )
else()
  set(TARGET_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
endif()
message(STATUS "target architecture ${TARGET_ARCHITECTURE}")

# Define an array of flags




