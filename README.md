
# KDevCXX with AI

KDevCXX with AI is an innovative extension for the KDevelop IDE, focusing on integrating AI-powered features specifically designed to enhance C++ development. This project leverages the latest advancements in machine learning and natural language processing to provide KDevelop users with intelligent coding assistance, including code completion, documentation lookup, and advanced code analysis.

## Features

- **AI-Powered Code Completion**: Utilizes a trained model `gpt-4-1106-preview` to suggest code completions, significantly speeding up the development process.
- **Documentation Lookup**: Instant access to documentation for C++ standard library functions, classes, and more directly within the IDE.
- **Code Analysis**: Advanced code analysis features powered by AI, helping to identify potential improvements and optimizations.
- **Error Correction Suggestions**: Offers suggestions for correcting syntax errors and logical mistakes.
- **Refactoring Assistance**: AI-assisted refactoring tools to improve and simplify existing code without changing its external behavior.

## Planned Features

- **Multiple Presets** for AI query assistant working with new and existing code, explanation providing information with tools windows, refactor for refactoring old code
- **Add kdevelop settings kcm ui** for easier settings access
  
## Getting Started

### Prerequisites

- KDevelop IDE installed on your system.
- Internet connection for accessing AI features.


## Required System Packages

- **CMake**: Required to configure the build system.
- **Boost**: Version 1.66.0 or later, components `system` and `thread` required.
- **OpenSSL**: Necessary for https secure data handling.

Install these packages using your system's package manager.

## External Libraries Managed with CPM

This project uses CPM for automatic management of external libraries:

- **simple_enum** from [`arturbac/simple_enum`](https://github.com/arturbac/simple_enum)
- **small_vectors** from [`arturbac/small_vectors`](https://github.com/arturbac/small_vectors)
- **stralgo** from [`arturbac/stralgo`](https://github.com/arturbac/stralgo)
- **glaze** from [`stephenberry/glaze`](https://github.com/stephenberry/glaze)
- **fmt** from [`fmtlib/fmt`](https://github.com/fmtlib/fmt)
- **ut** from [`C++20 μ(micro)/Unit Testing Framework`](https://github.com/boost-ext/ut)
  
## KDE Dependencies

KDE frameworks are utilized, requiring:

- **Extra CMake Modules (ECM)** version `5.68.0` or newer.

Ensure KDE development libraries and the ECM package are installed.

## Installation Instructions

Follow these steps to install and configure KDevCXX with AI:

### 1. Clone the Repository

Start by cloning the repository to your local machine using the following command:

```bash
git clone https://github.com/arturbac/kdevcxx_with_ai.git
```

### 2. Build the Plugin

Use CMake to build the plugin with the following command:

```bash
cmake --workflow --preset="clang-release"
```

### 3. Install the Plugin

Use provided install.sh script it will determine location of kdevplatform plugins and install plugin into that directory


### 4. Configure OpenAI Key

Upon launching KDevelop, a modal dialog will appear, prompting you to review and update your settings located at ~/.config/kdevcxx_with_ai/kdevcxx_with_ai_ai_settings.json.

## Usage

After installing the plugin, restart KDevelop. The AI features should now be seamlessly integrated into your IDE.

To use the AI functionality within the editor, follow these steps:

1. Write your AI request in the editor, preceded by `[AI DO something for me with that code]`, followed by the code you wish AI to analyze or modify.

2. Highlight the request and the target code.

3. Press `Ctrl + 1` to activate the AI processing of selected request and code.

## Example use 

Command

Implementing unit tests example. note that it takes into effect my system rules specifying explicitly to use bost-ext/ut and You can change this in settings
```json
{
   "api_key": "******************",
   "cxx_rules": "You are great c++23 coder,... if implementing  unit tests You use boost-ext/ut ...",
   "version": 1
}

```

![Bez nazwy2](https://github.com/arturbac/kdevcxx_with_ai/assets/14975842/dfbe7bbe-ccca-4f3d-b577-52a87c2480fe)


![obraz](https://github.com/arturbac/kdevcxx_with_ai/assets/14975842/31e7bcb6-caf1-4a0e-9278-d4560c9266f6)

After execution

![obraz](https://github.com/arturbac/kdevcxx_with_ai/assets/14975842/4565cb0f-9ad2-46e9-bda8-93ff633af673)

Or asking for help with explanations

![obraz](https://github.com/arturbac/kdevcxx_with_ai/assets/14975842/0f818de6-9146-453d-beed-b66832c9e8ae)

or example refactoring of very old code

![Bez nazwy](https://github.com/arturbac/kdevcxx_with_ai/assets/14975842/f7128298-bcaf-4772-a929-9f94b23579c1)




## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.

## Acknowledgments

- Thanks to the KDevelop community for providing a powerful and extensible IDE platform.

