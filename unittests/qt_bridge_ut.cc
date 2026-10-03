// SPDX-FileCopyrightText: 2026 Artur Bać
// SPDX-License-Identifier: MIT

#include <boost/ut.hpp>
#include <ostream>
#include <event_guard.h>
#include <qt_bridge.h>

#include <stdexcept>

using namespace boost::ut;
using namespace Qt::StringLiterals;
using namespace std::string_view_literals;

// lets boost::ut print QString values of failed expectations
[[maybe_unused]]
static auto operator<<(std::ostream & os, QString const & s) -> std::ostream &
  { return os << s.toStdString(); }

int main()
  {
  "ascii_and_empty"_test = []
  {
    expect(eq(ide_qt::to_std(u"/p/a.cc"_s), "/p/a.cc"sv));
    expect(eq(ide_qt::to_qt("/p/a.cc"), u"/p/a.cc"_s));
    expect(ide_qt::to_std(QString{}).empty());
    expect(ide_qt::to_qt("").isEmpty());
  };

  "multibyte_and_surrogate_pairs_match_qt"_test = []
  {
    // 2-byte, 3-byte and 4-byte UTF-8 (the last is a UTF-16 surrogate pair)
    auto const text{u"zażółć € 😀 end"_s};
    auto const utf8{ide_qt::to_std(text)};
    expect(eq(utf8, text.toStdString()));
    expect(eq(ide_qt::to_qt(utf8), text));
    expect(eq(ide_qt::to_qt(utf8).size(), text.size()));
  };

  "string_list"_test = []
  {
    auto const list{ide_qt::to_std(QStringList{u"/a"_s, u"/ż"_s})};
    expect(list == std::vector<std::string>{"/a", "/ż"});
  };

  "event_guard_stops_exceptions"_test = []
  {
    bool ran{};
    expect(ide_qt::event_guard("ok", [&] { ran = true; }));
    expect(ran);
    expect(not ide_qt::event_guard("std", [] { throw std::runtime_error{"boom"}; }));
    expect(not ide_qt::event_guard("other", [] { throw 1; }));
  };
  }
