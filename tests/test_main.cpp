#include <catch2/catch_test_macros.hpp>
#include <spdlog/spdlog.h>

TEST_CASE("spdlog is working", "[unit]")
{
  REQUIRE_NOTHROW(spdlog::info("test"));
}

TEST_CASE("basic math works", "[unit]")
{
  REQUIRE(1 + 1 == 2);
  REQUIRE(2 * 3 == 6);
}

TEST_CASE("string operations", "[unit]")
{
  std::string hello = "hello";
  REQUIRE(hello.size() == 5);
  REQUIRE(hello + " world" == "hello world");
}
