kdevplatform_add_plugin(kdevcxx_with_ai_p
    SOURCES
        kdevcxx_with_ai.json
        kdevcxx_with_ai.cc
        claude_view.cc
        config_page.cc
        diff_dialog.cc
)

target_link_libraries(kdevcxx_with_ai_p PRIVATE
    kdevcxx_with_ai::ide_protocol
    KDev::Interfaces
    KDev::Language
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::I18n
    KF6::Parts
    KF6::TextEditor
    KF6::XmlGui
)
set_target_properties(kdevcxx_with_ai_p PROPERTIES OUTPUT_NAME "kdevcxx_with_ai")
target_compile_definitions(kdevcxx_with_ai_p PRIVATE TRANSLATION_DOMAIN="kdevcxx_with_ai")
