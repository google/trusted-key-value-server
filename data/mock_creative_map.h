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

#ifndef MOCK_CREATIVE_MAP_H_
#define MOCK_CREATIVE_MAP_H_

#include <memory>
#include <string>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/synchronization/mutex.h"
#include "data/creative_map.h"
#include "gmock/gmock.h"
#include "google/cloud/spanner/bytes.h"
#include "google/cloud/spanner/client.h"
#include "google/cloud/spanner/mocks/mock_spanner_connection.h"
#include "google/cloud/spanner/timestamp.h"
#include "google/protobuf/text_format.h"
#include "proto/creative_data.pb.h"
#include "proto/response.pb.h"

namespace spanner = ::google::cloud::spanner;

using ::testing::_;
using ::testing::InSequence;
using ::testing::Return;

namespace trusted_server {

// Mock CreativeMap class that mocks the connection to
// spanner for testing.
class MockCreativeMap : public CreativeMap {
 public:
  static std::shared_ptr<MockCreativeMap> CreateMockMap();

 protected:
  void InitializeSpannerClient() override;
  std::shared_ptr<google::cloud::spanner_mocks::MockConnection> conn_;
  std::unique_ptr<google::cloud::spanner_mocks::MockResultSetSource> source_;
};

}  // namespace trusted_server
#endif  // MOCK_CREATIVE_MAP_H_