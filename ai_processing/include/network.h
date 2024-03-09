#pragma once

#ifdef Q_MOC_RUN
#error "Dont process this with moc, use #ifndef Q_MOC_RUN"
#endif

#include <string>
#include "expectd.h"

struct url_components
  {
  std::string_view scheme;
  std::string_view host;
  std::string_view port;
  std::string_view target;
  };


auto parse(std::string_view url) -> url_components;

enum struct send_text_to_gpt_error
  {
  network_error,
  other_exception
  };
[[nodiscard]]
auto send_text_to_gpt(
  std::string_view host,
  std::string_view port,
  std::string_view target,
  std::string_view api_key,
  std::string_view text,
  int version
) -> expected<std::string, send_text_to_gpt_error>;

