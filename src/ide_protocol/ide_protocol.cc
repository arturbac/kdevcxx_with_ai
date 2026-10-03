// SPDX-FileCopyrightText: 2026 Artur Bać
// SPDX-License-Identifier: MIT

#include "ide_protocol.h"

#include <glaze/glaze.hpp>
#include <simple_enum/generic_error_category_impl.hpp>

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstdio>
#include <format>
#include <map>
#include <unistd.h>

namespace ide_protocol
  {
// message shapes, a named namespace because glaze reflection needs types with linkage
namespace json
  {
  enum struct rpc_error_e
    {
    parse_error = -32700,
    method_not_found = -32601,
    invalid_params = -32602,
    internal_error = -32603
    };

  struct text_content_t
    {
    std::string_view type{"text"};
    std::string text;
    };

  /// tools/call result {"content":[{"type":"text","text":...}],"isError":true}
  struct tool_result_t
    {
    std::vector<text_content_t> content;
    std::optional<bool> is_error{};
    };

  struct empty_t
    {
    };

  template<typename result_type>
  struct response_t
    {
    std::string_view jsonrpc{"2.0"};
    glz::raw_json_view id;
    result_type result;
    };

  struct rpc_error_t
    {
    int code;
    std::string message;
    };

  struct error_response_t
    {
    std::string_view jsonrpc{"2.0"};
    glz::raw_json_view id;
    rpc_error_t error;
    };

  template<typename params_type>
  struct notification_t
    {
    std::string_view jsonrpc{"2.0"};
    std::string_view method;
    params_type params;
    };

  struct request_t
    {
    std::optional<glz::raw_json> id;
    std::string method;
    glz::raw_json params{"{}"};
    };

  struct initialize_params_t
    {
    // the server speaks whatever version the client asks for, it only uses tools
    std::string protocol_version{"2025-06-18"};
    };

  struct tools_capability_t
    { bool list_changed{}; };

  struct capabilities_t
    { tools_capability_t tools; };

  struct server_info_t
    {
    std::string_view name{"kdevcxx_with_ai"};
    std::string_view version{KDEVCXX_WITH_AI_VERSION};
    };

  struct initialize_result_t
    {
    std::string protocol_version;
    capabilities_t capabilities{};
    server_info_t server_info{};
    };

  struct tool_info_t
    {
    std::string_view name;
    std::string_view description;
    glz::raw_json_view input_schema;
    };

  struct tools_list_t
    { std::span<tool_info_t const> tools; };

  struct call_params_t
    {
    std::string name;
    glz::raw_json arguments{"{}"};
    };

  struct close_tab_args_t
    { std::string tab_name; };

  struct diagnostics_args_t
    { std::string uri; };

  struct range_t
    {
    position_t start;
    position_t end;
    };

  struct diagnostic_json_t
    {
    std::string_view message;
    std::string_view severity;
    range_t range;
    std::string_view source;
    };

  struct file_diagnostics_t
    {
    std::string uri;
    std::vector<diagnostic_json_t> diagnostics;
    };

  struct lock_file_t
    {
    std::int64_t pid;
    std::span<std::string const> workspace_folders;
    std::string_view ide_name;
    std::string_view transport{"ws"};
    std::string_view auth_token;
    };

  struct selection_t
    {
    position_t start;
    position_t end;
    bool is_empty;
    };

  struct selection_params_t
    {
    std::string_view text;
    std::string_view file_path;
    std::string file_url;
    selection_t selection;
    };

  struct at_mentioned_params_t
    {
    std::string_view file_path;
    std::optional<int> line_start;
    std::optional<int> line_end;
    };
  }  // namespace json
  }  // namespace ide_protocol

// JSON keys of the protocol are camelCase, the C++ members follow the project naming
template<>
struct glz::meta<ide_protocol::json::tool_result_t> : glz::camel_case
  {
  };

template<>
struct glz::meta<ide_protocol::json::initialize_params_t> : glz::camel_case
  {
  };

template<>
struct glz::meta<ide_protocol::json::tools_capability_t> : glz::camel_case
  {
  };

template<>
struct glz::meta<ide_protocol::json::initialize_result_t> : glz::camel_case
  {
  };

template<>
struct glz::meta<ide_protocol::json::tool_info_t> : glz::camel_case
  {
  };

template<>
struct glz::meta<ide_protocol::json::lock_file_t> : glz::camel_case
  {
  };

template<>
struct glz::meta<ide_protocol::json::selection_t> : glz::camel_case
  {
  };

template<>
struct glz::meta<ide_protocol::json::selection_params_t> : glz::camel_case
  {
  };

template<>
struct glz::meta<ide_protocol::json::at_mentioned_params_t> : glz::camel_case
  {
  };

