string (ASCII 27 _escape)
set( clr_clear "${_escape}[0m")
set( clr_blue "${_escape}[1;34m")
set( clr_green "${_escape}[1;32m")
set( clr_red "${_escape}[1;31m")

function(check_term_color)
    # Read the value of the TERM environment variable into a CMake variable
    set(TERM_ENV $ENV{TERM})

    # Output the value of the TERM environment variable
    # message(STATUS "TERM environment variable: ${TERM_ENV}")

    # Check if the TERM_ENV variable contains the word "color"
    string(FIND "${TERM_ENV}" "color" TERM_COLOR_INDEX)

    # string (ASCII 27 _escape)
    # set(GREEN "${_escape}[1;32m")
    # set(RESET "${_escape}[0m")
    if (TERM_COLOR_INDEX EQUAL -1)
        # message(STATUS "TERM is monochromatic.")
        set( TERM_COLOR 0 PARENT_SCOPE)
        set(clr_blue "" PARENT_SCOPE)
        set(clr_clear "" PARENT_SCOPE)
    else()
        # message(STATUS "TERM ${GREEN}supports${RESET} color.")
        set( TERM_COLOR 1 PARENT_SCOPE)
    endif()
endfunction()
