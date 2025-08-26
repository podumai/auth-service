#include <auth_service/handlers/v1/remove_user_handler.hpp>
#include <userver/storages/postgres/component.hpp>
#include <auth_service/sql_queries.hpp>

namespace auth_service::api::v1
{

RemoveUserHandler::RemoveUserHandler(
  const userver::components::ComponentConfig& config,  //
  const userver::components::ComponentContext& context
)
  : HttpHandlerJsonBase{config, context}  //
  , cluster_{context.FindComponent<userver::components::Postgres>("auth-db").GetCluster()}
{ }

auto RemoveUserHandler::HandleRequestJsonThrow(
  const userver::server::http::HttpRequest& request,  //
  [[maybe_unused]] const userver::formats::json::Value& json,
  userver::server::request::RequestContext& context
) const -> userver::formats::json::Value
{
  std::string uid{request.GetArg("uid")};

  auto query_result{cluster_->Execute(
    userver::storages::postgres::ClusterHostType::kMaster, auth_service::sql::kDeleteUser, std::move(uid)
  )};

  request.SetResponseStatus(userver::server::http::HttpStatus::kNoContent);
  return {};
}

}  // namespace auth_service::api::v1