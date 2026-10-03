// SPDX-FileCopyrightText: 2026 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

#include <ide_protocol.h>

#include <QList>
#include <QObject>
#include <QWebSocketServer>
#include <string>

class QWebSocket;

namespace ide_qt
  {
/// WebSocket transport of the MCP server on 127.0.0.1, accepts only clients sending the auth token header.
/// Messages are handled by ide_protocol::handle_message, tools must outlive the server.
class ide_server_t : public QObject
  {
  Q_OBJECT

public:
  ide_server_t(std::string auth_token, ide_protocol::ide_tools_t & tools, QObject * parent = nullptr);
  ~ide_server_t() override;

  /// listens on a free port, returns it or 0 on failure
  [[nodiscard]]
  auto listen() -> quint16;

  auto broadcast(std::string_view message) -> void;

Q_SIGNALS:
  /// claude sent ide_connected, it is ready for notifications
  void client_connected();

private:
  auto on_new_connection() -> void;

  QWebSocketServer server_;
  std::string auth_token_;
  ide_protocol::ide_tools_t & tools_;
  QList<QWebSocket *> clients_;
  };
  }  // namespace ide_qt
