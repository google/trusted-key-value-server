#!/bin/bash
# Copyright 2021 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


# Fail on any error.
set -e

rm -fr /home/kbuilder/.cache/bazel/_bazel_kbuilder/install/4cfcf40fe067e89c8f5c38e156f8d8ca
use_bazel.sh 4.0.0

# Specify we want to build with GCC 9
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo aptitude update -yq
sudo aptitude install -yq gcc-9 g++-9
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 100 --slave /usr/bin/g++ g++ /usr/bin/g++-9
sudo update-alternatives --set gcc "/usr/bin/gcc-9"

cd "${KOKORO_ARTIFACTS_DIR}/git/trusted-server"
./presubmits/build.sh
./presubmits/clang.sh
./presubmits/test.sh