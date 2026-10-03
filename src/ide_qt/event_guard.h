// SPDX-FileCopyrightText: 2026 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

// Last safety net for code called from the Qt event loop (slots, lambdas, timers): an exception must not pass
// through Qt. Expected errors travel as expected_ec; this only catches what escaped, and logs it.

#include <QtGlobal>
#include <exception>
#include <utility>

namespace ide_qt
  {
/// runs fn, logs and swallows anything it throws; returns false when it threw
template<typename function_type>
auto event_guard(char const * where, function_type && fn) noexcept -> bool
  {
  try
    {
    std::forward<function_type>(fn)();
    return true;
    }
  catch(std::exception const & e)
    {
    qWarning("kdevcxx_with_ai: %s: %s", where, e.what());
    }
  catch(...)
    {
    qWarning("kdevcxx_with_ai: %s: unknown exception", where);
    }
  return false;
  }
  }  // namespace ide_qt
