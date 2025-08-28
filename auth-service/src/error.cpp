#include <auth_service/error.hpp>

namespace auth_service::error
{

auto MakeResponseErrorMessage(
  userver::http::StatusCode status_code,  //
  std::string_view message
) -> userver::formats::json::Value
{
  userver::formats::json::ValueBuilder builder;
  builder["code"] = std::to_underlying(status_code);
  builder["message"] = message;
  return builder.ExtractValue();
}

}  // namespace auth_service::error