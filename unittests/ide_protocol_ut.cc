// SPDX-FileCopyrightText: 2026 Artur Bać
// SPDX-License-Identifier: MIT

#include <boost/ut.hpp>
#include <ostream>
#include <ide_protocol.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <vector>

using namespace boost::ut;
using namespace Qt::StringLiterals;

// lets boost::ut print QString values of failed expectations
[[maybe_unused]]
static auto operator<<(std::ostream & os, QString const & s) -> std::ostream &
  { return os << s.toStdString(); }

namespace
  {
[[nodiscard]]
auto parse(QByteArray const & json) -> QJsonObject
  { return QJsonDocument::fromJson(json).object(); }

/// runs handle_message and collects the parsed responses
struct session_t
  {
  std::vector<ide_protocol::tool_t> tools;
  std::vector<QJsonObject> sent;
  QString notification;

  auto operator()(QJsonObject const & request) -> void
    {
    notification = ide_protocol::handle_message(
      QJsonDocument{request}.toJson(), tools, [this](QByteArray const & m) { sent.push_back(parse(m)); }
    );
    }
  };

[[nodiscard]]
auto request(int id, QString const & method, QJsonObject const & params = {}) -> QJsonObject
  { return {{u"jsonrpc"_s, u"2.0"_s}, {u"id"_s, id}, {u"method"_s, method}, {u"params"_s, params}}; }

[[nodiscard]]
auto first_text(QJsonObject const & result) -> QString
  { return result[u"content"_s].toArray().at(0).toObject()[u"text"_s].toString(); }
  }  // namespace

