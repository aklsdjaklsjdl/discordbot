load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
    name = "com_github_google_dpp",
    commit = "f87c45dc632c8ece9079fadbcc87cb1a0b8d17f2",
    remote = "https://github.com/brainboxdotcc/DPP.git",
)

http_archive(
  name = "com_google_absl",
  urls = ["https://github.com/abseil/abseil-cpp/archive/98eb410c93ad059f9bba1bf43f5bb916fc92a5ea.zip"],
  strip_prefix = "abseil-cpp-98eb410c93ad059f9bba1bf43f5bb916fc92a5ea",
)
# GoogleTest/GoogleMock framework. Used by most unit-tests.
http_archive(
    name = "com_google_googletest", 
    sha256 = "a1d3123179024258f9c399d45da3e0b09c4aaf8d2c041466ce5b4793a8929f23",
    strip_prefix = "googletest-86add13493e5c881d7e4ba77fb91c1f57752b3a4",
    urls = ["https://github.com/google/googletest/archive/86add13493e5c881d7e4ba77fb91c1f57752b3a4.zip"],
)

# RE2 (the regular expression library used by GoogleTest)
# Note this must use a commit from the `abseil` branch of the RE2 project.
# https://github.com/google/re2/tree/abseil
http_archive(
    name = "com_googlesource_code_re2",
    sha256 = "0a890c2aa0bb05b2ce906a15efb520d0f5ad4c7d37b8db959c43772802991887",
    strip_prefix = "re2-a427f10b9fb4622dd6d8643032600aa1b50fbd12",
    urls = ["https://github.com/google/re2/archive/a427f10b9fb4622dd6d8643032600aa1b50fbd12.zip"],
)

# Google benchmark.
http_archive(
    name = "com_github_google_benchmark",
    sha256 = "a62735f4f4bebc069225aa725dc68eb6836dcdbf550424b5f18e2673e7d40f2c",
    strip_prefix = "benchmark-da652a748675b679947710117329e9f77f374f2d",
    urls = ["https://github.com/google/benchmark/archive/da652a748675b679947710117329e9f77f374f2d.zip"],
)

# Bazel Skylib.
http_archive(
    name = "bazel_skylib",
    urls = ["https://github.com/bazelbuild/bazel-skylib/releases/download/1.2.1/bazel-skylib-1.2.1.tar.gz"],
    sha256 = "f7be3474d42aae265405a592bb7da8e171919d74c16f082a5457840f06054728",
)

# Bazel platform rules.
http_archive(
    name = "platforms",
    sha256 = "a879ea428c6d56ab0ec18224f976515948822451473a80d06c2e50af0bbe5121",
    strip_prefix = "platforms-da5541f26b7de1dc8e04c075c99df5351742a4a2",
    urls = ["https://github.com/bazelbuild/platforms/archive/da5541f26b7de1dc8e04c075c99df5351742a4a2.zip"],
)