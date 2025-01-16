// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#include "kdevcxx_with_ai_config_page.h"
#include <klocalizedstring.h>

#ifndef Q_MOC_RUN
#include <aiprocess/app_settings.h>
#include <aiprocess/spdlog.hpp>
#include <simple_enum/enum_index.hpp>
#include <aiprocess/log.h>
#endif

kdevcxx_with_ai_config_page::kdevcxx_with_ai_config_page(KDevelop::IPlugin * plugin, QWidget * parent) :
    KDevelop::ConfigPage{plugin, nullptr, parent}
  {
  kdevcxxai::config_page::construct<aiprocess::backend_type_e::kdevelop>(
    *this, ui, std::bind(&kdevcxx_with_ai_config_page::emit_changed, this)
  );
  }

kdevcxx_with_ai_config_page::~kdevcxx_with_ai_config_page() = default;

void kdevcxx_with_ai_config_page::emit_changed() { emit changed(); }

auto kdevcxx_with_ai_config_page::name() const -> QString { return i18n("OpenAI Configuration"); }

auto kdevcxx_with_ai_config_page::fullName() const -> QString { return i18n("Configure Open AI Settings"); }

auto kdevcxx_with_ai_config_page::icon() const -> QIcon { return QIcon::fromTheme("preferences-other"); }

void kdevcxx_with_ai_config_page::apply() { kdevcxxai::config_page::apply<aiprocess::backend_type_e::kdevelop>(ui); }

void kdevcxx_with_ai_config_page::reset() { kdevcxxai::config_page::reset<aiprocess::backend_type_e::kdevelop>(ui); }

void kdevcxx_with_ai_config_page::defaults() { kdevcxxai::config_page::defaults(ui); }
