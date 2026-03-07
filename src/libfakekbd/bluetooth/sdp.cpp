#include "sdp.hpp"
#include "libfakekbd/hid/report.hpp"
#include <iomanip>
#include <spdlog/spdlog.h>
#include <sstream>

namespace fakekbd::bluetooth {

auto
sdp_record::set_device_name(std::string_view name) -> sdp_record&
{
  device_name_ = std::string(name);
  return *this;
}

auto
sdp_record::set_vendor_id(uint16_t id) -> sdp_record&
{
  vendor_id_ = id;
  return *this;
}

auto
sdp_record::set_product_id(uint16_t id) -> sdp_record&
{
  product_id_ = id;
  return *this;
}

auto
sdp_record::set_version(uint16_t version) -> sdp_record&
{
  version_ = version;
  return *this;
}

auto
sdp_record::set_report_descriptor(std::vector<uint8_t> const& descriptor) -> sdp_record&
{
  report_descriptor_ = descriptor;
  return *this;
}

auto
sdp_record::build_xml() const -> std::string
{
  std::ostringstream xml;

  xml << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
  xml << "<record>\n";

  xml << "  <attribute id=\"0x0001\">\n";
  xml << "    <sequence>\n";
  xml << "      <uuid value=\"0x1124\" />\n";
  xml << "    </sequence>\n";
  xml << "  </attribute>\n";

  xml << "  <attribute id=\"0x0004\">\n";
  xml << "    <sequence>\n";
  xml << "      <sequence>\n";
  xml << "        <uuid value=\"0x0100\" />\n";
  xml << "        <uint16 value=\"0x0011\" />\n";
  xml << "      </sequence>\n";
  xml << "      <sequence>\n";
  xml << "        <uuid value=\"0x0011\" />\n";
  xml << "      </sequence>\n";
  xml << "    </sequence>\n";
  xml << "  </attribute>\n";

  xml << "  <attribute id=\"0x0005\">\n";
  xml << "    <sequence>\n";
  xml << "      <uuid value=\"0x1002\" />\n";
  xml << "    </sequence>\n";
  xml << "  </attribute>\n";

  xml << "  <attribute id=\"0x0006\">\n";
  xml << "    <sequence>\n";
  xml << "      <uint16 value=\"0x656e\" />\n";
  xml << "      <uint16 value=\"0x006a\" />\n";
  xml << "      <uint16 value=\"0x0100\" />\n";
  xml << "    </sequence>\n";
  xml << "  </attribute>\n";

  xml << "  <attribute id=\"0x0009\">\n";
  xml << "    <sequence>\n";
  xml << "      <sequence>\n";
  xml << "        <uuid value=\"0x1124\" />\n";
  xml << "        <uint16 value=\"0x0100\" />\n";
  xml << "      </sequence>\n";
  xml << "    </sequence>\n";
  xml << "  </attribute>\n";

  xml << "  <attribute id=\"0x000d\">\n";
  xml << "    <sequence>\n";
  xml << "      <sequence>\n";
  xml << "        <sequence>\n";
  xml << "          <uuid value=\"0x0100\" />\n";
  xml << "          <uint16 value=\"0x0013\" />\n";
  xml << "        </sequence>\n";
  xml << "        <sequence>\n";
  xml << "          <uuid value=\"0x0011\" />\n";
  xml << "        </sequence>\n";
  xml << "      </sequence>\n";
  xml << "    </sequence>\n";
  xml << "  </attribute>\n";

  xml << "  <attribute id=\"0x0100\">\n";
  xml << "    <text value=\"" << device_name_ << "\" />\n";
  xml << "  </attribute>\n";

  xml << "  <attribute id=\"0x0101\">\n";
  xml << "    <text value=\"Fake Keyboard\" />\n";
  xml << "  </attribute>\n";

  xml << "  <attribute id=\"0x0102\">\n";
  xml << "    <text value=\"Fake Keyboard\" />\n";
  xml << "  </attribute>\n";

  xml << "  <attribute id=\"0x0200\">\n";
  xml << "    <uint16 value=\"0x0100\" />\n";
  xml << "  </attribute>\n";

  xml << "  <attribute id=\"0x0201\">\n";
  xml << "    <uint16 value=\"" << std::hex << std::showbase << vendor_id_ << "\" />\n";
  xml << "  </attribute>\n";

  xml << "  <attribute id=\"0x0202\">\n";
  xml << "    <uint16 value=\"" << std::hex << std::showbase << product_id_ << "\" />\n";
  xml << "  </attribute>\n";

  xml << "  <attribute id=\"0x0203\">\n";
  xml << "    <uint8 value=\"0x40\" />\n";
  xml << "  </attribute>\n";

  xml << "  <attribute id=\"0x0204\">\n";
  xml << "    <boolean value=\"true\" />\n";
  xml << "  </attribute>\n";

  xml << "  <attribute id=\"0x0205\">\n";
  xml << "    <boolean value=\"true\" />\n";
  xml << "  </attribute>\n";

  if (!report_descriptor_.empty()) {
    xml << "  <attribute id=\"0x0206\">\n";
    xml << "    <sequence>\n";
    xml << "      <sequence>\n";
    xml << "        <uint8 value=\"0x22\" />\n";
    xml << "        <text encoding=\"hex\" value=\"";

    for (auto byte : report_descriptor_) {
      xml << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }

    xml << "\" />\n";
    xml << "      </sequence>\n";
    xml << "    </sequence>\n";
    xml << "  </attribute>\n";
  }

  xml << "  <attribute id=\"0x0207\">\n";
  xml << "    <sequence>\n";
  xml << "      <sequence>\n";
  xml << "        <uint16 value=\"0x0409\" />\n";
  xml << "        <uint16 value=\"0x0100\" />\n";
  xml << "      </sequence>\n";
  xml << "    </sequence>\n";
  xml << "  </attribute>\n";

  xml << "  <attribute id=\"0x020b\">\n";
  xml << "    <uint16 value=\"" << std::hex << std::showbase << version_ << "\" />\n";
  xml << "  </attribute>\n";

  xml << "  <attribute id=\"0x020c\">\n";
  xml << "    <uint16 value=\"0x0c80\" />\n";
  xml << "  </attribute>\n";

  xml << "  <attribute id=\"0x020d\">\n";
  xml << "    <boolean value=\"true\" />\n";
  xml << "  </attribute>\n";

  xml << "  <attribute id=\"0x020e\">\n";
  xml << "    <boolean value=\"false\" />\n";
  xml << "  </attribute>\n";

  xml << "  <attribute id=\"0x020f\">\n";
  xml << "    <uint16 value=\"0x0640\" />\n";
  xml << "  </attribute>\n";

  xml << "  <attribute id=\"0x0210\">\n";
  xml << "    <uint16 value=\"0x0320\" />\n";
  xml << "  </attribute>\n";

  xml << "</record>\n";

  return xml.str();
}

auto
build_hid_sdp_record(std::string_view device_name,
                     uint16_t vendor_id,
                     uint16_t product_id,
                     uint16_t version,
                     std::vector<uint8_t> const& report_descriptor) -> std::string
{
  return sdp_record{}
    .set_device_name(device_name)
    .set_vendor_id(vendor_id)
    .set_product_id(product_id)
    .set_version(version)
    .set_report_descriptor(report_descriptor)
    .build_xml();
}

auto
build_keyboard_sdp_record(std::string_view device_name) -> std::string
{
  auto report_descriptor = hid::build_keyboard_report_descriptor();

  return build_hid_sdp_record(device_name, 0x1234, 0x5678, 0x0100, report_descriptor);
}

}
