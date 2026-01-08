#----------------------------------------------------------------
# boost-ext/ut
#----------------------------------------------------------------

find_package(ut QUIET)
if(ut_FOUND)
    message(STATUS "Using system Boost.UT library.")
else()
    message(STATUS "Using CPM arturbac/ut-ext package.")
    CPMAddPackage(
        ut
        GITHUB_REPOSITORY arturbac/ut-ext
        GIT_TAG           v2.0.1_2
        EXCLUDE_FROM_ALL  YES
    )
endif()

add_engine_ut(parse_json_response_ut.cc)
add_engine_ut(clang_format_ut.cc)
add_engine_ut(clean_string_ut.cc)
add_engine_ut(network_ut.cc)
add_engine_ut(is_valid_openai_bearer_key_ut.cc)
