add_library(ide_protocol STATIC
    ide_protocol.cc
    ide_server.cc
)
add_library(kdevcxx_with_ai::ide_protocol ALIAS ide_protocol)

target_include_directories(ide_protocol PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ide_protocol PUBLIC Qt6::Core Qt6::WebSockets)
target_compile_definitions(ide_protocol PRIVATE KDEVCXX_WITH_AI_VERSION="${PROJECT_VERSION}")
set_target_properties(ide_protocol PROPERTIES POSITION_INDEPENDENT_CODE ON)
