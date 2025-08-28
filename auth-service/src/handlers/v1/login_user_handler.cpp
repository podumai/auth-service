#include <algorithm>
#include <auth_service/error.hpp>
#include <auth_service/handlers/v1/login_user_handler.hpp>
#include <auth_service/sql_queries.hpp>
#include <tuple>
#include <userver/crypto/crypto.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/io/bytea.hpp>
#include <userver/utils/boost_uuid4.hpp>

namespace
{

[[nodiscard]] auto CreateJwtToken(
  std::string&& uid,  //
  std::string&& role
) -> std::string
{
  constexpr std::string_view header{R"({
  "alg": "HS256",
  "typ": "JWT"
})"};
  std::string header_encoded{userver::crypto::base64::Base64UrlEncode(header, userver::crypto::base64::Pad::kWithout)};

  std::time_t current_time{userver::utils::datetime::Timestamp()};
  constexpr std::time_t kTokenTtl{60 * 15};
  std::time_t expiration_time{current_time + kTokenTtl};

  userver::formats::json::ValueBuilder payload_builder;
  payload_builder["sub"] = std::move(uid);
  payload_builder["role"] = std::move(role);
  payload_builder["exp"] = expiration_time;
  payload_builder["iat"] = current_time;
  std::string payload{userver::formats::json::ToString(payload_builder.ExtractValue())};
  std::string payload_encoded{
    userver::crypto::base64::Base64UrlEncode(payload, userver::crypto::base64::Pad::kWithout)
  };

  std::string signature{header_encoded + '.' + payload_encoded};
  constexpr std::string_view kSecretKey{"9G2tBHs63fvN0PdVeJ7q"};
  std::string signature_signed{
    userver::crypto::hash::HmacSha256(kSecretKey, signature, userver::crypto::hash::OutputEncoding::kBase64)
  };
  auto range_start{signature_signed.begin()};
  auto new_range_end{std::remove(range_start, signature_signed.end(), '=')};
  signature_signed.resize(std::distance(range_start, new_range_end));
  std::for_each(
    range_start,
    new_range_end,
    [](char& symbol) -> void
    {
      if (symbol == '+')
      {
        symbol = '-';
      }
      else if (symbol == '/')
      {
        symbol = '_';
      }
    }
  );

  std::string token{signature + '.' + signature_signed};
  return token;
}

}  // namespace

namespace auth_service::api::v1
{

LoginUserHandler::LoginUserHandler(
  const userver::components::ComponentConfig& config,  //
  const userver::components::ComponentContext& context
)
  : HttpHandlerJsonBase{config, context}  //
  , cluster_{context.FindComponent<userver::components::Postgres>("auth-db").GetCluster()}
{ }

auto LoginUserHandler::HandleRequestJsonThrow(
  const userver::server::http::HttpRequest& request,  //
  [[maybe_unused]] const userver::formats::json::Value& json,
  [[maybe_unused]] userver::server::request::RequestContext& context
) const -> userver::formats::json::Value
{
  try
  {
    if (request.HasArg("username") == false || request.HasArg("password") == false)
    {
      request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
      return {};
    }
    std::string username{request.GetArg("username")};
    std::string password{request.GetArg("password")};
    std::string password_hash{
      userver::crypto::hash::Blake2b128(password, userver::crypto::hash::OutputEncoding::kBinary)
    };

    auto query_result{cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kSlave,
      auth_service::sql::kSelectUser,
      username,
      userver::storages::postgres::Bytea(password_hash)
    )};

    if (query_result.RowsAffected() != 1)
    {
      request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
      return {};
    }

    using ResultType = typename std::tuple<std::string, std::string>;  // userver::storages::postgres::TimePointTz>;
    auto result_set{query_result.AsSingleRow<ResultType>(userver::storages::postgres::kRowTag)};
    auto& response{request.GetHttpResponse()};
    response.SetStatus(userver::server::http::HttpStatus::kOk);
    std::string token{CreateJwtToken(std::move(std::get<0>(result_set)), std::move(std::get<1>(result_set)))};
    response.SetHeader(userver::http::headers::kSetCookie, token);
    return {};
  }
  catch (const userver::formats::json::MemberMissingException& member_error)
  {
    request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
    userver::formats::json::ValueBuilder builder;
    builder["code"] = std::to_underlying(userver::server::http::HttpStatus::kBadRequest);
    builder["message"] = member_error.GetMessage();
    return builder.ExtractValue();
  }
}

}  // namespace auth_service::api::v1