// SPDX-FileCopyrightText: 2026 Artur Bać
// SPDX-License-Identifier: MIT

// Integration test: a real QWebSocket client against ide_server_t on 127.0.0.1

#include <boost/ut.hpp>
#include <ostream>
#include <ide_server.h>
#include <qt_bridge.h>

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QSignalSpy>
#include <QTimer>
#include <QWebSocket>

using namespace boost::ut;
using namespace Qt::StringLiterals;

// lets boost::ut print QString values of failed expectations
[[maybe_unused]]
static auto operator<<(std::ostream & os, QString const & s) -> std::ostream &
  { return os << s.toStdString(); }

namespace
  {
struct no_tools_t final : ide_protocol::ide_tools_t
  {
  auto open_diff(ide_protocol::open_diff_args_t const &, ide_protocol::diff_reply_t reply) -> void override
    { reply(ide_protocol::diff_outcome_e::rejected); }

  auto close_tab(std::string_view) -> void override {}

  auto close_all_diff_tabs() -> std::size_t override { return 0; }

  auto diagnostics(std::string_view) -> ide_protocol::diagnostics_t override { return {}; }
  };

[[nodiscard]]
auto connect_client(QWebSocket & client, quint16 port, QByteArray const & token) -> bool
  {
  QNetworkRequest request{QUrl{u"ws://127.0.0.1:%1"_s.arg(port)}};
  if(not token.isEmpty())
    request.setRawHeader(QByteArray{ide_protocol::auth_header.data(), ide_protocol::auth_header.size()}, token);
  QSignalSpy connected{&client, &QWebSocket::connected};
  client.open(request);
  return connected.wait(2000);
  }

/// waits for the next text message, empty on timeout
[[nodiscard]]
auto next_message(QWebSocket & client) -> QJsonObject
  {
  QSignalSpy spy{&client, &QWebSocket::textMessageReceived};
  if(not spy.wait(2000))
    return {};
  return QJsonDocument::fromJson(spy.at(0).at(0).toString().toUtf8()).object();
  }
  }  // namespace

int main(int argc, char ** argv)
  {
  QCoreApplication app{argc, argv};
  auto const token{ide_protocol::make_auth_token().value()};
  auto const token_bytes{QByteArray::fromStdString(token)};
  no_tools_t tools;
  ide_qt::ide_server_t server{token, tools};
  auto const listened{server.listen()};
  auto const port{listened.value_or(0)};

  "listens_on_free_port"_test = [&] { expect(port != 0); };

  "rejects_missing_token"_test = [&]
  {
    QWebSocket client;
    QSignalSpy closed{&client, &QWebSocket::disconnected};
    // the handshake succeeds, the server closes right after checking the header
    std::ignore = connect_client(client, port, {});
    expect(not closed.isEmpty() or closed.wait(2000));
    expect(client.state() != QAbstractSocket::ConnectedState);
  };

  "accepts_token_and_answers_initialize"_test = [&]
  {
    QWebSocket client;
    expect(connect_client(client, port, token_bytes) >> fatal);
    client.sendTextMessage(uR"({"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"x"}})"_s);
    auto const reply{next_message(client)};
    expect(eq(reply[u"result"_s].toObject()[u"protocolVersion"_s].toString(), u"x"_s));
  };

  "broadcast_after_ide_connected"_test = [&]
  {
    QWebSocket client;
    expect(connect_client(client, port, token_bytes) >> fatal);
    QSignalSpy ready{&server, &ide_qt::ide_server_t::client_connected};
    client.sendTextMessage(uR"({"jsonrpc":"2.0","method":"ide_connected","params":{"pid":1}})"_s);
    expect(ready.wait(2000) >> fatal);
    server.broadcast(ide_protocol::at_mentioned("/p/ż.cc", 1, 2).value());
    auto const message{next_message(client)};
    expect(eq(message[u"method"_s].toString(), u"at_mentioned"_s));
    expect(eq(message[u"params"_s].toObject()[u"filePath"_s].toString(), u"/p/ż.cc"_s));
  };
  }
