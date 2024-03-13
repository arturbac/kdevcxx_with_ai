set(CMAKE_AUTOMOC ON)

find_package(KF5TextEditor REQUIRED)

add_library( plugin_commn STATIC )

add_library( kdevcxx_with_ai::plugin_commn ALIAS plugin_commn )

target_sources(plugin_commn PRIVATE
 plugin_common.cc
 info_dialog.cc
 )

target_link_libraries(plugin_commn PUBLIC
    ai_processing::core
    # Qt5::Widgets
    KF5::TextEditor
    ${BASIC_LINK_FLAGS}
)
target_compile_options( plugin_commn PUBLIC "-fexceptions")

target_include_directories( plugin_commn PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR} )
