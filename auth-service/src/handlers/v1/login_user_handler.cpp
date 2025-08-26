#include <auth_service/handlers/v1/login_user_handler.hpp>
#include <auth_service/sql_queries.hpp>
#include <tuple>
#include <userver/crypto/crypto.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/io/bytea.hpp>

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
  const userver::formats::json::Value& json,
  [[maybe_unused]] userver::server::request::RequestContext& context
) const -> userver::formats::json::Value
{
  std::string username{json["username"].As<std::string>()};
  std::string password{json["password"].As<std::string>()};
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

  using ResultType = typename std::tuple<std::string, userver::storages::postgres::TimePointTz>;
  auto result_set{query_result.AsSingleRow<ResultType>(userver::storages::postgres::kRowTag)};
  request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
  userver::formats::json::ValueBuilder builder;
  builder["uid"] = std::move(std::get<0>(result_set));
  builder["username"] = std::move(username);
  builder["register_date"] = std::get<1>(result_set);
  return builder.ExtractValue();
}

}  // namespace auth_service::api::v1