// Copyright 2021 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>
#include <unordered_set>

#include "absl/container/flat_hash_map.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "boost/asio/ip/tcp.hpp"
#include "boost/asio/strand.hpp"
#include "boost/beast/core.hpp"
#include "boost/beast/http.hpp"
#include "boost/beast/version.hpp"
#include "data/creative_map.h"
#include "data/mock_creative_map.h"
#include "glog/logging.h"
#include "google/cloud/spanner/client.h"
#include "google/protobuf/util/json_util.h"
#include "proto/response.pb.h"

ABSL_FLAG(bool, mock_spanner, false,
          "If enabled uses a mock spanner client with test data.");

ABSL_FLAG(std::string, address, "0.0.0.0", "Server address to bind to.");

// Default Cloud Run port value is 8080.
ABSL_FLAG(std::uint16_t, port, 8080, "Port the address is listening on.");

ABSL_FLAG(std::string, key_param, "keys",
          "Parameter name to use for the key value lookup.");

using ::boost::asio::ip::tcp;
using ::google::protobuf::util::MessageToJsonString;
using ::trusted_server::CreativeMap;
using ::trusted_server::MockCreativeMap;
namespace http = ::boost::beast::http;

absl::StatusOr<absl::flat_hash_map<std::string, std::string>> QueryParamsToMap(
    const http::request<http::string_body>& request) {
  absl::flat_hash_map<std::string, std::string> params;
  if (request.target().length() < 2) {
    return absl::Status(absl::StatusCode::kInvalidArgument,
                        "Target does not contain valid query string.");
  }
  auto query_pos = request.target().to_string().find("?");
  if (query_pos == std::string::npos) {
    return absl::Status(absl::StatusCode::kInvalidArgument,
                        "Target does not contain query delimiter.");
  }
  // =&; are reservered url query characters.
  params = absl::StrSplit(request.target().to_string().substr(query_pos + 1),
                          absl::ByAnyChar("=&;"));
  return params;
}

void SendErrorResponse(tcp::socket& socket, uint http_version,
                       http::status status) {
  boost::beast::error_code error_code;
  http::response<http::string_body> response{status, http_version};
  http::write(socket, response, error_code);
  socket.shutdown(tcp::socket::shutdown_send, error_code);
}

void HandleSession(tcp::socket socket,
                   std::shared_ptr<CreativeMap> creative_map) {
  boost::beast::error_code error_code;
  boost::beast::flat_buffer buffer;

  http::request<http::string_body> request;
  http::read(socket, buffer, request, error_code);

  if (error_code) {
    SendErrorResponse(socket, request.version(), http::status::bad_request);
    return;
  }

  std::string key_param = absl::GetFlag(FLAGS_key_param);
  auto status_or_params = QueryParamsToMap(request);
  if (!status_or_params.ok() ||
      status_or_params.value().find(key_param) ==
          status_or_params.value().end() ||
      status_or_params.value()[key_param].empty()) {
    SendErrorResponse(socket, request.version(), http::status::bad_request);
    return;
  }
  std::vector<std::string> keys =
      absl::StrSplit(status_or_params.value()[key_param], ",");

  ::trusted_server::Response proto_response = creative_map->Lookup(keys);
  std::string json_response;
  if (!MessageToJsonString(proto_response, &json_response).ok()) {
    SendErrorResponse(socket, request.version(), http::status::bad_request);
    return;
  }

  // Send the response
  http::response<http::string_body> response{http::status::ok,
                                             request.version()};

  response.set(http::field::server, BOOST_BEAST_VERSION_STRING);
  response.set(http::field::content_type, "application/json");
  response.keep_alive(request.keep_alive());
  response.body() = std::move(json_response);
  response.prepare_payload();

  http::write(socket, response, error_code);
  if (error_code) {
    return;
  }

  socket.shutdown(tcp::socket::shutdown_send, error_code);
}

void RunServer() {
  auto address = boost::asio::ip::make_address(absl::GetFlag(FLAGS_address));
  auto port = absl::GetFlag(FLAGS_port);
  std::shared_ptr<CreativeMap> creative_map =
      absl::GetFlag(FLAGS_mock_spanner) ? MockCreativeMap::CreateMockMap()
                                        : CreativeMap::CreateMap();

  boost::asio::io_context ioc{/*concurrency_hint=*/80};
  tcp::acceptor acceptor{ioc, {address, port}};
  for (;;) {
    // Blocks until a new connection is attempted.
    auto socket = acceptor.accept(ioc);
    if (!socket.is_open()) break;
    // Run a thread per-session, transferring ownership of the socket
    std::thread{HandleSession, std::move(socket), std::ref(creative_map)}
        .detach();
  }
}

int main(int argc, char* argv[]) try {
  absl::ParseCommandLine(argc, argv);
  RunServer();
  return 0;
} catch (std::exception const& ex) {
  LOG(ERROR) << "Standard exception caught " << ex.what() << '\n';
  return 1;
}
