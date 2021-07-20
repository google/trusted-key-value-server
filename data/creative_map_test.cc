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
#include "gmock/gmock.h"
#include "google/cloud/spanner/bytes.h"
#include "google/cloud/spanner/client.h"
#include "google/cloud/spanner/mocks/mock_spanner_connection.h"
#include "google/cloud/spanner/timestamp.h"
#include "google/protobuf/text_format.h"
#include "gtest/gtest.h"
#include "proto/creative_data.pb.h"

namespace trusted_server {

namespace {

using ::testing::_;
using ::testing::InSequence;
using ::testing::Return;
namespace spanner = ::google::cloud::spanner;

class CreativeMapTest : public testing::Test {
 protected:
  CreativeMapTest() : creative_map_(MockCreativeMap::CreateMockMap()){};

  void TearDown() {
    EXPECT_TRUE(testing::Mock::VerifyAndClearExpectations(creative_map_.get()));
  }

  std::shared_ptr<MockCreativeMap> creative_map_;
};

TEST_F(CreativeMapTest, BasicLookup) {
  std::vector<std::string> keys({"google.com/ad1"});
  trusted_server::Response response = creative_map_->Lookup(keys);
  EXPECT_EQ(response.creatives().size(), 1);
  trusted_server::CreativeMetadata metadata;
  EXPECT_EQ("google.com/ad1", response.creatives().at(0).key());
  std::string data_string = response.creatives().at(0).creative_data();
  ASSERT_TRUE(metadata.ParseFromString(data_string));

  EXPECT_TRUE(metadata.has_is_servible());
  EXPECT_FALSE(metadata.is_servible());
}

TEST_F(CreativeMapTest, KeyNotFound) {
  std::vector<std::string> keys({"google.com/wrongurl"});
  trusted_server::Response response = creative_map_->Lookup(keys);
  EXPECT_EQ(response.creatives().size(), 1);
  trusted_server::CreativeMetadata metadata;
  EXPECT_EQ("google.com/wrongurl", response.creatives().at(0).key());
  EXPECT_FALSE(response.creatives().at(0).has_creative_data());
}

TEST_F(CreativeMapTest, MultipleKeyLookup) {
  std::vector<std::string> keys(
      {"google.com/ad1", "google.com/ad2", "wrong_key"});
  trusted_server::Response response = creative_map_->Lookup(keys);
  EXPECT_EQ(response.creatives().size(), 3);
  std::unordered_map<std::string, trusted_server::CreativeMetadata>
      response_map;
  for (const auto& creative : response.creatives()) {
    std::string result = creative.creative_data();
    trusted_server::CreativeMetadata metadata;
    ASSERT_TRUE(metadata.ParseFromString(result));
    response_map[creative.key()] = metadata;
  }
  EXPECT_FALSE(response_map["wrong_key"].has_is_servible());
  EXPECT_TRUE(response_map["google.com/ad1"].has_is_servible());
  EXPECT_FALSE(response_map["google.com/ad1"].is_servible());
  EXPECT_TRUE(response_map["google.com/ad2"].is_servible());
}

}  // namespace

}  // namespace trusted_server