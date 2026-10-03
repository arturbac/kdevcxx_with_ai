# Qt adapter of the core: WebSocket transport and std <-> Qt string conversion
add_library(ide_qt STATIC
    ide_server.cc
)
add_library(kdevcxx_with_ai::ide_qt ALIAS ide_qt)

target_include_directories(ide_qt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ide_qt PUBLIC kdevcxx_with_ai::ide_protocol stralgo Qt6::Core Qt6::WebSockets)
set_target_properties(ide_qt PROPERTIES POSITION_INDEPENDENT_CODE ON)
