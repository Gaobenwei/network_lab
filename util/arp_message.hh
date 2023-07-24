#pragma once

#include "ethernet_header.hh"
#include "ipv4_header.hh"
#include "parser.hh"

// [ARP](\ref rfc::rfc826) message
struct ARPMessage
{
  static constexpr size_t LENGTH = 28;         // ARP message length in bytes
  static constexpr uint16_t TYPE_ETHERNET = 1; // ARP type for Ethernet/Wi-Fi as link-layer protocol 以太网/Wi-Fi作为链路层协议的ARP类型
  static constexpr uint16_t OPCODE_REQUEST = 1;
  static constexpr uint16_t OPCODE_REPLY = 2;

  uint16_t hardware_type = TYPE_ETHERNET;             // Type of the link-layer protocol (generally Ethernet/Wi-Fi) 链路层协议类型(一般为以太网/Wi-Fi)
  uint16_t protocol_type = EthernetHeader::TYPE_IPv4; // Type of the Internet-layer protocol (generally IPv4) internet层协议类型(一般为IPv4)
  uint8_t hardware_address_size = sizeof( EthernetHeader::src );
  uint8_t protocol_address_size = sizeof( IPv4Header::src );
  uint16_t opcode {}; // Request or reply 请求或答复

  EthernetAddress sender_ethernet_address {}; //发送方以太网地址
  uint32_t sender_ip_address {};

  EthernetAddress target_ethernet_address {}; //目标以太网地址
  uint32_t target_ip_address {};

  // Return a string containing the ARP message in human-readable format 以人类可读的格式返回包含ARP消息的字符串
  std::string to_string() const;

  // Is this type of ARP message supported by the parser? 解析器是否支持这种类型的ARP消息?
  bool supported() const;

  void parse( Parser& parser );
  void serialize( Serializer& serializer ) const;
};
