#pragma once

#include <string_view>
#include <userver/formats/json.hpp>
#include <userver/http/status_code.hpp>
#include <userver/http/common_headers.hpp>

namespace auth_service::error
{

[[nodiscard]] extern auto MakeResponseErrorMessage(
  userver::http::StatusCode status_code,  //
  std::string_view message
) -> userver::formats::json::Value;

}