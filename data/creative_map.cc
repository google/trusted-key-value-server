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

#include "data/creative_map.h"

#include <string>
#include <thread>

#include "absl/container/flat_hash_map.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "absl/synchronization/mutex.h"
#include "boost/asio/ip/tcp.hpp"
#include "boost/asio/strand.hpp"
#include "boost/beast/core.hpp"
#include "boost/beast/http.hpp"
#include "boost/beast/version.hpp"
#include "glog/logging.h"
#include "google/cloud/spanner/bytes.h"
#include "google/cloud/spanner/client.h"
#include "google/protobuf/util/json_util.h"

namespace spanner = ::google::cloud::spanner;

ABSL_FLAG(std::string, spanner_project_id, "ads-trusted-server-dev",
          "Project ID of Spanner instance to connect to.");

ABSL_FLAG(std::string, spanner_instance_id, "tfgen-spanid-20210518145728389",
          "Instance ID of Spanner instance to connect to.");

ABSL_FLAG(std::string, spanner_database_id, "trusted-server-database",
          "Database ID of Spanner instance to connect to.");

ABSL_FLAG(int, refresh_period_sec, 600,
          "Period in seconds to refresh creative data map.");

namespace trusted_server {

std::shared_ptr<CreativeMap> CreativeMap::CreateMap() {
  std::shared_ptr<CreativeMap> creative_map =
      std::shared_ptr<CreativeMap>(new CreativeMap());
  creative_map->InitializeSpannerClient();
  creative_map->PopulateMap();
  std::thread([creative_map] { creative_map->RefreshMap(); }).detach();
  return creative_map;
}

void CreativeMap::InitializeSpannerClient() {
  client_ = std::unique_ptr<spanner::Client>(
      new spanner::Client(spanner::MakeConnection(
          spanner::Database(absl::GetFlag(FLAGS_spanner_project_id),
                            absl::GetFlag(FLAGS_spanner_instance_id),
                            absl::GetFlag(FLAGS_spanner_database_id)))));
}

void CreativeMap::PopulateMap() {
  auto rows = client_->ExecuteQuery(spanner::SqlStatement(
      "SELECT CreativeId, CreativeData FROM CreativeMetadata"));

  latest_read_ = rows.ReadTimestamp().value();

  for (auto const& row :
       spanner::StreamOf<std::tuple<std::string, spanner::Bytes>>(rows)) {
    if (!row) {
      LOG(ERROR) << "Invalid Spanner response.";
      return;
    }
    creative_data_[std::get<0>(*row)] = std::get<1>(*row);
  }
}

void CreativeMap::RefreshMap() {
  for (;;) {
    absl::SleepFor(absl::Seconds(absl::GetFlag(FLAGS_refresh_period_sec)));
    std::string stmt(
        "SELECT CreativeId, CreativeData FROM CreativeMetadata WHERE "
        "LastUpdateTime > @latest_time");
    spanner::SqlStatement::ParamType params = {
        {"latest_time", spanner::Value(latest_read_)}};
    auto rows = client_->ExecuteQuery(spanner::SqlStatement(stmt, params));
    mutex_.Lock();
    for (auto const& row :
         spanner::StreamOf<std::tuple<std::string, spanner::Bytes>>(rows)) {
      if (!row) {
        LOG(ERROR) << "Invalid Spanner response.";
        return;
      }
      creative_data_.insert_or_assign(std::get<0>(*row), std::get<1>(*row));
    }
    mutex_.Unlock();
  }
}

trusted_server::Response CreativeMap::Lookup(
    const std::vector<std::string>& keys) const {
  trusted_server::Response response;
  for (const auto& key : keys) {
    auto* creative = response.add_creatives();
    creative->set_key(key);
  }

  mutex_.ReaderLock();
  for (auto& creative : *response.mutable_creatives()) {
    auto val_it = creative_data_.find(creative.key());
    if (val_it != creative_data_.end()) {
      creative.set_creative_data(val_it->second.get<std::string>());
    }
  }
  mutex_.ReaderUnlock();
  return response;
}

}  // namespace trusted_server