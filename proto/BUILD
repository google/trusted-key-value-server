load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_proto_library")

package(default_visibility = ["//server:__subpackages__",
                              "//data:__subpackages__"])

proto_library(
    name = "creative_data_proto",
    srcs = ["creative_data.proto"],
)

cc_proto_library(
    name = "creative_data_cc_proto",
    deps = [":creative_data_proto"],
)

proto_library(
    name = "response_proto",
    srcs = ["response.proto"],
    deps = [
        ":creative_data_proto",
    ],
)

cc_proto_library(
    name = "response_cc_proto",
    deps = [":response_proto"],
)

