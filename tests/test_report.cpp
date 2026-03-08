#include <catch2/catch_test_macros.hpp>
#include <libfakekbd/hid/report.hpp>

using namespace fakekbd::hid;

TEST_CASE("report_descriptor_builder creates empty descriptor", "[unit][report]")
{
  report_descriptor_builder builder;
  auto descriptor = builder.build();
  REQUIRE(descriptor.empty());
}

TEST_CASE("report_descriptor_builder appends usage_page", "[unit][report]")
{
  report_descriptor_builder builder;
  builder.usage_page(usage_page::KEYBOARD);
  auto descriptor = builder.build();

  REQUIRE(descriptor.size() == 2);
  REQUIRE(descriptor[0] == 0x05);
  REQUIRE(descriptor[1] == 0x07);
}

TEST_CASE("report_descriptor_builder appends usage", "[unit][report]")
{
  report_descriptor_builder builder;
  builder.usage(usage::KEYBOARD);
  auto descriptor = builder.build();

  REQUIRE(descriptor.size() == 2);
  REQUIRE(descriptor[0] == 0x09);
  REQUIRE(descriptor[1] == 0x06);
}

TEST_CASE("report_descriptor_builder appends collection", "[unit][report]")
{
  report_descriptor_builder builder;
  builder.collection(collection::APPLICATION);
  auto descriptor = builder.build();

  REQUIRE(descriptor.size() == 2);
  REQUIRE(descriptor[0] == 0xA1);
  REQUIRE(descriptor[1] == collection::APPLICATION);
}

TEST_CASE("report_descriptor_builder appends end_collection", "[unit][report]")
{
  report_descriptor_builder builder;
  builder.end_collection();
  auto descriptor = builder.build();

  REQUIRE(descriptor.size() == 2);
  REQUIRE(descriptor[0] == 0xC1);
  REQUIRE(descriptor[1] == 0);
}

TEST_CASE("report_descriptor_builder appends report_size", "[unit][report]")
{
  report_descriptor_builder builder;
  builder.report_size(8);
  auto descriptor = builder.build();

  REQUIRE(descriptor.size() == 2);
  REQUIRE(descriptor[0] == 0x75);
  REQUIRE(descriptor[1] == 8);
}

TEST_CASE("report_descriptor_builder appends report_count", "[unit][report]")
{
  report_descriptor_builder builder;
  builder.report_count(6);
  auto descriptor = builder.build();

  REQUIRE(descriptor.size() == 2);
  REQUIRE(descriptor[0] == 0x95);
  REQUIRE(descriptor[1] == 6);
}

TEST_CASE("report_descriptor_builder appends logical_min", "[unit][report]")
{
  report_descriptor_builder builder;
  builder.logical_min(0);
  auto descriptor = builder.build();

  REQUIRE(descriptor.size() == 2);
  REQUIRE(descriptor[0] == 0x15);
  REQUIRE(descriptor[1] == 0);
}

TEST_CASE("report_descriptor_builder appends logical_max", "[unit][report]")
{
  report_descriptor_builder builder;
  builder.logical_max(255);
  auto descriptor = builder.build();

  REQUIRE(descriptor.size() == 2);
  REQUIRE(descriptor[0] == 0x25);
  REQUIRE(descriptor[1] == 255);
}

TEST_CASE("report_descriptor_builder appends input", "[unit][report]")
{
  report_descriptor_builder builder;
  builder.input(report_flags::DATA | report_flags::VARIABLE | report_flags::ABSOLUTE);
  auto descriptor = builder.build();

  REQUIRE(descriptor.size() == 2);
  REQUIRE(descriptor[0] == 0x81);
  REQUIRE(descriptor[1] == (report_flags::DATA | report_flags::VARIABLE | report_flags::ABSOLUTE));
}

TEST_CASE("report_descriptor_builder appends output", "[unit][report]")
{
  report_descriptor_builder builder;
  builder.output(report_flags::DATA | report_flags::VARIABLE | report_flags::ABSOLUTE);
  auto descriptor = builder.build();

  REQUIRE(descriptor.size() == 2);
  REQUIRE(descriptor[0] == 0x91);
  REQUIRE(descriptor[1] == (report_flags::DATA | report_flags::VARIABLE | report_flags::ABSOLUTE));
}

TEST_CASE("report_descriptor_builder supports method chaining", "[unit][report]")
{
  auto descriptor = report_descriptor_builder()
                      .usage_page(usage_page::GENERIC_DESKTOP)
                      .usage(usage::KEYBOARD)
                      .collection(collection::APPLICATION)
                      .end_collection()
                      .build();

  REQUIRE_FALSE(descriptor.empty());
}

TEST_CASE("build_keyboard_report_descriptor returns non-empty descriptor", "[unit][report]")
{
  auto descriptor = build_keyboard_report_descriptor();
  REQUIRE_FALSE(descriptor.empty());
  REQUIRE(descriptor.size() > 50);
}

TEST_CASE("build_keyboard_report_descriptor contains usage page", "[unit][report]")
{
  auto descriptor = build_keyboard_report_descriptor();

  bool found_usage_page = false;
  for (size_t i = 0; i < descriptor.size() - 1; ++i)
  {
    if (descriptor[i] == 0x05 && descriptor[i + 1] == 0x01)
    {
      found_usage_page = true;
      break;
    }
  }
  REQUIRE(found_usage_page);
}

TEST_CASE("report_descriptor_builder handles extended usage_page", "[unit][report]")
{
  report_descriptor_builder builder;
  builder.usage_page(0xFF00);
  auto descriptor = builder.build();

  REQUIRE(descriptor.size() >= 3);
  REQUIRE(descriptor[0] == 0x06);
}

TEST_CASE("report_descriptor_builder handles negative logical_min", "[unit][report]")
{
  report_descriptor_builder builder;
  builder.logical_min(-128);
  auto descriptor = builder.build();

  REQUIRE_FALSE(descriptor.empty());
  REQUIRE(descriptor.size() >= 2);
}

TEST_CASE("report_descriptor_builder data method returns span", "[unit][report]")
{
  report_descriptor_builder builder;
  builder.usage_page(usage_page::KEYBOARD);

  auto data = builder.data();
  REQUIRE(data.size() == 2);
  REQUIRE(data[0] == 0x05);
}

TEST_CASE("report constants are correctly defined", "[unit][report]")
{
  REQUIRE(usage_page::GENERIC_DESKTOP == 0x01);
  REQUIRE(usage_page::KEYBOARD == 0x07);
  REQUIRE(usage_page::LED == 0x08);
  REQUIRE(usage_page::BUTTON == 0x09);

  REQUIRE(usage::KEYBOARD == 0x06);
  REQUIRE(usage::KEYBOARD_NUM_LOCK == 0x01);

  REQUIRE(collection::PHYSICAL == 0x00);
  REQUIRE(collection::APPLICATION == 0x01);
  REQUIRE(collection::LOGICAL == 0x02);

  REQUIRE(report_flags::DATA == 0x00);
  REQUIRE(report_flags::CONSTANT == 0x01);
  REQUIRE(report_flags::VARIABLE == 0x02);
  REQUIRE(report_flags::ABSOLUTE == 0x00);
  REQUIRE(report_flags::RELATIVE == 0x04);
}
