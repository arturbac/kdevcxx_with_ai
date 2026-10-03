// SPDX-FileCopyrightText: 2026 Artur Bać
// SPDX-License-Identifier: MIT

#include <boost/ut.hpp>
#include <ostream>
#include <ide_protocol.h>

#include <glaze/glaze.hpp>

#include <algorithm>
#include <string>
#include <vector>

using namespace boost::ut;
using namespace std::string_view_literals;

namespace
  {
[[nodiscard]]
auto parse(std::string_view json) -> glz::generic
  {
  glz::generic value;
  if(glz::read_json(value, json))
    return {};
  return value;
  }

/// records the calls of the protocol into the IDE
struct fake_tools_t final : ide_protocol::ide_tools_t
  {
  std::vector<ide_protocol::open_diff_args_t> opened;
  ide_protocol::diff_reply_t pending;
  std::vector<std::string> closed;
  std::size_t open_tabs{};
  std::string diagnostics_file{"<not called>"};
  ide_protocol::diagnostics_t diagnostics_result;

  auto open_diff(ide_protocol::open_diff_args_t const & args, ide_protocol::diff_reply_t reply) -> void override
    {
    opened.push_back(args);
    pending = std::move(reply);
    }

  auto close_tab(std::string_view tab_name) -> void override { closed.emplace_back(tab_name); }

  auto close_all_diff_tabs() -> std::size_t override { return std::exchange(open_tabs, 0); }

  auto diagnostics(std::string_view file) -> ide_protocol::diagnostics_t override
    {
    diagnostics_file = file;
    return diagnostics_result;
    }
  };

/// runs handle_message and collects the parsed responses
struct session_t
  {
  fake_tools_t tools;
  std::vector<glz::generic> sent;
  std::string notification;

  auto operator()(std::string_view message) -> void
    {
    notification
      = ide_protocol::handle_message(message, tools, [this](std::string reply) { sent.push_back(parse(reply)); });
    }
  };

[[nodiscard]]
auto request(int id, std::string_view method, std::string_view params = "{}") -> std::string
  { return std::format(R"({{"jsonrpc":"2.0","id":{},"method":"{}","params":{}}})", id, method, params); }

[[nodiscard]]
auto call(int id, std::string_view tool, std::string_view arguments = "{}") -> std::string
  { return request(id, "tools/call", std::format(R"({{"name":"{}","arguments":{}}})", tool, arguments)); }

[[nodiscard]]
auto text(glz::generic & response, std::size_t index = 0) -> std::string
  { return response["result"]["content"][std::size_t{index}]["text"].get_string(); }
  }  // namespace

