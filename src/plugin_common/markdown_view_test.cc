// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#include <qapplication.h>
#include <qmainwindow.h>
#include "markdown_view.h"

int main(int argc, char * argv[])
  {
  QApplication app(argc, argv);

  QMainWindow main_window;
  main_window.resize(800, 600);

  markdown_view * md_view = new markdown_view(&main_window);
  main_window.addDockWidget(Qt::RightDockWidgetArea, md_view);

  md_view->load_markdown(R"(
# simple_enum

## License Update for Version 0.6.0

I am updating the project license from the MIT License to the Boost Software License - Version 1.0, effective starting with version 0.6.0. This change aims to provide clearer patent rights while maintaining the permissiveness of our licensing. This transition affects no external contributors and requires no action from users. I believe this update supports my commitment to open-source best practices.


## Features

- **Bounded Enum Views**: Provides `enum_view` for iterating over bounded enumerations, leveraging `std::ranges::views`.
- **Enum to String and Back**: Supports conversion from enum to `std::string_view` and vice versa with minimal compile-time overhead.
- **Enumeration Indexing**: Offers `enum_index`, allowing for index retrieval of enum values based on compile-time metadata.

```cpp

template<typename type>
concept enum_concept = std::is_enum_v<type>;
```
)");

  main_window.show();
  return app.exec();
  }
