function(add_ut_test_target source_file_name)
    get_filename_component(test_executable_name ${source_file_name} NAME_WE)
    # message("${test_executable_name}")
    add_executable(${test_executable_name} ${source_file_name})
    target_link_libraries(${test_executable_name} PRIVATE
                          ai_processing::core
                          Boost::ut
                          ${BASIC_LINK_FLAGS}
                         )
    target_compile_options(${test_executable_name} PUBLIC -Wno-global-constructors )
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
    target_compile_options( ${test_executable_name} PRIVATE 
      "-Wno-undefined-func-template"
      "-Wno-exit-time-destructors" 
      )
    endif()
endfunction()

function(add_engine_ut source_file_name)
  add_ut_test_target(${source_file_name})
  get_filename_component(test_executable_name ${source_file_name} NAME_WE)
  add_test(NAME "${test_executable_name}"
           COMMAND ${test_executable_name})
  # add_dependencies(unit_tests ${test_executable_name})
endfunction()
