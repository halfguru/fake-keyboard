#include <bluetooth/sdp.hpp>
#include <catch2/catch_test_macros.hpp>
#include <hid/report.hpp>

using namespace fakekbd::bluetooth;
using namespace fakekbd::hid;

TEST_CASE("sdp_record builds with default values", "[unit][sdp]")
{
  sdp_record record;
  auto xml = record.build_xml();

  REQUIRE_FALSE(xml.empty());
  REQUIRE(xml.find("<?xml version=\"1.0\"") != std::string::npos);
  REQUIRE(xml.find("<record>") != std::string::npos);
  REQUIRE(xml.find("</record>") != std::string::npos);
}

TEST_CASE("sdp_record sets device name", "[unit][sdp]")
{
  sdp_record record;
  record.set_device_name("Test Device");
  auto xml = record.build_xml();

  REQUIRE(xml.find("Test Device") != std::string::npos);
}

TEST_CASE("sdp_record sets vendor ID", "[unit][sdp]")
{
  sdp_record record;
  record.set_vendor_id(0xABCD);
  auto xml = record.build_xml();

  REQUIRE(xml.find("0xabcd") != std::string::npos);
}

TEST_CASE("sdp_record sets product ID", "[unit][sdp]")
{
  sdp_record record;
  record.set_product_id(0x1234);
  auto xml = record.build_xml();

  REQUIRE(xml.find("0x1234") != std::string::npos);
}

TEST_CASE("sdp_record sets version", "[unit][sdp]")
{
  sdp_record record;
  record.set_version(0x0200);
  auto xml = record.build_xml();

  REQUIRE(xml.find("0x200") != std::string::npos);
}

TEST_CASE("sdp_record sets report descriptor", "[unit][sdp]")
{
  sdp_record record;
  std::vector<uint8_t> descriptor = { 0x05, 0x01, 0x09, 0x06 };
  record.set_report_descriptor(descriptor);
  auto xml = record.build_xml();

  REQUIRE(xml.find("encoding=\"hex\"") != std::string::npos);
  REQUIRE(xml.find("0x5") != std::string::npos);
  REQUIRE(xml.find("0x1") != std::string::npos);
  REQUIRE(xml.find("0x9") != std::string::npos);
  REQUIRE(xml.find("0x6") != std::string::npos);
}

TEST_CASE("sdp_record supports method chaining", "[unit][sdp]")
{
  auto xml = sdp_record{}
               .set_device_name("Chained Device")
               .set_vendor_id(0xDEAD)
               .set_product_id(0xBEEF)
               .set_version(0x0100)
               .build_xml();

  REQUIRE_FALSE(xml.empty());
  REQUIRE(xml.find("Chained Device") != std::string::npos);
  REQUIRE(xml.find("0xdead") != std::string::npos);
  REQUIRE(xml.find("0xbeef") != std::string::npos);
}

TEST_CASE("sdp_record includes HID service class UUID", "[unit][sdp]")
{
  sdp_record record;
  auto xml = record.build_xml();

  REQUIRE(xml.find("0x1124") != std::string::npos);
}

TEST_CASE("sdp_record includes L2CAP PSM values", "[unit][sdp]")
{
  sdp_record record;
  auto xml = record.build_xml();

  REQUIRE(xml.find("0x0011") != std::string::npos);
  REQUIRE(xml.find("0x0013") != std::string::npos);
}

TEST_CASE("sdp_record includes HID descriptor type", "[unit][sdp]")
{
  sdp_record record;
  std::vector<uint8_t> descriptor = { 0x05, 0x07 };
  record.set_report_descriptor(descriptor);
  auto xml = record.build_xml();

  REQUIRE(xml.find("0x22") != std::string::npos);
}

TEST_CASE("sdp_record includes HID attributes", "[unit][sdp]")
{
  sdp_record record;
  auto xml = record.build_xml();

  REQUIRE(xml.find("0x0200") != std::string::npos);
  REQUIRE(xml.find("0x0201") != std::string::npos);
  REQUIRE(xml.find("0x0202") != std::string::npos);
}

