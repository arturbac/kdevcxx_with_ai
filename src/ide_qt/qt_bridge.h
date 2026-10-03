// SPDX-FileCopyrightText: 2026 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

// The only std <-> Qt string conversion: UTF-8 std::string <-> UTF-16 QString with stralgo::utf, one copy

#include <stralgo/utf/utf.h>

#include <QString>
#include <QStringList>
#include <QStringView>
#include <string>
#include <string_view>
#include <vector>

namespace ide_qt
  {
[[nodiscard]]
inline auto to_std(QStringView text) -> std::string
  { return stralgo::utf::stl::to_string(std::u16string_view{text.utf16(), static_cast<std::size_t>(text.size())}); }

[[nodiscard]]
inline auto to_qt(std::string_view text) -> QString
  {
  QString result{static_cast<qsizetype>(stralgo::utf::u16capacity(text)), Qt::Uninitialized};
  auto * const first{reinterpret_cast<char16_t *>(result.data())};
  auto * const last{stralgo::utf::convert(text, first)};
  result.truncate(last - first);
  return result;
  }

[[nodiscard]]
inline auto to_std(QStringList const & list) -> std::vector<std::string>
  {
  std::vector<std::string> result;
  result.reserve(static_cast<std::size_t>(list.size()));
  for(auto const & text: list)
    result.push_back(to_std(text));
  return result;
  }
  }  // namespace ide_qt
