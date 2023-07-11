#pragma once

#include "address.hh"
#include "file_descriptor.hh"

#include <cstdint>
#include <functional>
#include <sys/socket.h>

//! \brief Base class for network sockets (TCP, UDP, etc.)网络套接字的基类
//! \details Socket is generally used via a subclass. See TCPSocket and UDPSocket for usage examples. Socket通常通过子类使用
class Socket : public FileDescriptor
{
private:
  //! Get the local or peer address the socket is connected to 获取套接字所连接的本地或对端地址
  Address get_address( const std::string& name_of_function,
                       const std::function<int( int, sockaddr*, socklen_t* )>& function ) const;

protected:
  //! Construct via [socket(2)](\ref man2::socket) 通过[socket(2)](\ref man2::socket)构建
  Socket( int domain, int type, int protocol = 0 );

  //! Construct from a file descriptor. 从文件描述符构造。
  Socket( FileDescriptor&& fd, int domain, int type, int protocol = 0 );

  //! Wrapper around [getsockopt(2)](\ref man2::getsockopt) 围绕[getsockopt(2)](\ref man2::getsockopt)的封装程序
  template<typename option_type>
  socklen_t getsockopt( int level, int option, option_type& option_value ) const;

  //! Wrappers around [setsockopt(2)](\ref man2::setsockopt) 围绕[setockopt(2)](ref man2::setockopt)的包络器
  template<typename option_type>
  void setsockopt( int level, int option, const option_type& option_value );

  void setsockopt( int level, int option, std::string_view option_val );

public:
  //! Bind a socket to a specified address with [bind(2)](\ref man2::bind), usually for listen/accept 
  //使用[Bind (2)](\ref man2:: Bind)将套接字绑定到指定地址，通常用于listen/accept
  void bind( const Address& address );

  //! Bind a socket to a specified device 将一个套接字绑定到一个指定的设备上
  void bind_to_device( std::string_view device_name );

  //! Connect a socket to a specified peer address with [connect(2)](\ref man2::connect)
  //用[connect(2)](\ref man2::connect) 连接一个套接字到一个指定的对等地址。
  void connect( const Address& address );

  //! Shut down a socket via [shutdown(2)](\ref man2::shutdown)
  //通过[shutdown(2)]关闭一个套接字（\ref man2::shutdown）。
  void shutdown( int how );

  //! Get local address of socket with [getsockname(2)](\ref man2::getsockname) 获取套接字的本地地址
  Address local_address() const;
  //! Get peer address of socket with [getpeername(2)](\ref man2::getpeername)
  //使用[getpeername(2)]获取套接字的对端地址(\ref man2::getpeername)
  Address peer_address() const;

  //! Allow local address to be reused sooner via [SO_REUSEADDR](\ref man7::socket)
  //通过[SO_REUSEADDR](\ref man7::socket)允许本地地址更早地被重复使用。
  void set_reuseaddr();

  //! Check for errors (will be seen on non-blocking sockets) 检查错误（将在非阻塞套接字上看到）
  void throw_if_error() const;
};
//数据报套接字类型
class DatagramSocket : public Socket
{
  using Socket::Socket;

public:
  //! Receive a datagram and the Address of its sender
  //接收数据报及其发送者的地址
  void recv( Address& source_address, std::string& payload );

  //! Send a datagram to specified Address
  //发送一个数据报到指定的地址
  void sendto( const Address& destination, std::string_view payload );

  //! Send datagram to the socket's connected address (must call connect() first)
  //发送数据报到套接字的连接地址(必须先调用connect())
  void send( std::string_view payload );
};

//! A wrapper around [UDP sockets](\ref man7::udp)
class UDPSocket : public DatagramSocket
{
  //! \param[in] fd is the FileDescriptor from which to construct  fd是文件描述符，用来构建文件描述符。
  explicit UDPSocket( FileDescriptor&& fd ) : DatagramSocket( std::move( fd ), AF_INET, SOCK_DGRAM ) {}

public:
  //! Default: construct an unbound, unconnected UDP socket 默认值:构造一个未绑定、未连接的UDP套接字
  UDPSocket() : DatagramSocket( AF_INET, SOCK_DGRAM ) {}
};

//! A wrapper around [TCP sockets](\ref man7::tcp)
class TCPSocket : public Socket
{
private:
  //! \brief Construct from FileDescriptor (used by accept()) 来自FileDescriptor的简短构造(由accept()使用)
  //! \param[in] fd is the FileDescriptor from which to construct
  explicit TCPSocket( FileDescriptor&& fd ) : Socket( std::move( fd ), AF_INET, SOCK_STREAM ) {}

public:
  //! Default: construct an unbound, unconnected TCP socket
  TCPSocket() : Socket( AF_INET, SOCK_STREAM ) {}

  //! Mark a socket as listening for incoming connections 将套接字标记为侦听传入连接
  void listen( int backlog = 16 );

  //! Accept a new incoming connection 接受一个新传入的连接
  TCPSocket accept();
};

//! A wrapper around [packet sockets](\ref man7:packet)
class PacketSocket : public DatagramSocket
{
public:
  PacketSocket( const int type, const int protocol ) : DatagramSocket( AF_PACKET, type, protocol ) {}

  void set_promiscuous();
};
