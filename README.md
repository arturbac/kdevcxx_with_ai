
# KDevCXX with AI, Kate with AI

KDevCXX with AI is an innovative extension for the KDevelop IDE and Kate Editor, focusing on integrating AI-powered features specifically designed to enhance C++ development. This project leverages the latest advancements in machine learning and natural language processing to provide KDevelop users with intelligent coding assistance, including code completion, documentation lookup, and advanced code analysis.

## Features

- **AI-Powered Code Completion**: Utilizes a trained model `gpt-4-1106-preview` to suggest code completions, significantly speeding up the development process.
- **Documentation Lookup**: Instant access to documentation for C++ standard library functions, classes, and more directly within the IDE.
- **Code Analysis**: Advanced code analysis features powered by AI, helping to identify potential improvements and optimizations.
- **Error Correction Suggestions**: Offers suggestions for correcting syntax errors and logical mistakes.
- **Refactoring Assistance**: AI-assisted refactoring tools to improve and simplify existing code without changing its external behavior.

## Planned Features

- **Multiple Presets** for AI query assistant working with new and existing code, explanation providing information with tools windows, refactor for refactoring old code
- **Chat with GPT windows** producing desired result with finall button apply to move result to code editor window.
  
## Getting Started

### Prerequisites

- KDevelop IDE installed on your system for kdevelop plugin
- Kate Editor installed on your system for kate plugin
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
kate plugin will be build and if kdevplatform development headers and libraries are present then kdevelop plugin will be build also.

### 3. Install the Plugin

Use provided install_kdeveplugin.sh and or install_kateplugin.sh scripts it will determine location of kdevplatform plugins and qt/plugins for texteditor and install plugins into that directories


### 4. Configure OpenAI Key

Upon launching KDevelop, a modal dialog will appear, prompting you to review and update your settings in KDevelop settings dialog.
For kate You need to go Kate settings and enable plugin and edit them.
Note that ai settings are shared between kate and kdevelop, so You only need to do it once.

## Usage

After installing the plugin, restart KDevelop/Kate. The AI features should now be seamlessly integrated into your IDE.

To use the AI functionality within the editor, follow these steps:

1. Write your AI request in the editor, preceded by `[AI Do something for me with that code]`, followed by the code/text you wish AI to analyze or modify.

2. Highlight the request and the target code.

3. Select from Drop down menu `Process with AI` to activate the AI processing of selected request and code in format \[AI perform command \].


## Example use 

![Bez nazwy](https://github.com/arturbac/kdevcxx_with_ai/assets/14975842/b30c6e56-09d7-4cec-b50b-7d1d4d9b1b2a)


Implementing unit tests example. note that it takes into effect my system rules specifying explicitly to use bost-ext/ut and You can change this in settings
```json
{
   "api_key": "******************",
   "cxx_rules": "You are great c++23 coder,... if implementing  unit tests You use boost-ext/ut ...",
   "version": 1
}

```

![Bez nazwy2](https://github.com/arturbac/kdevcxx_with_ai/assets/14975842/dfbe7bbe-ccca-4f3d-b577-52a87c2480fe)

Or asking for help with explanations

![obraz](https://github.com/arturbac/kdevcxx_with_ai/assets/14975842/0f818de6-9146-453d-beed-b66832c9e8ae)

or example refactoring of very old code

![Bez nazwy](https://github.com/arturbac/kdevcxx_with_ai/assets/14975842/f7128298-bcaf-4772-a929-9f94b23579c1)

And perhaps the most weird use of AI i did with code, optimise ai query with ai for ai

![obraz](https://github.com/arturbac/kdevcxx_with_ai/assets/14975842/a4ec262c-e805-4252-ab45-01b59be4ce38)

Or some crazy s...

![obraz](https://github.com/arturbac/kdevcxx_with_ai/assets/14975842/952db481-e76a-4935-a812-5cb10c951421)


## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.

## Acknowledgments

- Thanks to the KDevelop community for providing a powerful and extensible IDE platform.

