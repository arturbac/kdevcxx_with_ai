
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

kcoreaddons_add_plugin(kate_with_ai # your plugin name here
    INSTALL_NAMESPACE "ktexteditor")

# add_library(kate_with_ai MODULE)
target_sources(kate_with_ai PRIVATE
   kate_with_ai.cc
   kate_with_ai_config_page.cc
   )

target_compile_definitions(kate_with_ai PRIVATE TRANSLATION_DOMAIN="kate_with_ai")

target_link_libraries(kate_with_ai
  kdevcxx_with_ai::plugin_commn
  ${BASIC_LINK_FLAGS}
  KF5::CoreAddons
  KF5::I18n
  KF5::TextEditor
  )

install(TARGETS kate_with_ai
    LIBRARY DESTINATION lib64/qt5/plugins/ktexteditor
)
