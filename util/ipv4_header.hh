#pragma once

#include "parser.hh"

#include <cstddef>
#include <cstdint>
#include <string>

// IPv4 Internet datagram header (note: IP options are not supported) IPv4互联网数据报报头(注意:不支持IP选项)
struct IPv4Header
{
  static constexpr size_t LENGTH = 20;        // IPv4 header length, not including options
  static constexpr uint8_t DEFAULT_TTL = 128; // A reasonable default TTL value 合理的默认TTL值
  static constexpr uint8_t PROTO_TCP = 6;     // Protocol number for TCP TCP协议号

  static constexpr uint64_t serialized_length() { return LENGTH; }

  /*
   *   0                   1                   2                   3
   *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *  |Version|  IHL  |Type of Service|          Total Length         |
   *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *  |         Identification        |Flags|      Fragment Offset    |
   *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *  |  Time to Live |    Protocol   |         Header Checksum       |
   *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *  |                       Source Address                          |
   *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *  |                    Destination Address                        |
   *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *  |                    Options                    |    Padding    |
   *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   */

  // IPv4 Header fields IPv4报头字段
  uint8_t ver = 4;           // IP version  IP版本
  uint8_t hlen = LENGTH / 4; // header length (multiples of 32 bits) 报头长度(32位的倍数)
  uint8_t tos = 0;           // type of service 服务类型
  uint16_t len = 0;          // total length of packet 数据包的总长度
  uint16_t id = 0;           // identification number 识别号码
  bool df = true;            // don't fragment flag 不分段标志
  bool mf = false;           // more fragments flag more fragments标志
  uint16_t offset = 0;       // fragment offset field 片段偏移字段
  uint8_t ttl = DEFAULT_TTL; // time to live field 
  uint8_t proto = PROTO_TCP; // protocol field 协议字段
  uint16_t cksum = 0;        // checksum field 校验和字段
  uint32_t src = 0;          // src address SRC地址
  uint32_t dst = 0;          // dst address DST地址

  // Length of the payload 负载长度
  uint16_t payload_length() const;

  // Pseudo-header's contribution to the TCP checksum 伪头文件对 TCP 校验和的贡献
  uint32_t pseudo_checksum() const;

  // Set checksum to correct value 将校验和设置为正确的值
  void compute_checksum();

  // Return a string containing a header in human-readable format 返回一个包含人类可读格式的头文件的字符串
  std::string to_string() const;

  void parse( Parser& parser ); //从字符串中解析。
  void serialize( Serializer& serializer ) const; //序列化IPv4Header(不重新计算校验和)
};
