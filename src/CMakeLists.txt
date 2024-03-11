
set(CXX_WITH_GPT_UI plugin_settings.ui)
qt5_wrap_ui(CXX_WITH_GPT_UI_HEADERS ${CXX_WITH_GPT_UI})

set(kdevcxx_with_ai_SRCS
    kdevcxx_with_ai.cc
    info_dialog.cc
    plugin_settings.cc
    markdown_view.cc
    ${CXX_WITH_GPT_UI_HEADERS}
)

kdevplatform_add_plugin(kdevcxx_with_ai
    SOURCES ${kdevcxx_with_ai_SRCS}
)

target_link_libraries(kdevcxx_with_ai
    KDev::Interfaces
    ai_processing::core
    Qt5::Widgets
    Qt5::WebEngineWidgets
    ${BASIC_LINK_FLAGS}
)
target_compile_options( kdevcxx_with_ai PRIVATE "-fexceptions")
set_target_properties(kdevcxx_with_ai PROPERTIES OUTPUT_NAME "kdevcxx_with_ai")

install(TARGETS kdevcxx_with_ai DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kdevcxx_with_ai.json DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
message(STATUS "PLUGIN_INSTALL_DIR ${PLUGIN_INSTALL_DIR}")
message(STATUS "KDE_INSTALL_KSERVICES5DIR ${KDE_INSTALL_KSERVICES5DIR}")
