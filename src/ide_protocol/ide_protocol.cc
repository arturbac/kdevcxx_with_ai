// SPDX-FileCopyrightText: 2026 Artur Bać
// SPDX-License-Identifier: MIT

#include "ide_protocol.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMap>
#include <QRandomGenerator>
#include <QUrl>
#include <algorithm>
#include <array>

namespace ide_protocol
  {
namespace
  {
  using namespace Qt::StringLiterals;

  enum struct rpc_error_e
    {
    parse_error = -32700,
    method_not_found = -32601,
    invalid_params = -32602
    };

  [[nodiscard]]
  auto to_json(QJsonObject const & object) -> QByteArray
    { return QJsonDocument{object}.toJson(QJsonDocument::Compact); }

  [[nodiscard]]
  auto notification(QString const & method, QJsonObject const & params) -> QByteArray
    { return to_json({{u"jsonrpc"_s, u"2.0"_s}, {u"method"_s, method}, {u"params"_s, params}}); }

  [[nodiscard]]
  auto response(QJsonValue const & id, QJsonObject const & result) -> QByteArray
    { return to_json({{u"jsonrpc"_s, u"2.0"_s}, {u"id"_s, id}, {u"result"_s, result}}); }

  [[nodiscard]]
  auto error_response(QJsonValue const & id, rpc_error_e code, QString const & message) -> QByteArray
    {
    return to_json(
      {{u"jsonrpc"_s, u"2.0"_s},
       {u"id"_s, id},
       {u"error"_s, QJsonObject{{u"code"_s, static_cast<int>(code)}, {u"message"_s, message}}}}
    );
    }

  [[nodiscard]]
  auto to_json(position_t pos) -> QJsonObject
    { return {{u"line"_s, pos.line}, {u"character"_s, pos.character}}; }

  [[nodiscard]]
  auto severity_name(severity_e severity) -> QString
    {
    switch(severity)
      {
      case severity_e::error:   return u"Error"_s;
      case severity_e::warning: return u"Warning"_s;
      case severity_e::info:    return u"Info"_s;
      case severity_e::hint:    return u"Hint"_s;
      }
    return u"Info"_s;
    }

  [[nodiscard]]
  auto initialize_result(QJsonObject const & params) -> QJsonObject
    {
    // the server speaks whatever version the client asks for, it only uses tools
    auto version{params.value(u"protocolVersion"_s).toString(u"2025-06-18"_s)};
    return {
      {u"protocolVersion"_s, version},
      {u"capabilities"_s, QJsonObject{{u"tools"_s, QJsonObject{{u"listChanged"_s, false}}}}},
      {u"serverInfo"_s,
       QJsonObject{{u"name"_s, u"kdevcxx_with_ai"_s}, {u"version"_s, QStringLiteral(KDEVCXX_WITH_AI_VERSION)}}}
    };
    }

  [[nodiscard]]
  auto tools_list_result(std::span<tool_t const> tools) -> QJsonObject
    {
    QJsonArray list;
    for(auto const & tool: tools)
      list.append(
        QJsonObject{{u"name"_s, tool.name}, {u"description"_s, tool.description}, {u"inputSchema"_s, tool.input_schema}}
      );
    return {{u"tools"_s, list}};
    }
  }  // namespace

auto make_auth_token() -> QString
  {
  std::array<quint32, 4> words{};
  QRandomGenerator::system()->fillRange(words.data(), words.size());
  QString token;
  for(auto word: words)
    token += u"%1"_s.arg(word, 8, 16, u'0');
  return token;
  }

auto lock_dir(QString const & claude_config_dir, QString const & home) -> QString
  {
  if(not claude_config_dir.isEmpty())
    return QDir{claude_config_dir}.filePath(u"ide"_s);
  return QDir{home}.filePath(u".claude/ide"_s);
  }

auto lock_file_json(qint64 pid, QStringList const & workspace_folders, QString const & auth_token) -> QByteArray
  {
  return to_json(
    {{u"pid"_s, pid},
     {u"workspaceFolders"_s, QJsonArray::fromStringList(workspace_folders)},
     {u"ideName"_s, QString::fromLatin1(ide_name)},
     {u"transport"_s, u"ws"_s},
     {u"authToken"_s, auth_token}}
  );
  }

auto claude_launch_command(quint16 port, QString const & claude_command) -> QString
  {
  // env works the same in sh, bash, zsh and fish
  return u"env CLAUDE_CODE_SSE_PORT=%1 ENABLE_IDE_INTEGRATION=true %2"_s.arg(port).arg(claude_command);
  }

auto file_uri(QString const & path) -> QString { return QUrl::fromLocalFile(path).toString(); }

auto uri_to_path(QString const & uri) -> QString
  {
  if(uri.startsWith(u"file:"_s))
    return QUrl{uri}.toLocalFile();
  return uri;
  }

auto text_result(QString const & text) -> QJsonObject
  { return {{u"content"_s, QJsonArray{QJsonObject{{u"type"_s, u"text"_s}, {u"text"_s, text}}}}}; }

auto error_result(QString const & text) -> QJsonObject
  {
  auto result{text_result(text)};
  result.insert(u"isError"_s, true);
  return result;
  }

auto file_saved_result(QString const & contents) -> QJsonObject
  {
  return {
    {u"content"_s,
     QJsonArray{
       QJsonObject{{u"type"_s, u"text"_s}, {u"text"_s, u"FILE_SAVED"_s}},
       QJsonObject{{u"type"_s, u"text"_s}, {u"text"_s, contents}}
     }}
  };
  }

auto diagnostics_result(QStringList const & files, std::span<diagnostic_t const> diagnostics) -> QJsonObject
  {
  QMap<QString, QJsonArray> by_file;
  for(auto const & file: files)
    by_file[file];
  for(auto const & d: diagnostics)
    by_file[d.file_path].append(
      QJsonObject{
        {u"message"_s, d.message},
        {u"severity"_s, severity_name(d.severity)},
        {u"range"_s, QJsonObject{{u"start"_s, to_json(d.start)}, {u"end"_s, to_json(d.end)}}},
        {u"source"_s, d.source}
      }
    );

  QJsonArray result;
  for(auto it{by_file.cbegin()}; it != by_file.cend(); ++it)
    result.append(QJsonObject{{u"uri"_s, file_uri(it.key())}, {u"diagnostics"_s, it.value()}});
  return text_result(QString::fromUtf8(QJsonDocument{result}.toJson(QJsonDocument::Compact)));
  }

auto selection_changed(QString const & path, QString const & text, position_t start, position_t end) -> QByteArray
  {
  bool const is_empty{start.line == end.line and start.character == end.character};
  return notification(
    u"selection_changed"_s,
    {{u"text"_s, text},
     {u"filePath"_s, path},
     {u"fileUrl"_s, file_uri(path)},
     {u"selection"_s, QJsonObject{{u"start"_s, to_json(start)}, {u"end"_s, to_json(end)}, {u"isEmpty"_s, is_empty}}}}
  );
  }

auto at_mentioned(QString const & path, std::optional<int> line_start, std::optional<int> line_end) -> QByteArray
  {
  QJsonObject params{{u"filePath"_s, path}};
  if(line_start)
    params.insert(u"lineStart"_s, *line_start);
  if(line_end)
    params.insert(u"lineEnd"_s, *line_end);
  return notification(u"at_mentioned"_s, params);
  }

auto handle_message(QByteArray const & message, std::span<tool_t const> tools, send_t const & send) -> QString
  {
  QJsonParseError parse_error;
  auto const doc{QJsonDocument::fromJson(message, &parse_error)};
  if(parse_error.error != QJsonParseError::NoError or not doc.isObject())
    {
    send(error_response(QJsonValue::Null, rpc_error_e::parse_error, u"Parse error"_s));
    return {};
    }

  auto const request{doc.object()};
  auto const method{request.value(u"method"_s).toString()};
  // responses and notifications (ide_connected, notifications/initialized) need no answer
  if(not request.contains(u"id"_s))
    return method;
  if(method.isEmpty())
    return {};

  auto const id{request.value(u"id"_s)};
  auto const params{request.value(u"params"_s).toObject()};

  if(method == u"initialize"_s)
    send(response(id, initialize_result(params)));
  else if(method == u"tools/list"_s)
    send(response(id, tools_list_result(tools)));
  else if(method == u"ping"_s)
    send(response(id, {}));
  else if(method == u"tools/call"_s)
    {
    auto const name{params.value(u"name"_s).toString()};
    auto const tool{std::ranges::find(tools, name, &tool_t::name)};
    if(tool == tools.end())
      send(error_response(id, rpc_error_e::invalid_params, u"Unknown tool: %1"_s.arg(name)));
    else
      tool->call(
        params.value(u"arguments"_s).toObject(), [send, id](QJsonObject const & result) { send(response(id, result)); }
      );
    }
  else
    send(error_response(id, rpc_error_e::method_not_found, u"Method not found: %1"_s.arg(method)));
  return {};
  }
  }  // namespace ide_protocol
