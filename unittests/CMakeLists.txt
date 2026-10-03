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
        GIT_TAG           v2.0.1_9
        EXCLUDE_FROM_ALL  YES
    )
endif()

add_engine_ut(ide_protocol_ut.cc)
target_link_libraries(ide_protocol_ut PRIVATE glaze::glaze)
add_engine_ut(qt_bridge_ut.cc)
target_link_libraries(qt_bridge_ut PRIVATE kdevcxx_with_ai::ide_qt)
add_engine_ut(ide_server_ut.cc)
target_link_libraries(ide_server_ut PRIVATE kdevcxx_with_ai::ide_qt Qt6::Test)
