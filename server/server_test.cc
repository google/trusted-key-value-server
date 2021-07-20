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

#include <netinet/in.h>
#include <sys/socket.h>

#include "absl/cleanup/cleanup.h"
#include "absl/random/random.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/escaping.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "boost/asio/connect.hpp"
#include "boost/asio/deadline_timer.hpp"
#include "boost/asio/io_service.hpp"
#include "boost/asio/ip/tcp.hpp"
#include "boost/beast/core.hpp"
#include "boost/beast/http.hpp"
#include "boost/beast/version.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "glog/logging.h"
#include "gmock/gmock.h"
#include "google/protobuf/util/json_util.h"
#include "gtest/gtest.h"
#include "proto/creative_data.pb.h"
#include "proto/response.pb.h"
#include "subprocess.hpp"

using ::boost::asio::ip::tcp;
namespace http = ::boost::beast::http;
namespace asio = ::boost::asio;

const char kCreaiveJson[] = "{\"key\":\"%s\",\"creativeData\":\"%s\"}";

class TrustedServer : public ::testing::Environment {
 public:
  void SetUp() override {
    std::string test_workspace_dir =
        absl::StrCat(std::string(std::getenv("TEST_SRCDIR")), "/",
                     std::string(std::getenv("TEST_WORKSPACE")));
    std::string server_binary =
        absl::StrCat(test_workspace_dir, "/server/server");
    server_process_ =
        subprocess::RunBuilder(
            {server_binary, "--mock_spanner=true", "--address=0.0.0.0",
             absl::StrCat("--port=", FindUnusedPort().value())})
            .popen();
    ABSL_ASSERT(WaitUntilServerIsReady());
  }
  void TearDown() override { server_process_.kill(); }
  std::string Address() const { return address_; }
  int Port() const { return port_; }

 private:
  // Waits until server under test is ready to accept connections.
  // Returns true if the server is accepting connections.
  bool WaitUntilServerIsReady() const {
    bool result = false;
    int retries = 10;
    do {
      try {
        asio::io_service svc;
        tcp::socket s(svc);
        asio::deadline_timer timer(svc, boost::posix_time::milliseconds(200));

        timer.async_wait([&](boost::beast::error_code) { s.cancel(); });
        s.async_connect({{}, port_},
                        [&](boost::beast::error_code ec) { result = !ec; });

        svc.run();
      } catch (...) {
        LOG(ERROR) << "Error connecting to subprocess server.";
        continue;
      }
    } while (--retries > 0);

    return result;
  }
  // Finds an unused local TCP port.
  absl::StatusOr<int> FindUnusedPort() {
    auto fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    absl::Cleanup fd_closer = [&fd] { close(fd); };
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = 0;  // port 0 will automatically find a free port.
    addr.sin_addr.s_addr = 0;
    if (bind(fd, (struct sockaddr*)&addr, sizeof(sockaddr_in)) == -1) {
      return false;
    }
    if (listen(fd, 1) == -1) {
      return false;
    }
    // getsockname will reserve a free port for the address.
    socklen_t addrlen = sizeof(addr);
    getsockname(fd, (struct sockaddr*)&addr, &addrlen);
    port_ = ntohs(addr.sin_port);
    return port_;
  }

  short unsigned int port_;
  std::string address_;
  subprocess::Popen server_process_;
};

// Creates a single environment per T and register as a GlobalTestEnvironment.
template <typename T>
T* GetEnv() {
  static T* const instance = [] {
    T* instance = new T;
    ::testing::AddGlobalTestEnvironment(instance);
    return instance;
  }();
  return instance;
}

TrustedServer* const trusted_server_env = GetEnv<TrustedServer>();
class ServerTest : public ::testing::Test {
 protected:
  http::response<http::string_body> SendRequest(std::string target) {
    // The io_context is required for all I/O
    asio::io_context ioc;

    // These objects perform our I/O
    tcp::resolver resolver(ioc);
    boost::beast::tcp_stream stream(ioc);

    // Look up the domain name
    auto const results =
        resolver.resolve(GetEnv<TrustedServer>()->Address(),
                         absl::StrCat(GetEnv<TrustedServer>()->Port()));

    // Make the connection on the IP address we get from a lookup
    stream.connect(results);

    // Set up an HTTP GET request message
    http::request<http::string_body> req{http::verb::get, target,
                                         /*version=*/11};
    req.set(http::field::host, GetEnv<TrustedServer>()->Address());
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    http::write(stream, req);

    boost::beast::flat_buffer buffer;

    // Declare a container to hold the response
    http::response<http::string_body> res;

    // Receive the HTTP response
    http::read(stream, buffer, res);

    boost::beast::error_code ec;
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);
    return res;
  }
};

TEST_F(ServerTest, BasicTest) {
  http::response<http::string_body> response =
      SendRequest("?keys=google.com/ad1");
  EXPECT_EQ(response.result_int(), 200);

  trusted_server::CreativeMetadata c1;
  c1.set_is_servible(false);

  EXPECT_EQ(
      response.body(),
      absl::StrCat("{\"creatives\":[",
                   absl::StrFormat(kCreaiveJson, "google.com/ad1",
                                   absl::Base64Escape(c1.SerializeAsString())),
                   "]}"));
}

TEST_F(ServerTest, MultipleKeys) {
  http::response<http::string_body> response = SendRequest(
      "?keys=google.com/ad1,google.com/ad2,google.com/"
      "nonexistentad&randparam=700");
  EXPECT_EQ(response.result_int(), 200);
  trusted_server::CreativeMetadata c1;
  c1.set_is_servible(false);
  trusted_server::CreativeMetadata c2;
  c2.set_is_servible(true);

  EXPECT_THAT(response.body(),
              ::testing::HasSubstr(
                  absl::StrFormat(kCreaiveJson, "google.com/ad1",
                                  absl::Base64Escape(c1.SerializeAsString()))));
  EXPECT_THAT(response.body(),
              ::testing::HasSubstr(
                  absl::StrFormat(kCreaiveJson, "google.com/ad2",
                                  absl::Base64Escape(c2.SerializeAsString()))));

  // No data set for key not found.
  EXPECT_THAT(response.body(),
              ::testing::HasSubstr("key\":\"google.com/nonexistentad\"}"));
}

TEST_F(ServerTest, NoQuery) {
  http::response<http::string_body> response = SendRequest("/keys=123");
  EXPECT_EQ(response.result_int(), 400);
}

TEST_F(ServerTest, NoKeysParam) {
  http::response<http::string_body> response = SendRequest("?randparam=123");
  EXPECT_EQ(response.result_int(), 400);
}

TEST_F(ServerTest, EmptyRequest) {
  http::response<http::string_body> response = SendRequest("");
  EXPECT_EQ(response.result_int(), 400);
}

TEST_F(ServerTest, NoKeyValues) {
  http::response<http::string_body> response =
      SendRequest("?keys=&randparam=123");
  EXPECT_EQ(response.result_int(), 400);
}