TEST_CASE("build_hid_sdp_record creates valid XML", "[unit][sdp]")
{
  auto descriptor = build_keyboard_report_descriptor();
  auto xml = build_hid_sdp_record("Test Keyboard", 0x1234, 0x5678, 0x0100, descriptor);

  REQUIRE_FALSE(xml.empty());
  REQUIRE(xml.find("Test Keyboard") != std::string::npos);
  REQUIRE(xml.find("0x1234") != std::string::npos);
  REQUIRE(xml.find("0x5678") != std::string::npos);
}

TEST_CASE("build_keyboard_sdp_record creates valid XML with defaults", "[unit][sdp]")
{
  auto xml = build_keyboard_sdp_record();

  REQUIRE_FALSE(xml.empty());
  REQUIRE(xml.find("Fake Keyboard") != std::string::npos);
  REQUIRE(xml.find("<?xml") != std::string::npos);
  REQUIRE(xml.find("<record>") != std::string::npos);
}

TEST_CASE("build_keyboard_sdp_record includes report descriptor", "[unit][sdp]")
{
  auto xml = build_keyboard_sdp_record("Custom Keyboard");

  REQUIRE(xml.find("encoding=\"hex\"") != std::string::npos);
  REQUIRE(xml.find("Custom Keyboard") != std::string::npos);
}

TEST_CASE("sdp_record encodes empty descriptor correctly", "[unit][sdp]")
{
  sdp_record record;
  std::vector<uint8_t> empty_descriptor;
  record.set_report_descriptor(empty_descriptor);
  auto xml = record.build_xml();

  REQUIRE_FALSE(xml.empty());
}

TEST_CASE("sdp_record encodes large descriptor", "[unit][sdp]")
{
  sdp_record record;
  std::vector<uint8_t> large_descriptor(100, 0xAA);
  record.set_report_descriptor(large_descriptor);
  auto xml = record.build_xml();

  REQUIRE(xml.find("encoding=\"hex\"") != std::string::npos);
}

TEST_CASE("sdp_record includes SDP version", "[unit][sdp]")
{
  sdp_record record;
  auto xml = record.build_xml();

  REQUIRE(xml.find("0x0100") != std::string::npos);
}

TEST_CASE("sdp_record includes browse group list", "[unit][sdp]")
{
  sdp_record record;
  auto xml = record.build_xml();

  REQUIRE(xml.find("0x0001") != std::string::npos);
}

TEST_CASE("sdp_record XML is well-formed", "[unit][sdp]")
{
  sdp_record record;
  auto xml = record.build_xml();

  auto open_count = std::count(xml.begin(), xml.end(), '<');
  auto close_count = std::count(xml.begin(), xml.end(), '>');

  REQUIRE(open_count == close_count);
}

TEST_CASE("build_keyboard_sdp_record with custom name", "[unit][sdp]")
{
  auto xml = build_keyboard_sdp_record("My Custom Keyboard");

  REQUIRE(xml.find("My Custom Keyboard") != std::string::npos);
}

TEST_CASE("sdp_record includes HID release version attribute", "[unit][sdp]")
{
  sdp_record record;
  auto xml = record.build_xml();

  REQUIRE(xml.find("0x020b") != std::string::npos);
}

TEST_CASE("sdp_record includes parser version", "[unit][sdp]")
{
  sdp_record record;
  auto xml = record.build_xml();

  REQUIRE(xml.find("0x020c") != std::string::npos);
}

TEST_CASE("sdp_record includes device subversion", "[unit][sdp]")
{
  sdp_record record;
  auto xml = record.build_xml();

  REQUIRE(xml.find("0x020f") != std::string::npos);
  REQUIRE(xml.find("0x0210") != std::string::npos);
}
