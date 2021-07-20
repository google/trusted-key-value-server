load("@io_bazel_rules_docker//container:container.bzl", "container_image", "container_push")
load("@rules_pkg//:pkg.bzl", "pkg_tar")

package(default_visibility = ["//server:__subpackages__",])

cc_library(
    name = "creative_map",
    srcs = ["creative_map.cc"],
    hdrs = ["creative_map.h"],
    deps = [
        "//proto:response_cc_proto",
        "@boost//:asio",
        "@boost//:beast",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/synchronization:synchronization",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@com_github_googleapis_google_cloud_cpp//:spanner",
    ],
)

cc_library(
    name = "mock_creative_map",
    srcs = ["mock_creative_map.cc"],
    hdrs = ["mock_creative_map.h"],
    deps = [
        ":creative_map",
        "//proto:response_cc_proto",
        "//proto:creative_data_cc_proto",
        "@com_google_absl//absl/time",
        "@com_google_protobuf//:protobuf",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
        "@com_github_googleapis_google_cloud_cpp//:spanner",
        "@com_github_googleapis_google_cloud_cpp//google/cloud/spanner:spanner_client_testing",
    ],
)

cc_test(
    name = "creative_map_test",
    srcs = ["creative_map_test.cc"],
    linkstatic = 1,
    deps = [
        ":mock_creative_map",
        "//proto:response_cc_proto",
        "//proto:creative_data_cc_proto",
        "@boost//:asio",
        "@boost//:beast",
        "@boost//:date_time",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/cleanup",
        "@com_google_absl//absl/random",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
        "@com_google_protobuf//:protobuf",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
        "@com_github_googleapis_google_cloud_cpp//:spanner",
        "@com_github_googleapis_google_cloud_cpp//google/cloud/spanner:spanner_client_testing",
    ],
)