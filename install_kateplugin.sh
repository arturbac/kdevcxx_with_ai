#!/bin/bash

# Paths to search
search_paths=("/usr/lib64" "/usr/lib")

# Destination path suffix
path_suffix="qt5/plugins/ktexteditor"

# Compiled plugin to copy
compiled_plugin="build/clang-release/bin/ktexteditor/kate_with_ai.so"

# Function to search paths
search_and_copy() {
    for path in "${search_paths[@]}"; do
        full_path="$path/$path_suffix"
        if [[ -d "$full_path" ]]; then
            echo "Found ktexteditor directory at: $full_path"
            # Ask for permission to copy
            read -p "Do you want to copy kate_with_ai.so to $full_path? (y/n): " user_choice
            if [[ "$user_choice" == "y" || "$user_choice" == "Y" ]]; then
                sudo cp "$compiled_plugin" "$full_path/kate_with_ai.so" && echo "Copy successful." || echo "Copy failed."
            else
                echo "Copy operation aborted by the user."
            fi
            return 0
        fi
    done
    echo "ktexteditor directory not found in the specified paths."
}

# Invoke the search and copy function
search_and_copy
