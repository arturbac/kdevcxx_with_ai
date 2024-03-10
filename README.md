
# KDevCXX with AI

KDevCXX with AI is an innovative extension for the KDevelop IDE, focusing on integrating AI-powered features specifically designed to enhance C++ development. This project leverages the latest advancements in machine learning and natural language processing to provide KDevelop users with intelligent coding assistance, including code completion, documentation lookup, and advanced code analysis.

## Features

- **AI-Powered Code Completion**: Utilizes a trained model to suggest code completions, significantly speeding up the development process.
- **Documentation Lookup**: Instant access to documentation for C++ standard library functions, classes, and more directly within the IDE.
- **Code Analysis**: Advanced code analysis features powered by AI, helping to identify potential improvements and optimizations.
- **Error Correction Suggestions**: Offers suggestions for correcting syntax errors and logical mistakes.
- **Refactoring Assistance**: AI-assisted refactoring tools to improve and simplify existing code without changing its external behavior.

## Getting Started

### Prerequisites

- KDevelop IDE installed on your system.
- Internet connection for accessing AI features.


## Installation Instructions

Follow these steps to install and configure KDevCXX with AI:

### 1. Clone the Repository

Start by cloning the repository to your local machine using the following command:

```bash
git clone https://github.com/arturbac/kdevcxx_with_ai.git
```

### 2. Prepare Configuration Files

Navigate to the cloned directory and copy the example authentication data header file:

```bash
cd kdevcxx_with_ai
cp ai_processing/include/temp_auth_data.h.example ai_processing/include/temp_auth_data.h
```

### 3. Configure OpenAI Key

Currently, the OpenAI key is temporarily embedded in the binary. This will change in future versions. For now, open `ai_processing/include/temp_auth_data.h` and replace the placeholder with your actual OpenAI key.

### 4. Build the Plugin

Use CMake to build the plugin with the following command:

```bash
cmake --workflow --preset="clang-release"
```

### 5. Install the Plugin

Determine the appropriate installation directory for your system. The following commands copy the built plugin to a common location for KDevelop plugins. Adjust the `DESTINATION` path as necessary for your system:

```bash
SOURCE="build/clang-release/bin/kdevcxx_with_ai.so"
DESTINATION="/usr/lib64/qt5/plugins/kdevplatform/513/kdevcxx_with_ai.so"

# Copy the file to the destination using sudo
sudo cp "$SOURCE" "$DESTINATION"

# Verify the copy operation
if [ $? -eq 0 ]; then
    echo "Plugin installation successful."
else
    echo "Plugin installation failed."
fi
```

## Usage

After installing the plugin, restart KDevelop. The AI features should now be seamlessly integrated into your IDE.

To use the AI functionality within the editor, follow these steps:

1. Write your AI request in the editor, preceded by `[AI DO something for me with that code]`, followed by the code you wish AI to analyze or modify.

2. Highlight the request and the target code.

3. Press `Ctrl + M` to activate the AI processing.

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.

## Acknowledgments

- Thanks to the KDevelop community for providing a powerful and extensible IDE platform.

