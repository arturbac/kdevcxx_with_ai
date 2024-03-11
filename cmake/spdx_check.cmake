# List of directories to check
set(DIRECTORIES_TO_CHECK "src" "ai_processing" "unittests" )

# Generate the find command argument list from the directories
set(FIND_CMD_DIRS "")
foreach(DIR IN LISTS DIRECTORIES_TO_CHECK)
    list(APPEND FIND_CMD_DIRS ${CMAKE_SOURCE_DIR}/${DIR})
endforeach()

# SPDX check command using shell commands (Unix/Linux)
add_custom_target(spdx_check
    COMMAND ${CMAKE_COMMAND} -E echo "Checking for SPDX-License-Identifier tags in .h, .hpp, .c, .cc, .cpp files within specified directories..."
    COMMAND find ${FIND_CMD_DIRS} -type f "\\(" -name "*.h" -o -name "*.hpp" -o -name "*.c" -o -name "*.cc" -o -name "*.cpp" "\\)" -exec grep -L "SPDX-License-Identifier" {} +
    COMMAND ${CMAKE_COMMAND} -E echo "Check complete. Files listed above are missing SPDX-License-Identifier tags."
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "SPDX tag check..."
)