int main()
  {
  // QJsonArray values use = since auto x{array} picks the initializer_list ctor and wraps the array

  "auth_token_is_32_lowercase_hex"_test = []
  {
    auto const token{ide_protocol::make_auth_token()};
    expect(eq(token.size(), 32));
    expect(std::ranges::all_of(token, [](QChar c) { return (c >= u'0' and c <= u'9') or (c >= u'a' and c <= u'f'); }));
    expect(token != ide_protocol::make_auth_token());
  };

  "lock_dir_prefers_claude_config_dir"_test = []
  {
    expect(eq(ide_protocol::lock_dir(u"/home/u/.claude-x"_s, u"/home/u"_s), u"/home/u/.claude-x/ide"_s));
    expect(eq(ide_protocol::lock_dir(QString{}, u"/home/u"_s), u"/home/u/.claude/ide"_s));
  };

  "lock_file_fields"_test = []
  {
    auto const lock{parse(ide_protocol::lock_file_json(42, {u"/src/a"_s}, u"tok"_s))};
    expect(eq(lock[u"pid"_s].toInt(), 42));
    expect(eq(lock[u"workspaceFolders"_s].toArray().at(0).toString(), u"/src/a"_s));
    expect(eq(lock[u"ideName"_s].toString(), u"KDevelop"_s));
    expect(eq(lock[u"transport"_s].toString(), u"ws"_s));
    expect(eq(lock[u"authToken"_s].toString(), u"tok"_s));
  };

  "launch_command_sets_port"_test = []
  {
    expect(eq(
      ide_protocol::claude_launch_command(1234, u"claude --model opus"_s),
      u"env CLAUDE_CODE_SSE_PORT=1234 ENABLE_IDE_INTEGRATION=true claude --model opus"_s
    ));
  };

  "uri_round_trip"_test = []
  {
    expect(eq(ide_protocol::file_uri(u"/a b/c.cc"_s), u"file:///a b/c.cc"_s));
    expect(eq(ide_protocol::uri_to_path(u"file:///a%20b/c.cc"_s), u"/a b/c.cc"_s));
    expect(eq(ide_protocol::uri_to_path(u"/plain/path.cc"_s), u"/plain/path.cc"_s));
  };

  "initialize_echoes_protocol_version"_test = []
  {
    session_t s;
    s(request(1, u"initialize"_s, {{u"protocolVersion"_s, u"2025-03-26"_s}}));
    expect(eq(s.sent.size(), 1uz) >> fatal);
    auto const result{s.sent[0][u"result"_s].toObject()};
    expect(eq(s.sent[0][u"id"_s].toInt(), 1));
    expect(eq(result[u"protocolVersion"_s].toString(), u"2025-03-26"_s));
    expect(result[u"capabilities"_s].toObject().contains(u"tools"_s));
  };

  "tools_list_and_call"_test = []
  {
    session_t s;
    QJsonObject received;
    s.tools.push_back(
      {u"echo"_s,
       u"echo arguments"_s,
       {{u"type"_s, u"object"_s}},
       [&received](QJsonObject const & args, ide_protocol::reply_t reply)
       {
         received = args;
         reply(ide_protocol::text_result(args[u"x"_s].toString()));
       }}
    );
    s(request(2, u"tools/list"_s));
    auto const tools = s.sent.at(0)[u"result"_s].toObject()[u"tools"_s].toArray();
    expect(eq(tools.size(), 1));
    expect(eq(tools.at(0).toObject()[u"name"_s].toString(), u"echo"_s));
    expect(tools.at(0).toObject().contains(u"inputSchema"_s));

    s(request(3, u"tools/call"_s, {{u"name"_s, u"echo"_s}, {u"arguments"_s, QJsonObject{{u"x"_s, u"hi"_s}}}}));
    expect(eq(received[u"x"_s].toString(), u"hi"_s));
    expect(eq(s.sent.at(1)[u"id"_s].toInt(), 3));
    expect(eq(first_text(s.sent.at(1)[u"result"_s].toObject()), u"hi"_s));
  };

  "deferred_reply_keeps_string_id"_test = []
  {
    session_t s;
    ide_protocol::reply_t pending;
    s.tools.push_back(
      {u"wait"_s, {}, {}, [&pending](QJsonObject const &, ide_protocol::reply_t reply) { pending = std::move(reply); }}
    );
    s(
      {{u"jsonrpc"_s, u"2.0"_s},
       {u"id"_s, u"abc"_s},
       {u"method"_s, u"tools/call"_s},
       {u"params"_s, QJsonObject{{u"name"_s, u"wait"_s}}}}
    );
    expect(s.sent.empty());
    pending(ide_protocol::text_result(u"done"_s));
    expect(eq(s.sent.size(), 1uz) >> fatal);
    expect(eq(s.sent[0][u"id"_s].toString(), u"abc"_s));
  };

  "errors"_test = []
  {
    session_t s;
    s(request(4, u"tools/call"_s, {{u"name"_s, u"missing"_s}}));
    expect(eq(s.sent.at(0)[u"error"_s].toObject()[u"code"_s].toInt(), -32602));
    s(request(5, u"no/such"_s));
    expect(eq(s.sent.at(1)[u"error"_s].toObject()[u"code"_s].toInt(), -32601));
    ide_protocol::handle_message("{not json", {}, [&s](QByteArray const & m) { s.sent.push_back(parse(m)); });
    expect(eq(s.sent.at(2)[u"error"_s].toObject()[u"code"_s].toInt(), -32700));
  };

  "notifications_get_no_response"_test = []
  {
    session_t s;
    s({{u"jsonrpc"_s, u"2.0"_s}, {u"method"_s, u"ide_connected"_s}, {u"params"_s, QJsonObject{{u"pid"_s, 1}}}});
    expect(s.sent.empty());
    expect(eq(s.notification, u"ide_connected"_s));
    s(request(6, u"ping"_s));
    expect(eq(s.sent.size(), 1uz));
    expect(s.notification.isEmpty());
  };

  "open_diff_results"_test = []
  {
    auto const saved = ide_protocol::file_saved_result(u"new body"_s)[u"content"_s].toArray();
    expect(eq(saved.at(0).toObject()[u"text"_s].toString(), u"FILE_SAVED"_s));
    expect(eq(saved.at(1).toObject()[u"text"_s].toString(), u"new body"_s));
    expect(ide_protocol::error_result(u"x"_s)[u"isError"_s].toBool());
  };

  "diagnostics_grouped_by_file"_test = []
  {
    std::vector<ide_protocol::diagnostic_t> const list{
      {u"/p/a.cc"_s, u"bad"_s, ide_protocol::severity_e::error, {1, 2}, {1, 5}, u"clang"_s},
      {u"/p/a.cc"_s, u"meh"_s, ide_protocol::severity_e::warning, {3, 0}, {3, 1}, u"clang"_s},
    };
    auto const result{ide_protocol::diagnostics_result({u"/p/a.cc"_s, u"/p/b.cc"_s}, list)};
    auto const files = QJsonDocument::fromJson(first_text(result).toUtf8()).array();
    expect(eq(files.size(), 2) >> fatal);
    auto const a{files.at(0).toObject()};
    expect(eq(a[u"uri"_s].toString(), u"file:///p/a.cc"_s));
    auto const diags = a[u"diagnostics"_s].toArray();
    expect(eq(diags.size(), 2) >> fatal);
    auto const first{diags.at(0).toObject()};
    expect(eq(first[u"severity"_s].toString(), u"Error"_s));
    expect(eq(first[u"range"_s].toObject()[u"start"_s].toObject()[u"character"_s].toInt(), 2));
    expect(eq(diags.at(1).toObject()[u"severity"_s].toString(), u"Warning"_s));
    expect(eq(files.at(1).toObject()[u"diagnostics"_s].toArray().size(), 0));
  };

  "selection_and_mention_notifications"_test = []
  {
    auto const sel{parse(ide_protocol::selection_changed(u"/p/a.cc"_s, u"int x;"_s, {2, 0}, {2, 6}))};
    expect(eq(sel[u"method"_s].toString(), u"selection_changed"_s));
    expect(not sel.contains(u"id"_s));
    auto const params{sel[u"params"_s].toObject()};
    expect(eq(params[u"text"_s].toString(), u"int x;"_s));
    expect(eq(params[u"fileUrl"_s].toString(), u"file:///p/a.cc"_s));
    expect(not params[u"selection"_s].toObject()[u"isEmpty"_s].toBool());
    auto const empty{parse(ide_protocol::selection_changed(u"/p/a.cc"_s, {}, {4, 1}, {4, 1}))};
    expect(empty[u"params"_s].toObject()[u"selection"_s].toObject()[u"isEmpty"_s].toBool());

    auto const mention{parse(ide_protocol::at_mentioned(u"/p/a.cc"_s, 3, 7))[u"params"_s].toObject()};
    expect(eq(mention[u"lineStart"_s].toInt(), 3));
    expect(eq(mention[u"lineEnd"_s].toInt(), 7));
    auto const whole_file{parse(ide_protocol::at_mentioned(u"/p/a.cc"_s, {}, {}))[u"params"_s].toObject()};
    expect(not whole_file.contains(u"lineStart"_s));
  };
  }
