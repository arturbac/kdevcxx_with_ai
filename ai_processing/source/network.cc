#ifndef Q_MOC_RUN
#include "network.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <fmt/core.h>
//
// https://api.openai.com/v1/engines/gpt-3.5-turbo/completions
// curl -X POST
//      -H "Authorization: Bearer YOUR_API_KEY"
//      -H "Content-Type: application/json"
//      -d '{"prompt": "Hello, world!", "max_tokens": 5}'
//      https://api.openai.com/v1/engines/gpt-3.5-turbo/completions
//

namespace beast = boost::beast;  // from <boost/beast.hpp>
namespace http = beast::http;    // from <boost/beast/http.hpp>
namespace net = boost::asio;     // from <boost/asio.hpp>
using tcp = net::ip::tcp;        // from <boost/asio/ip/tcp.hpp>
namespace ssl = net::ssl;        // from <boost/asio/ssl.hpp>

auto send_text_to_gpt(
  std::string_view host,
  std::string_view port,
  std::string_view target,
  std::string_view api_key,
  std::string_view text,
  int version
) -> expected<std::string, send_text_to_gpt_error>
  {
  try
    {
    // The io_context is required for all I/O
    net::io_context ioc;

    // The SSL context is required, and we load the root certificates.
    ssl::context ctx(ssl::context::tls_client);
    ctx.set_default_verify_paths();

    // These objects perform our I/O
    tcp::resolver resolver(ioc);
    ssl::stream<tcp::socket> stream(ioc, ctx);

    // Set SNI Hostname (many hosts need this to handshake successfully)
    if(!SSL_set_tlsext_host_name(stream.native_handle(), host.data()))
      throw beast::system_error(
        beast::error_code(static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()),
        "Failed to set SNI Hostname"
      );

    // Look up the domain name
    auto const results = resolver.resolve(host, port);

    // Make the connection on the IP address we get from a lookup
    net::connect(stream.next_layer(), results.begin(), results.end());

    // Perform the SSL handshake
    stream.handshake(ssl::stream_base::client);

    fmt::print(
      "----------------------------------------------------------------------------------------\n"
      "Send:\n{}\n",
      text,
      "----------------------------------------------------------------------------------------\n"

    );

    // Set up the request. We use a string for the body.
    http::request<http::string_body> req{http::verb::post, target, version};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.set(http::field::content_type, "application/json");
    req.set(http::field::authorization, std::string("Bearer ").append(api_key));
    req.body() = text;
    req.prepare_payload();

    // Send the HTTP request to the remote host
    http::write(stream, req);

    // This buffer is used for reading and must be persisted
    beast::flat_buffer buffer;

    // Declare a container to hold the response
    http::response<http::dynamic_body> res;

    // Receive the HTTP response
    http::read(stream, buffer, res);

    // Write the message to standard out
    std::cout << res << std::endl;

    // Gracefully close the stream
    beast::error_code ec;
    stream.shutdown(ec);
    if(ec == net::error::eof)
      {
      // Rationale:
      // http://stackoverflow.com/questions/25587403/boost-asio-ssl-socket-shutdown
      ec = {};
      }
    if(ec)
      {
      fmt::print("Error: {}\n", ec.what());
      // return unexpected{send_text_to_gpt_error::network_error};
      }

    // If we get here then the connection is closed gracefully
    return beast::buffers_to_string(res.body().data());
    }
  catch(std::exception const & e)
    {
    fmt::print("Error: {}\n", e.what());
    return unexpected{send_text_to_gpt_error::other_exception};
    }
  }

auto parse(std::string_view url) -> url_components
  {
  url_components components;

  auto scheme_end = url.find("://");
  if(scheme_end != std::string_view::npos)
    {
    components.scheme = url.substr(0, scheme_end);
    url.remove_prefix(scheme_end + 3);  // Skip "://"
    }
  else
    {
    throw std::runtime_error("URL Scheme not found");
    }

  auto host_end = url.find('/');
  if(host_end != std::string_view::npos)
    {
    components.target = url.substr(host_end);
    }
  else
    {
    components.target = "/";  // Default target
    host_end = url.length();
    }

  auto port_start = url.find(":", 0, host_end);
  if(port_start != std::string_view::npos)
    {
    components.host = url.substr(0, port_start);
    components.port = url.substr(port_start + 1, host_end - port_start - 1);
    }
  else
    {
    components.host = url.substr(0, host_end);
    // Assign default port based on scheme
    components.port = components.scheme == "https" ? "443" : "80";
    }

  return components;
  }

#endif
