#include <auth_service/handlers/v1/register_user_handler.hpp>
#include <auth_service/handlers/v1/remove_user_handler.hpp>
#include <auth_service/handlers/v1/login_user_handler.hpp>
#include <auth_service/handlers/http/resource_handler.hpp>
#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/components/fs_cache.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/http_handler_static.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>

auto main(
  int argc,  //
  char* argv[]
) -> int {
  auto component_list{userver::components::MinimalServerComponentList()
                        .Append<auth_service::http::ResourceHandler>()
                        .Append<userver::components::HttpClient>()
                        .Append<userver::clients::dns::Component>()
                        .Append<userver::server::handlers::Ping>()
                        .Append<userver::components::Postgres>("auth-db")
                        .Append<auth_service::api::v1::RegisterUserHandler>()
                        .Append<auth_service::api::v1::LoginUserHandler>()
                        .Append<auth_service::api::v1::RemoveUserHandler>()
                        .Append<userver::components::FsCache>("fs-cache-main")
                        .Append<userver::server::handlers::HttpHandlerStatic>()
                        .Append<userver::components::TestsuiteSupport>()};
  return userver::utils::DaemonMain(argc, argv, component_list);
}