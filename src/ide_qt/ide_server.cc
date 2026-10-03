// SPDX-FileCopyrightText: 2026 Artur Bać
// SPDX-License-Identifier: MIT

#include "ide_server.h"
#include "qt_bridge.h"

#include <QPointer>
#include <QWebSocket>

namespace ide_qt
  {
using namespace Qt::StringLiterals;

ide_server_t::ide_server_t(std::string auth_token, ide_protocol::ide_tools_t & tools, QObject * parent) :
    QObject{parent},
    server_{u"kdevcxx_with_ai"_s, QWebSocketServer::NonSecureMode},
    auth_token_{std::move(auth_token)},
    tools_{tools}
  { connect(&server_, &QWebSocketServer::newConnection, this, &ide_server_t::on_new_connection); }

ide_server_t::~ide_server_t()
  {
  // sockets die after the members, their disconnected signal must not reach clients_ any more
  for(auto * client: std::as_const(clients_))
    client->disconnect(this);
  }

auto ide_server_t::listen() -> quint16
  {
  if(not server_.listen(QHostAddress::LocalHost, 0))
    return 0;
  return server_.serverPort();
  }

auto ide_server_t::broadcast(std::string_view message) -> void
  {
  auto const text{to_qt(message)};
  for(auto * client: std::as_const(clients_))
    client->sendTextMessage(text);
  }

auto ide_server_t::on_new_connection() -> void
  {
  while(auto * client{server_.nextPendingConnection()})
    {
    client->setParent(this);
    auto const header{client->request().rawHeader(QByteArrayView{ide_protocol::auth_header})};
    if(header != QByteArrayView{auth_token_})
      {
      client->close(QWebSocketProtocol::CloseCodePolicyViolated, u"invalid auth token"_s);
      client->deleteLater();
      continue;
      }

    clients_.append(client);
    connect(
      client,
      &QWebSocket::textMessageReceived,
      this,
      [this, client](QString const & message)
      {
        // tools may reply after the client is gone (openDiff waits for the user)
        QPointer<QWebSocket> guard{client};
        auto const notification{ide_protocol::handle_message(
          to_std(message),
          tools_,
          [guard](std::string reply)
          {
            if(guard)
              guard->sendTextMessage(to_qt(reply));
          }
        )};
        if(notification == "ide_connected")
          Q_EMIT client_connected();
      }
    );
    connect(
      client,
      &QWebSocket::disconnected,
      this,
      [this, client]
      {
        clients_.removeOne(client);
        client->deleteLater();
      }
    );
    }
  }
  }  // namespace ide_qt