int main()
  {
  // glz::generic values use = since auto x{value} picks the initializer_list ctor and wraps the value in an array

  "auth_token_is_32_lowercase_hex"_test = []
  {
    auto const token{ide_protocol::make_auth_token()};
    expect(token.has_value() >> fatal);
    expect(eq(token->size(), 32uz));
    expect(std::ranges::all_of(*token, [](char c) { return (c >= '0' and c <= '9') or (c >= 'a' and c <= 'f'); }));
    expect(*token != *ide_protocol::make_auth_token());
  };

  "lock_dir_prefers_claude_config_dir"_test = []
  {
    expect(eq(ide_protocol::lock_dir("/home/u/.claude-x", "/home/u").string(), "/home/u/.claude-x/ide"sv));
    expect(eq(ide_protocol::lock_dir("", "/home/u").string(), "/home/u/.claude/ide"sv));
  };

  "lock_file_fields"_test = []
  {
    std::vector<std::string> const folders{"/src/a"};
    auto lock = parse(ide_protocol::lock_file_json(42, folders, "tok"));
    expect(eq(lock["pid"].as<int>(), 42));
    expect(eq(lock["workspaceFolders"].get_array()[0].get_string(), "/src/a"sv));
    expect(eq(lock["ideName"].get_string(), "KDevelop"sv));
    expect(eq(lock["transport"].get_string(), "ws"sv));
    expect(eq(lock["authToken"].get_string(), "tok"sv));
  };

  "launch_command_sets_port"_test = []
  {
    expect(eq(
      ide_protocol::claude_launch_command(1234, "claude --model opus"),
      "env CLAUDE_CODE_SSE_PORT=1234 ENABLE_IDE_INTEGRATION=true claude --model opus"sv
    ));
  };

  "uri_round_trip"_test = []
  {
    expect(eq(ide_protocol::file_uri("/a b/c.cc"), "file:///a%20b/c.cc"sv));
    expect(eq(ide_protocol::file_uri("/x/ż#1%.cc"), "file:///x/%C5%BC%231%25.cc"sv));
    expect(eq(ide_protocol::file_uri("/k-_.~!$&'()*+,;=:@"), "file:///k-_.~!$&'()*+,;=:@"sv));
    expect(eq(ide_protocol::uri_to_path("file:///a%20b/c.cc"), "/a b/c.cc"sv));
    expect(eq(ide_protocol::uri_to_path(ide_protocol::file_uri("/x/ż#1%.cc")), "/x/ż#1%.cc"sv));
    expect(eq(ide_protocol::uri_to_path("file://localhost/p/a.cc"), "/p/a.cc"sv));
    expect(eq(ide_protocol::uri_to_path("file:/p/a.cc"), "/p/a.cc"sv));
    expect(eq(ide_protocol::uri_to_path("file:///bad%2"), "/bad%2"sv));
    expect(eq(ide_protocol::uri_to_path("/plain/path.cc"), "/plain/path.cc"sv));
  };

  "initialize_echoes_protocol_version"_test = []
  {
    session_t s;
    s(request(1, "initialize", R"({"protocolVersion":"2025-03-26"})"));
    expect(eq(s.sent.size(), 1uz) >> fatal);
    auto & result{s.sent[0]["result"]};
    expect(eq(s.sent[0]["id"].as<int>(), 1));
    expect(eq(result["protocolVersion"].get_string(), "2025-03-26"sv));
    expect(not result["capabilities"]["tools"]["listChanged"].get_boolean());
    expect(eq(result["serverInfo"]["name"].get_string(), "kdevcxx_with_ai"sv));
    s(request(2, "initialize"));
    expect(eq(s.sent.at(1)["result"]["protocolVersion"].get_string(), "2025-06-18"sv));
  };

  "tools_list"_test = []
  {
    session_t s;
    s(request(2, "tools/list"));
    auto & tools{s.sent.at(0)["result"]["tools"]};
    expect(eq(tools.size(), 4uz) >> fatal);
    std::vector<std::string> names;
    for(auto & tool: tools.get_array())
      {
      names.push_back(tool["name"].get_string());
      expect(eq(tool["inputSchema"]["type"].get_string(), "object"sv));
      }
    expect(names == std::vector<std::string>{"openDiff", "close_tab", "closeAllDiffTabs", "getDiagnostics"});
  };

  "open_diff_outcomes_keep_id"_test = []
  {
    using enum ide_protocol::diff_outcome_e;
    session_t s;
    auto const args{
      R"({"old_file_path":"/p/a.cc","new_file_path":"/p/a.cc","new_file_contents":"new body","tab_name":"t"})"sv
    };
    s(R"({"jsonrpc":"2.0","id":"abc","method":"tools/call","params":{"name":"openDiff","arguments":)"
      + std::string{args} + "}}");
    expect(s.sent.empty());
    expect(eq(s.tools.opened.size(), 1uz) >> fatal);
    expect(eq(s.tools.opened[0].old_file_path, "/p/a.cc"sv));
    expect(eq(s.tools.opened[0].new_file_contents, "new body"sv));
    expect(eq(s.tools.opened[0].tab_name, "t"sv));
    s.tools.pending(accepted);
    expect(eq(s.sent.size(), 1uz) >> fatal);
    expect(eq(s.sent[0]["id"].get_string(), "abc"sv));
    expect(eq(text(s.sent[0], 0), "FILE_SAVED"sv));
    expect(eq(text(s.sent[0], 1), "new body"sv));

    s(call(7, "openDiff", args));
    s.tools.pending(rejected);
    expect(eq(text(s.sent.at(1)), "DIFF_REJECTED"sv));
    s(call(8, "openDiff", args));
    s.tools.pending(tab_closed);
    expect(eq(text(s.sent.at(2)), "TAB_CLOSED"sv));
    expect(eq(s.sent.at(2)["id"].as<int>(), 8));
  };

  "close_tabs"_test = []
  {
    session_t s;
    s(call(3, "close_tab", R"({"tab_name":"t1"})"));
    expect(s.tools.closed == std::vector<std::string>{"t1"});
    expect(eq(text(s.sent.at(0)), "TAB_CLOSED"sv));
    s.tools.open_tabs = 2;
    s(call(4, "closeAllDiffTabs"));
    expect(eq(text(s.sent.at(1)), "CLOSED_2_DIFF_TABS"sv));
  };

  "diagnostics_grouped_by_file"_test = []
  {
    session_t s;
    s.tools.diagnostics_result = {
      .files = {"/p/b.cc", "/p/a.cc"},
      .diagnostics = {
        {"/p/a.cc", "bad", ide_protocol::severity_e::error, {1, 2}, {1, 5}, "clang"},
        {"/p/a.cc", "meh", ide_protocol::severity_e::warning, {3, 0}, {3, 1}, "clang"}
      }
    };
    s(call(5, "getDiagnostics", R"({"uri":"file:///p/a%20b.cc"})"));
    expect(eq(s.tools.diagnostics_file, "/p/a b.cc"sv));
    auto files = parse(text(s.sent.at(0)));
    expect(eq(files.size(), 2uz) >> fatal);
    auto & a{files.get_array()[0]};
    expect(eq(a["uri"].get_string(), "file:///p/a.cc"sv));
    auto & diags{a["diagnostics"]};
    expect(eq(diags.size(), 2uz) >> fatal);
    expect(eq(diags.get_array()[0]["severity"].get_string(), "Error"sv));
    expect(eq(diags.get_array()[0]["message"].get_string(), "bad"sv));
    expect(eq(diags.get_array()[0]["source"].get_string(), "clang"sv));
    expect(eq(diags.get_array()[0]["range"]["start"]["character"].as<int>(), 2));
    expect(eq(diags.get_array()[0]["range"]["end"]["character"].as<int>(), 5));
    expect(eq(diags.get_array()[1]["severity"].get_string(), "Warning"sv));
    expect(eq(files.get_array()[1]["diagnostics"].size(), 0uz));

    s(call(6, "getDiagnostics"));
    expect(s.tools.diagnostics_file.empty());
  };

  "severity_names"_test = []
  {
    using enum ide_protocol::severity_e;
    expect(eq(ide_protocol::severity_name(error), "Error"sv));
    expect(eq(ide_protocol::severity_name(warning), "Warning"sv));
    expect(eq(ide_protocol::severity_name(info), "Info"sv));
    expect(eq(ide_protocol::severity_name(hint), "Hint"sv));
  };

  "errors"_test = []
  {
    session_t s;
    s(call(4, "missing"));
    expect(eq(s.sent.at(0)["error"]["code"].as<int>(), -32602));
    expect(eq(s.sent.at(0)["id"].as<int>(), 4));
    s(request(5, "no/such"));
    expect(eq(s.sent.at(1)["error"]["code"].as<int>(), -32601));
    s("{not json");
    expect(eq(s.sent.at(2)["error"]["code"].as<int>(), -32700));
    expect(s.sent.at(2)["id"].is_null());
    s(call(6, "close_tab", R"({"tab_name":1})"));
    expect(eq(s.sent.at(3)["error"]["code"].as<int>(), -32602));
    expect(s.tools.closed.empty());
  };

  "notifications_get_no_response"_test = []
  {
    session_t s;
    s(R"({"jsonrpc":"2.0","method":"ide_connected","params":{"pid":1}})");
    expect(s.sent.empty());
    expect(eq(s.notification, "ide_connected"sv));
    s(request(6, "ping"));
    expect(eq(s.sent.size(), 1uz) >> fatal);
    expect(s.sent[0]["result"].get_object().empty());
    expect(s.notification.empty());
    // a response from the client (id, no method) needs no answer either
    s(R"({"jsonrpc":"2.0","id":9,"result":{}})");
    expect(eq(s.sent.size(), 1uz));
  };

  "selection_and_mention_notifications"_test = []
  {
    auto sel = parse(ide_protocol::selection_changed("/p/a.cc", "int x;", {2, 0}, {2, 6}));
    expect(eq(sel["method"].get_string(), "selection_changed"sv));
    expect(not sel.contains("id"));
    auto & params{sel["params"]};
    expect(eq(params["text"].get_string(), "int x;"sv));
    expect(eq(params["filePath"].get_string(), "/p/a.cc"sv));
    expect(eq(params["fileUrl"].get_string(), "file:///p/a.cc"sv));
    expect(eq(params["selection"]["end"]["character"].as<int>(), 6));
    expect(not params["selection"]["isEmpty"].get_boolean());
    auto empty = parse(ide_protocol::selection_changed("/p/a.cc", "", {4, 1}, {4, 1}));
    expect(empty["params"]["selection"]["isEmpty"].get_boolean());

    auto mention = parse(ide_protocol::at_mentioned("/p/a.cc", 3, 7));
    expect(eq(mention["params"]["lineStart"].as<int>(), 3));
    expect(eq(mention["params"]["lineEnd"].as<int>(), 7));
    auto whole_file = parse(ide_protocol::at_mentioned("/p/a.cc", {}, {}));
    expect(not whole_file["params"].contains("lineStart"));
    expect(eq(whole_file["params"]["filePath"].get_string(), "/p/a.cc"sv));
  };
  }