namespace ide_protocol
  {
using namespace json;

namespace
  {
  // messages may come as views into larger buffers, unknown keys are allowed by the protocol
  inline constexpr glz::opts read_opts{.null_terminated = false, .error_on_unknown_keys = false};

  [[nodiscard]]
  auto is_pchar(char c) -> bool
    {
    // unreserved, sub-delims, ":" and "@" (RFC 3986), plus "/" between segments
    return (c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z') or (c >= '0' and c <= '9')
           or std::string_view{"-._~!$&'()*+,;=:@/"}.contains(c);
    }

  [[nodiscard]]
  auto hex_value(char c) -> int
    {
    if(c >= '0' and c <= '9')
      return c - '0';
    if(c >= 'a' and c <= 'f')
      return c - 'a' + 10;
    if(c >= 'A' and c <= 'F')
      return c - 'A' + 10;
    return -1;
    }

  /// invalid escapes are kept as they are
  [[nodiscard]]
  auto percent_decode(std::string_view text) -> std::string
    {
    std::string out;
    out.reserve(text.size());
    for(std::size_t i{}; i != text.size(); ++i)
      {
      if(text[i] == '%' and i + 2 < text.size())
        {
        auto const high{hex_value(text[i + 1])};
        auto const low{hex_value(text[i + 2])};
        if(high >= 0 and low >= 0)
          {
          out.push_back(static_cast<char>(high * 16 + low));
          i += 2;
          continue;
          }
        }
      out.push_back(text[i]);
      }
    return out;
    }

  [[nodiscard]]
  auto make_file_uri(std::string_view path) -> std::string
    {
    std::string uri{"file://"};
    uri.reserve(uri.size() + path.size());
    for(auto c: path)
      if(is_pchar(c))
        uri.push_back(c);
      else
        std::format_to(std::back_inserter(uri), "%{:02X}", static_cast<unsigned char>(c));
    return uri;
    }

  [[nodiscard]]
  auto path_from_uri(std::string_view uri) -> std::string
    {
    constexpr std::string_view scheme{"file:"};
    if(not uri.starts_with(scheme))
      return std::string{uri};
    auto rest{uri.substr(scheme.size())};
    // file://host/path: the authority (empty or localhost) ends at the next slash
    if(rest.starts_with("//"))
      rest = rest.substr(std::min(rest.find('/', 2), rest.size()));
    return percent_decode(rest);
    }

  /// text of the exception being handled; call only inside a catch block
  [[nodiscard]]
  auto current_exception_message() -> std::string
    {
    try
      {
      throw;
      }
    catch(std::exception const & e)
      {
      return e.what();
      }
    catch(...)
      {
      return "unknown exception";
      }
    }

  /// last resort when not even an error response can be built (out of memory); the core has no logger
  auto report_lost_error(std::string_view where) noexcept -> void
    {
    std::fprintf(stderr, "kdevcxx_with_ai: %.*s: error response lost\n", static_cast<int>(where.size()), where.data());
    }

  /// all written types are plain data, writing cannot fail
  [[nodiscard]]
  auto to_json(auto const & value) -> std::string
    {
    std::string out;
    std::ignore = glz::write_json(value, out);
    return out;
    }

  template<typename value_type>
  [[nodiscard]]
  auto from_json(std::string_view json) -> std::optional<value_type>
    {
    value_type value{};
    if(glz::read<read_opts>(value, json))
      return std::nullopt;
    return value;
    }

  [[nodiscard]]
  auto response(std::string_view id, auto result) -> std::string
    { return to_json(response_t<decltype(result)>{.id = glz::raw_json_view{id}, .result = std::move(result)}); }

  [[nodiscard]]
  auto error_response(std::string_view id, rpc_error_e code, std::string message) -> std::string
    {
    return to_json(
      error_response_t{
        .id = glz::raw_json_view{id}, .error = {.code = static_cast<int>(code), .message = std::move(message)}
      }
    );
    }

  [[nodiscard]]
  auto notification(std::string_view method, auto params) -> std::string
    { return to_json(notification_t<decltype(params)>{.method = method, .params = std::move(params)}); }

  [[nodiscard]]
  auto text_result(std::string text) -> tool_result_t
    { return {.content = {{.text = std::move(text)}}}; }

  [[nodiscard]]
  auto diagnostics_json(diagnostics_t const & input) -> std::string
    {
    // sorted by path, files without diagnostics are listed too
    std::map<std::string_view, std::vector<diagnostic_json_t>> by_file;
    for(auto const & file: input.files)
      by_file[file];
    for(auto const & d: input.diagnostics)
      by_file[d.file_path].push_back(
        {.message = d.message,
         .severity = severity_name(d.severity),
         .range = {.start = d.start, .end = d.end},
         .source = d.source}
      );

    std::vector<file_diagnostics_t> result;
    result.reserve(by_file.size());
    for(auto & [file, diagnostics]: by_file)
      result.push_back({.uri = make_file_uri(file), .diagnostics = std::move(diagnostics)});
    return to_json(result);
    }

  // raw_json_view is not a literal type
  std::array const tools_info{
    tool_info_t{
      .name = "openDiff",
      .description = "Open a diff of a proposed file change for the user to accept or reject (blocking)",
      .input_schema
      = glz::raw_json_view{R"({"type":"object","properties":{)"
                           R"("old_file_path":{"type":"string","description":"Path of the file to change"},)"
                           R"("new_file_path":{"type":"string","description":"Path of the changed file"},)"
                           R"("new_file_contents":{"type":"string","description":"Proposed contents"},)"
                           R"("tab_name":{"type":"string","description":"Name of the diff tab"}},)"
                           R"("required":["old_file_path","new_file_path","new_file_contents","tab_name"]})"}
    },
    tool_info_t{
      .name = "close_tab",
      .description = "Close a diff tab by name",
      .input_schema = glz::
        raw_json_view{R"({"type":"object","properties":{"tab_name":{"type":"string","description":"Name of the diff tab"}},)"
                      R"("required":["tab_name"]})"}
    },
    tool_info_t{
      .name = "closeAllDiffTabs",
      .description = "Close all diff tabs",
      .input_schema = glz::raw_json_view{R"({"type":"object","properties":{}})"}
    },
    tool_info_t{
      .name = "getDiagnostics",
      .description = "Get language diagnostics (errors, warnings) from KDevelop for a file, or for all open files when "
                     "uri is omitted",
      .input_schema = glz::raw_json_view{
        R"({"type":"object","properties":{"uri":{"type":"string","description":"file:// URI of the file"}}})"
      }
    }
  };

  /// tools/call, the reply may be sent later (openDiff waits for the user)
  auto call_tool(std::string id, call_params_t const & params, ide_tools_t & tools, send_t const & send) -> void
    {
    auto const invalid_arguments{
      [&] { send(error_response(id, rpc_error_e::invalid_params, std::format("Invalid arguments: {}", params.name))); }
    };
    auto const reply{[&](tool_result_t result) { send(response(id, std::move(result))); }};

    if(params.name == "openDiff")
      {
      auto args{from_json<open_diff_args_t>(params.arguments.str)};
      if(not args)
        return invalid_arguments();
      tools.open_diff(
        *args,
        [send, id, contents = args->new_file_contents](diff_outcome_e outcome) noexcept
        {
          // called later from the event loop, nothing may escape
          try
            {
            tool_result_t result;
            switch(outcome)
              {
              case diff_outcome_e::accepted:
                result = {.content = {{.text = std::string{file_saved}}, {.text = contents}}};
                break;
              case diff_outcome_e::rejected:   result = text_result(std::string{diff_rejected}); break;
              case diff_outcome_e::tab_closed: result = text_result(std::string{tab_closed}); break;
              }
            send(response(id, std::move(result)));
            }
          catch(...)
            {
            try
              {
              send(error_response(id, rpc_error_e::internal_error, current_exception_message()));
              }
            catch(...)
              {
              report_lost_error("openDiff");
              }
            }
        }
      );
      }
    else if(params.name == "close_tab")
      {
      auto const args{from_json<close_tab_args_t>(params.arguments.str)};
      if(not args)
        return invalid_arguments();
      tools.close_tab(args->tab_name);
      reply(text_result(std::string{tab_closed}));
      }
    else if(params.name == "closeAllDiffTabs")
      reply(text_result(std::format("CLOSED_{}_DIFF_TABS", tools.close_all_diff_tabs())));
    else if(params.name == "getDiagnostics")
      {
      auto const args{from_json<diagnostics_args_t>(params.arguments.str)};
      if(not args)
        return invalid_arguments();
      auto const file{args->uri.empty() ? std::string{} : path_from_uri(args->uri)};
      reply(text_result(diagnostics_json(tools.diagnostics(file))));
      }
    else
      send(error_response(id, rpc_error_e::invalid_params, std::format("Unknown tool: {}", params.name)));
    }

  }  // namespace

auto make_error_code(ide_error_e error) noexcept -> std::error_code
  { return simple_enum::generic_error_category<ide_error_e>::make_error_code(error); }

auto current_exception_error() noexcept -> std::error_code
  {
  try
    {
    throw;
    }
  catch(std::bad_alloc const &)
    {
    return make_error_code(ide_error_e::out_of_memory);
    }
  catch(std::system_error const & e)
    {
    return e.code();
    }
  catch(...)
    {
    return make_error_code(ide_error_e::internal_error);
    }
  }

auto make_auth_token() noexcept -> expected_ec<std::string>
  {
  std::array<unsigned char, 16> bytes{};
  if(::getentropy(bytes.data(), bytes.size()) != 0)
    return unexpected_ec{std::error_code{errno, std::generic_category()}};
  return catch_to_expected(
    [&bytes]
    {
      std::string token;
      token.reserve(bytes.size() * 2);
      for(auto byte: bytes)
        std::format_to(std::back_inserter(token), "{:02x}", byte);
      return token;
    }
  );
  }

auto lock_dir(std::string_view claude_config_dir, std::filesystem::path const & home) noexcept
  -> expected_ec<std::filesystem::path>
  {
  return catch_to_expected(
    [&]
    {
      if(not claude_config_dir.empty())
        return std::filesystem::path{claude_config_dir} / "ide";
      return home / ".claude" / "ide";
    }
  );
  }

auto lock_file_json(
  std::int64_t pid, std::span<std::string const> workspace_folders, std::string_view auth_token
) noexcept -> expected_ec<std::string>
  {
  return catch_to_expected(
    [&]
    {
      return to_json(
        lock_file_t{.pid = pid, .workspace_folders = workspace_folders, .ide_name = ide_name, .auth_token = auth_token}
      );
    }
  );
  }

auto claude_launch_command(std::uint16_t port, std::string_view claude_command) noexcept -> expected_ec<std::string>
  {
  // env works the same in sh, bash, zsh and fish
  return catch_to_expected(
    [&] { return std::format("env CLAUDE_CODE_SSE_PORT={} ENABLE_IDE_INTEGRATION=true {}", port, claude_command); }
  );
  }

auto file_uri(std::string_view path) noexcept -> expected_ec<std::string>
  {
  return catch_to_expected([path] { return make_file_uri(path); });
  }

auto uri_to_path(std::string_view uri) noexcept -> expected_ec<std::string>
  {
  return catch_to_expected([uri] { return path_from_uri(uri); });
  }

auto severity_name(severity_e severity) noexcept -> std::string_view
  {
  switch(severity)
    {
    case severity_e::error:   return "Error";
    case severity_e::warning: return "Warning";
    case severity_e::info:    return "Info";
    case severity_e::hint:    return "Hint";
    }
  return "Info";
  }

namespace
  {
  auto dispatch(std::string const & id, request_t const & request, ide_tools_t & tools, send_t const & send) -> void
    {
    auto const & method{request.method};
    if(method == "initialize")
      {
      auto const params{from_json<initialize_params_t>(request.params.str).value_or(initialize_params_t{})};
      send(response(id, initialize_result_t{.protocol_version = params.protocol_version}));
      }
    else if(method == "tools/list")
      send(response(id, tools_list_t{.tools = tools_info}));
    else if(method == "ping")
      send(response(id, empty_t{}));
    else if(method == "tools/call")
      {
      if(auto const params{from_json<call_params_t>(request.params.str)}; params)
        call_tool(id, *params, tools, send);
      else
        send(error_response(id, rpc_error_e::invalid_params, "Invalid params"));
      }
    else
      send(error_response(id, rpc_error_e::method_not_found, std::format("Method not found: {}", method)));
    }
  }  // namespace

auto handle_message(std::string_view message, ide_tools_t & tools, send_t const & send) noexcept
  -> expected_ec<std::string>
  {
  return catch_to_expected(
    [&] -> std::string
    {
      auto const request{from_json<request_t>(message)};
      if(not request)
        {
        send(error_response("null", rpc_error_e::parse_error, "Parse error"));
        return {};
        }
      // responses and notifications (ide_connected, notifications/initialized) need no answer
      if(not request->id)
        return request->method;
      if(request->method.empty())
        return {};

      auto const & id{request->id->str};
      try
        {
        dispatch(id, *request, tools, send);
        }
      catch(...)
        {
        // early catch: a failure in the STL, glaze or an ide_tools_t implementation ends as a JSON-RPC error
        send(error_response(id, rpc_error_e::internal_error, current_exception_message()));
        }
      return {};
    }
  );
  }

auto selection_changed(std::string_view path, std::string_view text, position_t start, position_t end) noexcept
  -> expected_ec<std::string>
  {
  return catch_to_expected(
    [&]
    {
      bool const is_empty{start.line == end.line and start.character == end.character};
      return notification(
        "selection_changed",
        selection_params_t{
          .text = text,
          .file_path = path,
          .file_url = make_file_uri(path),
          .selection = {.start = start, .end = end, .is_empty = is_empty}
        }
      );
    }
  );
  }

auto at_mentioned(std::string_view path, std::optional<int> line_start, std::optional<int> line_end) noexcept
  -> expected_ec<std::string>
  {
  return catch_to_expected(
    [&]
    {
      return notification(
        "at_mentioned", at_mentioned_params_t{.file_path = path, .line_start = line_start, .line_end = line_end}
      );
    }
  );
  }
  }  // namespace ide_protocol

template class simple_enum::generic_error_category<ide_protocol::ide_error_e>;
