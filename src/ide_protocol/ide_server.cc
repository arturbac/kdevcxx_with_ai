// SPDX-FileCopyrightText: 2026 Artur Bać
// SPDX-License-Identifier: MIT

#include "ide_server.h"

#include <QPointer>
#include <QWebSocket>

namespace ide_protocol
  {
using namespace Qt::StringLiterals;

ide_server_t::ide_server_t(QString auth_token, std::vector<tool_t> tools, QObject * parent) :
    QObject{parent},
    server_{u"kdevcxx_with_ai"_s, QWebSocketServer::NonSecureMode},
    auth_token_{std::move(auth_token)},
    tools_{std::move(tools)}
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

auto ide_server_t::broadcast(QByteArray const & message) -> void
  {
  for(auto * client: std::as_const(clients_))
    client->sendTextMessage(QString::fromUtf8(message));
  }

auto ide_server_t::on_new_connection() -> void
  {
  while(auto * client{server_.nextPendingConnection()})
    {
    client->setParent(this);
    if(client->request().rawHeader(auth_header) != auth_token_.toLatin1())
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
        auto const notification{handle_message(
          message.toUtf8(),
          tools_,
          [guard](QByteArray const & reply)
          {
            if(guard)
              guard->sendTextMessage(QString::fromUtf8(reply));
          }
        )};
        if(notification == u"ide_connected"_s)
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
  }  // namespace ide_protocol
