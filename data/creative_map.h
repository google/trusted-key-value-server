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

#ifndef CREATIVE_MAP_H_
#define CREATIVE_MAP_H_

#include <memory>
#include <string>

#include "absl/base/thread_annotations.h"
#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/synchronization/mutex.h"
#include "google/cloud/spanner/bytes.h"
#include "google/cloud/spanner/client.h"
#include "google/cloud/spanner/timestamp.h"
#include "proto/response.pb.h"

namespace spanner = ::google::cloud::spanner;

namespace trusted_server {
// CreativeMap hold a map of creatives keyed by rendering URLs
// and holds serialized data as values.
class CreativeMap {
 public:
  // CreativeMap shared_ptr factory method initializes the connection
  // to the spanner database and populates the local map. This
  // factory method is supposed to be called once per instance,
  // the returned CreativeMap is intended to be shared across threads.
  static std::shared_ptr<CreativeMap> CreateMap();

  trusted_server::Response Lookup(const std::vector<std::string>& keys) const;

 protected:
  virtual void InitializeSpannerClient();
  virtual void PopulateMap();
  void RefreshMap();

  std::unique_ptr<spanner::Client> client_;
  mutable absl::Mutex mutex_;
  absl::flat_hash_map<std::string, spanner::Bytes> creative_data_
      ABSL_GUARDED_BY(mutex_);

  // Keep a record of most recent read to only query recently
  // modified database entries on refreshes.
  spanner::Timestamp latest_read_;
};
}  // namespace trusted_server
#endif  // SERVER_AD_AUCTIONS_H_