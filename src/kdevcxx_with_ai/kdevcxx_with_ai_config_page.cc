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
    KDevelop::ConfigPage(plugin, nullptr, parent)
  {
  kdevcxxai::config_page::construct<aiprocess::backend_type_e::kdevelop>(
    *this, ui, std::bind(&kdevcxx_with_ai_config_page::emit_changed, this)
  );
  }

void kdevcxx_with_ai_config_page::emit_changed() { Q_EMIT changed(); }

QString kdevcxx_with_ai_config_page::name() const { return i18n("OpenAI Configuration"); }

QString kdevcxx_with_ai_config_page::fullName() const { return i18n("Configure Open AI Settings"); }

QIcon kdevcxx_with_ai_config_page::icon() const { return QIcon::fromTheme(QLatin1String("preferences-other")); }

void kdevcxx_with_ai_config_page::apply() { kdevcxxai::config_page::apply<aiprocess::backend_type_e::kdevelop>(ui); }

void kdevcxx_with_ai_config_page::reset() { kdevcxxai::config_page::reset<aiprocess::backend_type_e::kdevelop>(ui); }

void kdevcxx_with_ai_config_page::defaults() { kdevcxxai::config_page::defaults(ui); }
