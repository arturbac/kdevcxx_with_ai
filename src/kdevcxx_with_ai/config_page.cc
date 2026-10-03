// SPDX-FileCopyrightText: 2026 Artur Bać
// SPDX-License-Identifier: MIT

#include "config_page.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>

using namespace Qt::StringLiterals;

namespace
  {
[[nodiscard]]
auto group() -> KConfigGroup
  { return KSharedConfig::openConfig()->group(QString::fromLatin1(config_group)); }
  }  // namespace

auto read_claude_command() -> QString
  { return group().readEntry(config_claude_command, QString::fromLatin1(default_claude_command)); }

config_page_t::config_page_t(KDevelop::IPlugin * plugin, QWidget * parent) :
    KDevelop::ConfigPage{plugin, nullptr, parent},
    claude_command_{new QLineEdit{this}}
  {
  auto * layout{new QFormLayout{this}};
  layout->addRow(i18n("Claude command:"), claude_command_);
  auto * note{new QLabel{i18n("Used when the Claude Code tool view starts, e.g. \"claude --model opus\".")}};
  note->setWordWrap(true);
  layout->addRow(note);
  connect(claude_command_, &QLineEdit::textChanged, this, &config_page_t::changed);
  reset();
  }

auto config_page_t::name() const -> QString { return i18n("Claude Code"); }

auto config_page_t::icon() const -> QIcon { return QIcon::fromTheme(u"utilities-terminal"_s); }

auto config_page_t::apply() -> void
  {
  group().writeEntry(config_claude_command, claude_command_->text());
  group().sync();
  }

auto config_page_t::reset() -> void { claude_command_->setText(read_claude_command()); }

auto config_page_t::defaults() -> void { claude_command_->setText(QString::fromLatin1(default_claude_command)); }
