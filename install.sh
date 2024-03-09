#!/bin/bash

# Define source and destination paths
SOURCE="build/debug/bin/kdevcxx_with_ai.so"
DESTINATION="/usr/lib64/qt5/plugins/kdevplatform/513/kdevcxx_with_ai.so"

# Use sudo to copy the file to the destination
sudo cp "$SOURCE" "$DESTINATION"

# Check if the copy operation was successful
if [ $? -eq 0 ]; then
    echo "Copy operation successful."
else
    echo "Copy operation failed."
fi

