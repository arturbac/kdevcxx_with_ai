// SPDX-FileCopyrightText: 2026 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

#include <interfaces/configpage.h>

class QLineEdit;

inline constexpr auto config_group{"kdevcxx_with_ai"};
inline constexpr auto config_claude_command{"claude_command"};
inline constexpr auto default_claude_command{"claude"};

[[nodiscard]]
auto read_claude_command() -> QString;

class config_page_t : public KDevelop::ConfigPage
  {
  Q_OBJECT

public:
  config_page_t(KDevelop::IPlugin * plugin, QWidget * parent);

  [[nodiscard]]
  auto name() const -> QString override;
  [[nodiscard]]
  auto icon() const -> QIcon override;

  auto apply() -> void override;
  auto reset() -> void override;
  auto defaults() -> void override;

private:
  QLineEdit * claude_command_;
  };
