#pragma once

#include "network_interface.hh"

#include <optional>
#include <queue>

// A wrapper for NetworkInterface that makes the host-side
// interface asynchronous: instead of returning received datagrams
// immediately (from the `recv_frame` method), it stores them for
// later retrieval. Otherwise, behaves identically to the underlying
// implementation of NetworkInterface.
/* NetworkInterface的包装器，使主机端接口异步:
不是立即返回接收到的数据报(从' recv_frame '方法)，
而是存储它们以供以后检索。
否则，其行为与NetworkInterface的底层实现相同。*/
class AsyncNetworkInterface : public NetworkInterface
{
  std::queue<InternetDatagram> datagrams_in_ {};

public:
  using NetworkInterface::NetworkInterface;

  // Construct from a NetworkInterface 从网络接口构造
  explicit AsyncNetworkInterface( NetworkInterface&& interface ) : NetworkInterface( interface ) {}

  // \brief Receives and Ethernet frame and responds appropriately. 接收一个以太网帧并做出相应的响应。

  // - If type is IPv4, pushes to the `datagrams_out` queue for later retrieval by the owner.
  // - If type is ARP request, learn a mapping from the "sender" fields, and send an ARP reply.
  // - If type is ARP reply, learn a mapping from the "target" fields.
  //
  // \param[in] frame the incoming Ethernet frame

  // -如果type为IPv4，则将数据推送到' datagrams_out '队列，供所有者稍后检索。
  // -如果类型为ARP请求，从“sender”字段中学习映射，然后发送ARP应答。
  // -如果type为ARP reply，从target字段学习映射。
  //
  // \param[in] frame传入的以太网帧

  void recv_frame( const EthernetFrame& frame )
  {
    auto optional_dgram = NetworkInterface::recv_frame( frame );
    if ( optional_dgram.has_value() ) {
      datagrams_in_.push( std::move( optional_dgram.value() ) );
    }
  };

  // Access queue of Internet datagrams that have been received //接收到的Internet数据报的访问队列
  std::optional<InternetDatagram> maybe_receive()
  {
    if ( datagrams_in_.empty() ) {
      return {};
    }

    InternetDatagram datagram = std::move( datagrams_in_.front() );
    datagrams_in_.pop();
    return datagram;
  }
};

// A router that has multiple network interfaces and
// performs longest-prefix-match routing between them.
//有多个网络接口的路由器
//在它们之间执行最长前缀匹配路由。
class Router
{
  // The router's collection of network interfaces 路由器的网络接口集合
  std::vector<AsyncNetworkInterface> interfaces_ {};

  using route_t=struct 
  {
    /* data */
    uint32_t route_prefix_;
    uint8_t prefix_length_;
    std::optional<Address> next_hop_;
    size_t interface_num_;
  };

  std::list<route_t> routing_table_ {}; //路由表  
public:
  // Add an interface to the router
  // interface: an already-constructed network interface
  // returns the index of the interface after it has been added to the router
  // 将接口添加到路由器接口：已构建的网络接口在添加到路由器后，返回接口的索引
  size_t add_interface( AsyncNetworkInterface&& interface )
  {
    interfaces_.push_back( std::move( interface ) );
    return interfaces_.size() - 1;
  }

  // Access an interface by index 通过索引访问接口
  AsyncNetworkInterface& interface( size_t N ) { return interfaces_.at( N ); }

  // Add a route (a forwarding rule) 添加路由(转发规则)
  void add_route( uint32_t route_prefix,
                  uint8_t prefix_length,
                  std::optional<Address> next_hop,
                  size_t interface_num );

  // Route packets between the interfaces. For each interface, use the
  // maybe_receive() method to consume every incoming datagram and
  // send it on one of interfaces to the correct next hop. The router
  // chooses the outbound interface and next-hop as specified by the
  // route with the longest prefix_length that matches the datagram's
  // destination address.
  /*在接口之间路由报文。对于每个接口，使用maybe_receive()方法来使用每个传入的数据报，
  并将其在其中一个接口上发送到正确的下一跳。
  路由器根据与数据报目的地址匹配的前缀长度最长的路由选择出接口和下一跳。*/
  void route();
};
