#pragma once

#include <cstddef>
#include <cstdint>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <utility>

//! Wrapper around [IPv4 addresses](@ref man7::ip) and DNS operations.
//围绕[IPv4地址](@ref man7::ip)和DNS操作的包装器。
class Address
{
public:
  //! \brief Wrapper around [sockaddr_storage](@ref man7::socket). 简要介绍 [sockaddr_storage](@ref man7::socket) 的封装器。
  //! \details A `sockaddr_storage` is enough space to store any socket address (IPv4 or IPv6). 一个`sockaddr_storage`有足够的空间来存储任何套接字地址（IPv4或IPv6）。
  class Raw
  {
  public:
    sockaddr_storage storage {}; //!< The wrapped struct itself. 被包装的结构本身。
    // NOLINTBEGIN (*-explicit-*)
    operator sockaddr*();
    operator const sockaddr*() const;
    // NOLINTEND (*-explicit-*)
  };

private:
  socklen_t _size; //!< Size of the wrapped address. 包装地址的大小
  Raw _address {}; //!< A wrapped [sockaddr_storage](@ref man7::socket) containing the address. 一个包装好的[sockaddr_storage](@ref man7::socket)，包含地址。

  //! Constructor from ip/host, service/port, and hints to the resolver. 从ip/host、service/port构造函数，并提示解析器。
  Address( const std::string& node, const std::string& service, const addrinfo& hints );

public:
  //! Construct by resolving a hostname and servicename. 通过解析一个主机名和服务名来构建。
  Address( const std::string& hostname, const std::string& service );

  //! Construct from dotted-quad string ("18.243.0.1") and numeric port. 由点阵字符串（"18.243.0.1"）和数字端口构成。
  explicit Address( const std::string& ip, std::uint16_t port = 0 );

  //! Construct from a [sockaddr *](@ref man7::socket). 从一个[sockaddr *](@ref man7::socket)构建。
  Address( const sockaddr* addr, std::size_t size );

  //! Equality comparison.
  bool operator==( const Address& other ) const;
  bool operator!=( const Address& other ) const { return not operator==( other ); }

  //! \name Conversions 转换
  //!@{

  //! Dotted-quad IP address string ("18.243.0.1") and numeric port. 点阵式IP地址字符串（"18.243.0.1"）和数字式端口。
  std::pair<std::string, uint16_t> ip_port() const;
  //! Dotted-quad IP address string ("18.243.0.1").
  std::string ip() const { return ip_port().first; }
  //! Numeric port (host byte order). 数字端口(主机字节顺序)。
  uint16_t port() const { return ip_port().second; }
  //! Numeric IP address as an integer (i.e., in [host byte order](\ref man3::byteorder)).
  //数字IP地址作为一个整数(即，在[主机字节顺序](\ref man3::byteorder)))。
  uint32_t ipv4_numeric() const;
  //! Create an Address from a 32-bit raw numeric IP address 从一个32位原始数字IP地址创建一个地址
  static Address from_ipv4_numeric( uint32_t ip_address );
  //! Human-readable string, e.g., "8.8.8.8:53". 人类可读的字符串，例如"8.8.8.8:53"。
  std::string to_string() const;
  //!@}

  //! \name Low-level operations
  //!@{

  //! Size of the underlying address storage. 底层地址存储的大小。
  socklen_t size() const { return _size; }
  //! Const pointer to the underlying socket address storage. 指向底层套接字地址存储的Const指针。
  operator const sockaddr*() const { return static_cast<const sockaddr*>( _address ); } // NOLINT(*-explicit-*)
  //! Safely convert to underlying sockaddr type 安全地转换为底层sockaddr类型
  template<typename sockaddr_type>
  const sockaddr_type* as() const;

  //!@}
};
