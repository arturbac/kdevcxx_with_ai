// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

#include <qdockwidget.h>
#include <QWebEngineView>
#include <qfile.h>

class markdown_view : public QDockWidget
  {
  Q_OBJECT
public:
  explicit markdown_view(QWidget * parent = nullptr) : QDockWidget(parent)
    {
    web_view = new QWebEngineView(this);
    setWidget(web_view);
    }

  void load_markdown(QString const & file_path);

private:
  QWebEngineView * web_view;
  };
