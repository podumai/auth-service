#include <auth_service/error.hpp>
#include <auth_service/handlers/v1/register_user_handler.hpp>
#include <auth_service/sql_queries.hpp>
#include <userver/crypto/crypto.hpp>
#include <userver/http/common_headers.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/io/bytea.hpp>
#include <userver/utils/boost_uuid4.hpp>
#include <userver/utils/datetime.hpp>

namespace auth_service::api::v1
{

RegisterUserHandler::RegisterUserHandler(
  const userver::components::ComponentConfig& config,  //
  const userver::components::ComponentContext& context
)
  : HttpHandlerJsonBase{config, context}  //
  , cluster_{context.FindComponent<userver::components::Postgres>("auth-db").GetCluster()}
{ }

auto RegisterUserHandler::HandleRequestJsonThrow(
  const userver::server::http::HttpRequest& request,  //
  const userver::formats::json::Value& json,
  [[maybe_unused]] userver::server::request::RequestContext& context
) const -> userver::formats::json::Value
{
  try
  {
    std::string username{json["username"].As<std::string>()};
    std::string password{json["password"].As<std::string>()};
    std::string email{json["email"].As<std::string>()};

    auto uuid{userver::utils::generators::GenerateBoostUuid()};
    auto register_date{userver::storages::postgres::TimePointTz{userver::utils::datetime::Now()}};
    std::string password_hash{
      userver::crypto::hash::Blake2b128(password, userver::crypto::hash::OutputEncoding::kBinary)
    };

    auto query_result{cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      auth_service::sql::kInsertUser,
      uuid,
      std::move(username),
      userver::storages::postgres::Bytea(password_hash),
      std::move(email),
      register_date
    )};

    auto& response{request.GetHttpResponse()};
    response.SetStatus(userver::server::http::HttpStatus::kCreated);
    response.SetHeader(userver::http::headers::kLocation, "/users/{uid}");
    return {};
  }
  catch (const userver::formats::json::MemberMissingException& member_error)
  {
    request.SetResponseStatus(userver::http::StatusCode::kBadRequest);
    return error::MakeResponseErrorMessage(
      userver::http::StatusCode::kBadRequest, member_error.GetMessage()
    );
  }
  catch (const userver::storages::postgres::IntegrityConstraintViolation& contraint_error)
  {
    request.SetResponseStatus(userver::http::StatusCode::kConflict);
    return error::MakeResponseErrorMessage(
      userver::http::StatusCode::kConflict, contraint_error.GetServerMessage().GetDetail()
    );
  }
}

}  // namespace auth_service::api::v1