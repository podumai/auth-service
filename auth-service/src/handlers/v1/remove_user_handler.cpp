#include <auth_service/handlers/v1/remove_user_handler.hpp>
#include <auth_service/sql_queries.hpp>
#include <userver/storages/postgres/component.hpp>
#include <auth_service/error.hpp>

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
  [[maybe_unused]] userver::server::request::RequestContext& context
) const -> userver::formats::json::Value
{
  if (request.HasArg("uid") == false)
  {
    request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
    return error::MakeResponseErrorMessage(
      userver::server::http::HttpStatus::kBadRequest, "Error at path 'uid': Field is missing"
    );
  }

  std::string uid{request.GetArg("uid")};

  auto query_result{cluster_->Execute(
    userver::storages::postgres::ClusterHostType::kMaster, auth_service::sql::kDeleteUser, std::move(uid)
  )};

  request.SetResponseStatus(userver::server::http::HttpStatus::kNoContent);
  return {};
}

}  // namespace auth_service::api::v1