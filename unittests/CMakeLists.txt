#----------------------------------------------------------------
# boost-ext/ut
#----------------------------------------------------------------
CPMAddPackage(
  ut
  GITHUB_REPOSITORY boost-ext/ut
  GIT_TAG        v2.0.1
)
find_package(ut REQUIRED)


add_engine_ut(parse_json_response_ut.cc)
add_engine_ut(clang_format_ut.cc)
add_engine_ut(clean_string_ut.cc)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
target_compile_options( parse_json_response_ut PRIVATE 
  "-Wno-undefined-func-template"
  "-Wno-exit-time-destructors" 
  )
endif()
