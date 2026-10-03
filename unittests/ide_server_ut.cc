// SPDX-FileCopyrightText: 2026 Artur Bać
// SPDX-License-Identifier: MIT

// Integration test: a real QWebSocket client against ide_server_t on 127.0.0.1

#include <boost/ut.hpp>
#include <ostream>
#include <ide_server.h>

#include <QCoreApplication>
#include <QJsonDocument>
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
[[nodiscard]]
auto connect_client(QWebSocket & client, quint16 port, QByteArray const & token) -> bool
  {
  QNetworkRequest request{QUrl{u"ws://127.0.0.1:%1"_s.arg(port)}};
  if(not token.isEmpty())
    request.setRawHeader(ide_protocol::auth_header, token);
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
  auto const token{ide_protocol::make_auth_token()};
  ide_protocol::ide_server_t server{token, {}};
  auto const port{server.listen()};

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
    expect(connect_client(client, port, token.toLatin1()) >> fatal);
    client.sendTextMessage(uR"({"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"x"}})"_s);
    auto const reply{next_message(client)};
    expect(eq(reply[u"result"_s].toObject()[u"protocolVersion"_s].toString(), u"x"_s));
  };

  "broadcast_after_ide_connected"_test = [&]
  {
    QWebSocket client;
    expect(connect_client(client, port, token.toLatin1()) >> fatal);
    QSignalSpy ready{&server, &ide_protocol::ide_server_t::client_connected};
    client.sendTextMessage(uR"({"jsonrpc":"2.0","method":"ide_connected","params":{"pid":1}})"_s);
    expect(ready.wait(2000) >> fatal);
    server.broadcast(ide_protocol::at_mentioned(u"/p/a.cc"_s, 1, 2));
    expect(eq(next_message(client)[u"method"_s].toString(), u"at_mentioned"_s));
  };
  }
