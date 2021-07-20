workspace(name = "trusted-server")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "io_bazel_rules_docker",
    sha256 = "59d5b42ac315e7eadffa944e86e90c2990110a1c8075f1cd145f487e999d22b3",
    strip_prefix = "rules_docker-0.17.0",
    urls = ["https://github.com/bazelbuild/rules_docker/releases/download/v0.17.0/rules_docker-v0.17.0.tar.gz"],
)
load(
    "@io_bazel_rules_docker//toolchains/docker:toolchain.bzl",
    docker_toolchain_configure = "toolchain_configure",
)
docker_toolchain_configure(
    name = "docker_config",
)
load(
    "@io_bazel_rules_docker//repositories:repositories.bzl",
    container_repositories = "repositories",
)
container_repositories()
load("@io_bazel_rules_docker//repositories:deps.bzl", container_deps = "deps")
container_deps()
load("@io_bazel_rules_docker//container:pull.bzl", "container_pull")

container_pull(
    name = "gcr-ubuntu",
    registry = "launcher.gcr.io",
    repository = "google/ubuntu16_04",
    tag = "latest",
)

# Loading Boost as recommended by https://docs.bazel.build/versions/1.2.0/rules.html
http_archive(
    name = "com_github_nelhage_rules_boost",
    strip_prefix = "rules_boost-652b21e35e4eeed5579e696da0facbe8dba52b1f",
    urls = ["https://github.com/nelhage/rules_boost/archive/652b21e35e4eeed5579e696da0facbe8dba52b1f.tar.gz"],
)

load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")
boost_deps()

load("@rules_pkg//:deps.bzl", "rules_pkg_dependencies")
rules_pkg_dependencies()

# Abseil library repository
http_archive(
    name = "com_google_absl",
    strip_prefix = "abseil-cpp-2e9532cc6c701a8323d0cffb468999ab804095ab",
    urls = ["https://github.com/abseil/abseil-cpp/archive/2e9532cc6c701a8323d0cffb468999ab804095ab.tar.gz"],
)

# Google Test
http_archive(
    name = "googletest",
    strip_prefix = "googletest-703bd9caab50b139428cea1aaff9974ebee5742e",
    urls = ["https://github.com/google/googletest/archive/703bd9caab50b139428cea1aaff9974ebee5742e.tar.gz"],
)

http_archive(
    name = "subprocess",
    build_file = "@//third_party:subprocess.BUILD",
    strip_prefix = "subprocess-0.4.0/src/cpp",
    urls = ["https://github.com/benman64/subprocess/archive/v0.4.0.tar.gz"],
)

# Protocol Buffer
http_archive(
    name = "rules_proto",
    strip_prefix = "rules_proto-f7a30f6f80006b591fa7c437fe5a951eb10bcbcf",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_proto/archive/f7a30f6f80006b591fa7c437fe5a951eb10bcbcf.tar.gz",
        "https://github.com/bazelbuild/rules_proto/archive/f7a30f6f80006b591fa7c437fe5a951eb10bcbcf.tar.gz",
    ],
)
load("@rules_proto//proto:repositories.bzl", "rules_proto_dependencies", "rules_proto_toolchains")
rules_proto_dependencies()
rules_proto_toolchains()

# Google Logging
http_archive(
    name = "com_github_gflags_gflags",
    sha256 = "34af2f15cf7367513b352bdcd2493ab14ce43692d2dcd9dfc499492966c64dcf",
    strip_prefix = "gflags-2.2.2",
    urls = ["https://github.com/gflags/gflags/archive/v2.2.2.tar.gz"],
)

http_archive(
    name = "com_github_google_glog",
    sha256 = "62efeb57ff70db9ea2129a16d0f908941e355d09d6d83c9f7b18557c0a7ab59e",
    strip_prefix = "glog-d516278b1cd33cd148e8989aec488b6049a4ca0b",
    urls = ["https://github.com/google/glog/archive/d516278b1cd33cd148e8989aec488b6049a4ca0b.zip"],
)
# Fetch the Google Cloud C++ libraries.
http_archive(
    name = "com_github_googleapis_google_cloud_cpp",
    sha256 = "84a7ac7b63db986bb737462e374c11fc6f35f6020ccaacec1d0e4d61ec929528",
    strip_prefix = "google-cloud-cpp-1.27.0",
    url = "https://github.com/googleapis/google-cloud-cpp/archive/v1.27.0.tar.gz",
)

# Load indirect dependencies due to
#     https://github.com/bazelbuild/bazel/issues/1943
load("@com_github_googleapis_google_cloud_cpp//bazel:google_cloud_cpp_deps.bzl", "google_cloud_cpp_deps")

google_cloud_cpp_deps()

load("@com_google_googleapis//:repository_rules.bzl", "switched_rules_by_language")

switched_rules_by_language(
    name = "com_google_googleapis_imports",
    cc = True,
    grpc = True,
)

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

grpc_deps()

load("@com_github_grpc_grpc//bazel:grpc_extra_deps.bzl", "grpc_extra_deps")

grpc_extra_deps()