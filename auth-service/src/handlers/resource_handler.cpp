#include <auth_service/handlers/http/resource_handler.hpp>

namespace
{

auto GetContentType(
  std::string_view extension
) noexcept -> std::string_view
{
  if (extension == ".js")
  {
    return "application/javascript";
  }
  else if (extension == ".css")
  {
    return "text/css";
  }
  else if (extension == ".html")
  {
    return "text/html; charset=UTF-8";
  }
  else
  {
    return "application/octet-stream";
  }
}

}

namespace auth_service::http
{

ResourceHandler::ResourceHandler(
  const userver::components::ComponentConfig& config,  //
  const userver::components::ComponentContext& context
)
  : HttpHandlerBase{config, context}  //
  , fs_client_{context.FindComponent<userver::components::FsCache>("fs-cache-main").GetClient()}
{ }

auto ResourceHandler::HandleRequestThrow(
  const userver::server::http::HttpRequest& request,  //
  [[maybe_unused]] userver::server::request::RequestContext& context
) const -> std::string
{
  auto subpath{request.GetPathArg("subpath")};
  auto file_ptr{fs_client_.TryGetFile("/" + subpath)};
  auto& response{request.GetHttpResponse()};
  if (file_ptr)
  {
    response.SetContentType(GetContentType(file_ptr->extension));
    response.SetHeader(userver::http::headers::kExpires, "600");
    return file_ptr->data;
  }
  response.SetStatus(userver::server::http::HttpStatus::kNotFound);
  return {};
}

}  // namespace auth_service::http