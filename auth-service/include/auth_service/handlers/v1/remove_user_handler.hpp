#pragma once

#include <string_view>
#include <userver/components/component.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>
#include <userver/storages/postgres/cluster.hpp>

namespace auth_service::api::v1
{

class RemoveUserHandler final : public userver::server::handlers::HttpHandlerJsonBase
{
 public:
  static constexpr std::string_view kName{"handler-remove-user"};

 public:
  RemoveUserHandler(
    const userver::components::ComponentConfig& config,  //
    const userver::components::ComponentContext& context
  );

  [[nodiscard]] auto HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,  //
    const userver::formats::json::Value& json,
    userver::server::request::RequestContext& context
  ) const -> userver::formats::json::Value override;

 private:
  userver::storages::postgres::ClusterPtr cluster_;
};

}  // namespace auth_service::api::v1