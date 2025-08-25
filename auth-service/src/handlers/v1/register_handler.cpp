#include <auth_service/handlers/v1/register_handler.hpp>
#include <userver/crypto/crypto.hpp>
#include <userver/http/common_headers.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/utils/boost_uuid4.hpp>
#include <userver/utils/datetime.hpp>

namespace auth_service::api::v1
{

RegisterHandler::RegisterHandler(
  const userver::components::ComponentConfig& config,  //
  const userver::components::ComponentContext& context
)
  : HttpHandlerJsonBase{config, context}  //
  , cluster_{context.FindComponent<userver::components::Postgres>("auth-db").GetCluster()}
{ }

auto RegisterHandler::HandleRequestJsonThrow(
  const userver::server::http::HttpRequest& request,  //
  [[maybe_unused]] const userver::formats::json::Value& json,
  [[maybe_unused]] userver::server::request::RequestContext& context
) const -> userver::formats::json::Value
{
  const auto& username{request.GetArg("username")};
  const auto& password{request.GetArg("password")};
  const auto& email{request.GetArg("email")};

  auto uuid{userver::utils::generators::GenerateBoostUuid()};
  auto register_date{userver::utils::datetime::UtcTimestring(userver::utils::datetime::Now())};
  std::string password_hash{userver::crypto::hash::Blake2b128(username)};

  auto query_result{cluster_->Execute(
    userver::storages::postgres::ClusterHostType::kMaster,
    "INSERT INTO auth.users (uid, username, passwd, email, register_date) VALUES ($1, $2, $3, $4, $5);",
    uuid,
    username,
    password,
    email,
    register_date
  )};

  auto& response{request.GetHttpResponse()};
  response.SetStatus(userver::server::http::HttpStatus::kCreated);
  response.SetContentType(userver::http::content_type::kApplicationJson);
  userver::formats::json::ValueBuilder builder;
  builder["username"] = username;
  builder["email"] = email;
  builder["register_date"] = register_date;
  return builder.ExtractValue();
}

}  // namespace auth_service::api::v1