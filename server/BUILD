load("@io_bazel_rules_docker//container:container.bzl", "container_image", "container_push")
load("@rules_pkg//:pkg.bzl", "pkg_tar")

cc_binary(
    name = "server",
    srcs = ["server.cc"],
    deps = [
        "//data:creative_map",
        "//data:mock_creative_map",
        "//proto:response_cc_proto",
        "@boost//:asio",
        "@boost//:beast",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@com_github_googleapis_google_cloud_cpp//:spanner",
    ],
)

cc_test(
    name = "server_test",
    srcs = ["server_test.cc"],
    data = [
        ":server",
    ],
    deps = [
        "//proto:creative_data_cc_proto",
        "//proto:response_cc_proto",
        "@boost//:asio",
        "@boost//:beast",
        "@boost//:date_time",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/cleanup",
        "@com_google_absl//absl/random",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
        "@com_google_protobuf//:protobuf",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
        "@subprocess",
    ],
)

pkg_tar(
    name = "entrypoint",
    srcs = [
        ":entrypoint.sh",
        ":server",
    ],
    package_dir = "/usr/local/bin",
    strip_prefix = ".",
)

container_image(
    name = "server-image",
    base = "@gcr-ubuntu//image",
    cmd = [
        "/bin/sh",
        "-c",
        "/usr/local/bin/entrypoint.sh",
    ],
    tars = [
        ":entrypoint",
    ],
)

container_push(
    name = "push_server",
    format = "Docker",
    image = ":server-image",
    registry = "gcr.io",
    repository = "ads-trusted-server-dev/server-image",
    tag = "latest",
)
