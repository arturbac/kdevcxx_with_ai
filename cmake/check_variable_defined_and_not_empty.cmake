macro(check_variable_defined_and_not_empty VAR_NAME)
    if(NOT DEFINED ${VAR_NAME} OR "${${VAR_NAME}}" STREQUAL "")
        message(FATAL_ERROR "${VAR_NAME} not defined or is empty")
    endif()
endmacro()

