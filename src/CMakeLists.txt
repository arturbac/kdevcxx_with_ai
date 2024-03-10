
set(CXX_WITH_GPT_UI cxx_with_gpt_settings.ui)
qt5_wrap_ui(CXX_WITH_GPT_UI_HEADERS ${CXX_WITH_GPT_UI})

set(cxx_with_gpt_SRCS
    cxx_with_gpt.cpp
    rcfile.ui.rc
    ${CXX_WITH_GPT_UI_HEADERS}
)

ecm_qt_declare_logging_category(cxx_with_gpt_SRCS
    HEADER debug.h
    IDENTIFIER PLUGIN_CXX_WITH_GPT
    CATEGORY_NAME "kdevelop.plugins.cxx_with_gpt"
    DESCRIPTION "KDevelop plugin: cxx_with_gpt"
    EXPORT CXX_WITH_GPT
)

kdevplatform_add_plugin(cxx_with_gpt
    SOURCES ${cxx_with_gpt_SRCS}
)

# kconfig_add_kcfg_files(cxx_with_gpt
#    GENERATE_MOC cxx_with_gpt.kcfgc
#    
# )

target_link_libraries(cxx_with_gpt
    KDev::Interfaces
    ai_processing::core
    ${BASIC_LINK_FLAGS}
)

target_compile_options( cxx_with_gpt PUBLIC -fexceptions)

set_target_properties(cxx_with_gpt PROPERTIES OUTPUT_NAME "kdevcxx_with_ai")

# Specify the resource file
set(RESOURCES
    resources.qrc
)

# Use Qt5's function to process the resource file
qt5_add_resources(RESOURCES_RCC ${RESOURCES})
