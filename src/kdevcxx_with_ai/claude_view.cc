// SPDX-FileCopyrightText: 2026 Artur Bać
// SPDX-License-Identifier: MIT

#include "claude_view.h"

#include <KLocalizedString>
#include <KParts/ReadOnlyPart>
#include <KPluginFactory>
#include <kde_terminal_interface.h>

#include <QLabel>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

claude_view_t::claude_view_t(QString launch_command, QString working_dir, QWidget * parent) :
    QWidget{parent},
    launch_command_{std::move(launch_command)},
    working_dir_{std::move(working_dir)}
  {
  setWindowTitle(i18n("Claude Code"));
  setWindowIcon(QIcon::fromTheme(u"utilities-terminal"_s));
  new QVBoxLayout{this};
  layout()->setContentsMargins({});
  load_part();
  }

auto claude_view_t::load_part() -> void
  {
  auto result{
    KPluginFactory::instantiatePlugin<KParts::ReadOnlyPart>(KPluginMetaData{u"kf6/parts/konsolepart"_s}, this)
  };
  auto * terminal{result ? qobject_cast<TerminalInterface *>(result.plugin) : nullptr};
  if(not terminal)
    {
    layout()->addWidget(new QLabel{i18n("Cannot load the Konsole part (konsolepart), is Konsole installed?"), this});
    return;
    }

  part_ = result.plugin;
  layout()->addWidget(part_->widget());
  setFocusProxy(part_->widget());
  // a shell keeps the terminal alive after claude exits, so it can be restarted from history
  terminal->showShellInDir(working_dir_);
  terminal->sendInput(launch_command_ + u'\n');
  // the part destroys itself when the shell exits
  connect(part_, &QObject::destroyed, this, &claude_view_t::load_part, Qt::QueuedConnection);
  }
