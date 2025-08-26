#pragma once

#include <string_view>
#include <string>
#include <userver/http/common_headers.hpp>
#include <userver/components/component.hpp>
#include <userver/components/fs_cache.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

namespace auth_service::http
{

class ResourceHandler final : public userver::server::handlers::HttpHandlerBase
{
 public:
  static constexpr std::string_view kName{"handler-resources"};

 public:
  ResourceHandler(
    const userver::components::ComponentConfig& config,  //
    const userver::components::ComponentContext& context
  );

  [[nodiscard]]
  auto HandleRequestThrow(
    const userver::server::http::HttpRequest& request,  //
    userver::server::request::RequestContext& context
  ) const -> std::string override;

 private:
  const userver::fs::FsCacheClient& fs_client_;
};

}  // namespace auth_service::api::v1