// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#include "markdown_view.h"
#include <qtextstream.h>

void markdown_view::load_markdown(QString const & html_content)
  {
  // QFile file(file_path);
  // if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
  // return;

  // QTextStream in(&file);
  // QString markdown_content = in.readAll();

  // QString html_content = convert_markdown_to_html(markdown_content);  // Assume this function exists

  web_view->setHtml(html_content);
  }

// QString markdown_view::convert_markdown_to_html(QString const & markdown)
//   {
//   // Placeholder for Markdown to HTML conversion
//   QString html_content = "<html><body>" + markdown.toHtmlEscaped() + "</body></html>";
//   return html_content;
//   }

markdown_view::~markdown_view() = default;
