#include <auth_service/handlers/v1/register_handler.hpp>
#include <userver/clients/dns/component.hpp>
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
) -> int
{
  auto component_list{userver::components::MinimalServerComponentList()
                        .Append<userver::clients::dns::Component>()
                        .Append<userver::server::handlers::Ping>()
                        .Append<userver::components::Postgres>("auth-db")
                        .Append<auth_service::api::v1::RegisterHandler>()
                        .Append<userver::components::FsCache>("fs-cache-main")
                        .Append<userver::server::handlers::HttpHandlerStatic>()
                        .Append<userver::components::TestsuiteSupport>()};
  return userver::utils::DaemonMain(argc, argv, component_list);
}