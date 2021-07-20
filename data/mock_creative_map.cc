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

#include "data/mock_creative_map.h"

#include <string>

#include "gmock/gmock.h"
#include "google/cloud/spanner/bytes.h"
#include "google/cloud/spanner/client.h"
#include "google/cloud/spanner/mocks/mock_spanner_connection.h"
#include "google/cloud/spanner/timestamp.h"
#include "google/protobuf/text_format.h"
#include "proto/creative_data.pb.h"

namespace spanner = ::google::cloud::spanner;

using ::testing::_;
using ::testing::InSequence;
using ::testing::Return;
namespace spanner = ::google::cloud::spanner;

namespace trusted_server {

std::shared_ptr<MockCreativeMap> MockCreativeMap::CreateMockMap() {
  std::shared_ptr<MockCreativeMap> creative_map =
      std::shared_ptr<MockCreativeMap>(new MockCreativeMap());
  creative_map->InitializeSpannerClient();
  creative_map->PopulateMap();
  return creative_map;
}

void MockCreativeMap::InitializeSpannerClient() {
  // Create a mock object to stream the results of a ExecuteQuery.
  source_ = std::unique_ptr<google::cloud::spanner_mocks::MockResultSetSource>(
      new google::cloud::spanner_mocks::MockResultSetSource);
  // Create a mock for `spanner::Connection`:
  conn_ = std::make_shared<google::cloud::spanner_mocks::MockConnection>();

  // Setup the return type of the ExecuteQuery results:
  auto constexpr kText = R"pb(
    row_type: {
      fields: {
        name: "CreativeId",
        type: { code: STRING }
      }
      fields: {
        name: "CreativeData",
        type: { code: BYTES }
      }
      fields: {
        name: "LastUpdatedTime",
        type: { code: TIMESTAMP }
      }
    }
    transaction: {
      id: "123"
      read_timestamp: { seconds: 0 }
    })pb";
  google::spanner::v1::ResultSetMetadata metadata;
  google::protobuf::TextFormat::ParseFromString(kText, &metadata);
  EXPECT_CALL(*source_, Metadata()).WillRepeatedly(Return(metadata));

  trusted_server::CreativeMetadata c1;
  c1.set_is_servible(false);
  trusted_server::CreativeMetadata c2;
  c2.set_is_servible(true);

  client_ = std::unique_ptr<spanner::Client>(new spanner::Client(conn_));

  std::vector<std::pair<std::string, spanner::Bytes>> key_values(
      {{"google.com/ad1", spanner::Bytes(c1.SerializeAsString())},
       {"google.com/ad2", spanner::Bytes(c2.SerializeAsString())}});
  EXPECT_CALL(*conn_, ExecuteQuery(_))
      .WillOnce(
          [this](spanner::Connection::SqlParams const&) -> spanner::RowStream {
            return spanner::RowStream(std::move(source_));
          });

  InSequence seq;
  for (const auto& key_val : key_values) {
    EXPECT_CALL(*source_, NextRow())
        .WillOnce(Return(spanner::MakeTestRow(
            {{"CreativeId", spanner::Value(key_val.first)},
             {"CreativdeData", spanner::Value(key_val.second)}})));
  }

  EXPECT_CALL(*source_, NextRow()).WillOnce(Return(spanner::Row()));
}

}  // namespace trusted_server