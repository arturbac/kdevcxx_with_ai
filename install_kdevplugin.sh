#!/bin/bash

# Source file
SOURCE="build/clang-release/bin/kdevcxx_with_ai.so"

# Search for kdevclangsupport.so and update DESTINATION
FOUND=$(find /usr -type f -name kdevclangsupport.so 2>/dev/null | head -n 1)
if [[ -z "$FOUND" ]]; then
    echo "kdevelop plugin directory not found."
    exit 1
else
    DESTINATION="${FOUND/kdevclangsupport.so/kdevcxx_with_ai.so}"
fi

echo "The file $SOURCE will be copied to $DESTINATION using sudo mode, requiring elevated permissions."
echo "Do you want to continue? [Y/N]"
read -r CONFIRM
if [[ "$CONFIRM" != "Y" && "$CONFIRM" != "y" ]]; then
    echo "Operation cancelled."
    exit 0
fi

# Copy the file to the destination using sudo
sudo cp "$SOURCE" "$DESTINATION"

# Verify the copy operation
if [ $? -eq 0 ]; then
    echo "Plugin installation successful."
else
    echo "Plugin installation failed."
fi

