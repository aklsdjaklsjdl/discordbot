cc_library(
    name = "philomena",
    srcs = ["src/philomena.cc"],
    hdrs = ["src/philomena.h", "src/json.hpp"],
    deps = ["@com_google_absl//absl/strings"],
    linkopts = ["-lcurl"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "bot",
    srcs = ["src/bot.cpp"],
    hdrs = ["src/bot.h"],
    deps = ["@com_google_absl//absl/strings",  ":philomena"],
    linkopts = ["-lcurl", "-lpthread", "-ldpp"],
)

cc_binary(
    name = "bot_main",
    srcs = ["src/bot_main.cpp"],
    deps = [":bot"],
    linkopts = ["-lcurl", "-lpthread", "-ldpp"],
)

cc_test(
  name = "test",
  size = "small",
  srcs = ["test/test.cc"],
  deps = ["@com_google_googletest//:gtest_main", ":philomena", ":bot"],
)
