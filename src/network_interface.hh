#pragma once

#include "address.hh"
#include "ethernet_frame.hh"
#include "ipv4_datagram.hh"

#include <iostream>
#include <list>
#include <optional>
#include <queue>
#include <unordered_map>
#include <utility>

// A "network interface" that connects IP (the internet layer, or network layer)
// with Ethernet (the network access layer, or link layer).
//连接IP(因特网层，或网络层)和以太网(网络接入层，或链路层)的“网络接口”。

// This module is the lowest layer of a TCP/IP stack
// (connecting IP with the lower-layer network protocol,
// e.g. Ethernet). But the same module is also used repeatedly
// as part of a router: a router generally has many network
// interfaces, and the router's job is to route Internet datagrams
// between the different interfaces.

/*该模块是TCP/IP栈的最底层(将IP与较底层的网络协议连接起来，例如以太网)。
但同样的模块也被反复用作路由器的一部分:路由器通常有许多网络接口，
路由器的工作是在不同的接口之间路由互联网数据报。
*/



// The network interface translates datagrams (coming from the
// "customer," e.g. a TCP/IP stack or router) into Ethernet
// frames. To fill in the Ethernet destination address, it looks up
// the Ethernet address of the next IP hop of each datagram, making
// requests with the [Address Resolution Protocol](\ref rfc::rfc826).
// In the opposite direction, the network interface accepts Ethernet
// frames, checks if they are intended for it, and if so, processes
// the the payload depending on its type. If it's an IPv4 datagram,
// the network interface passes it up the stack. If it's an ARP
// request or reply, the network interface processes the frame
// and learns or replies as necessary.
/*网络接口将数据报(来自“客户”，例如TCP/IP栈或路由器)转换成以太网帧。
为了填充以太网目的地址，它查找每个数据报的下一个IP跳的以太网地址，
并使用[地址解析协议](\ refrfc::rfc826)发出请求。在相反的方向上，
网络接口接受以太网帧，检查它们是否为它准备的，如果是，则根据其类型处理负载。
如果它是IPv4数据报，网络接口将它向上传递堆栈。如果是ARP请求或应答，
则网络接口处理该帧，并根据需要学习或应答。*/
class NetworkInterface
{
private:
  // Ethernet (known as hardware, network-access, or link-layer) address of the interface
  // 以太网(称为硬件、网络访问或链路层)的接口地址
  EthernetAddress ethernet_address_;

  // IP (known as Internet-layer or network-layer) address of the interface
  //接口的IP (internet层或网络层)地址
  Address ip_address_;
  
  //等待传输的以太网帧的队列
  std::queue<EthernetFrame> outbound_frames_ {};

  //ARP 最多存储 30 秒，这样可以减少 ARP 表的长度
  const size_t ARP_TIMEOUT_MS = 30000;
  //间隔 5 秒再发送相同的 ARP 请求  
  const size_t ARP_REQUEST_TIMEOUT_MS = 5*1000;
  using arp_t=struct
  {
    EthernetAddress eth_addr; //以太网地址
    size_t ttl; //存活时间
  };

  std::unordered_map<uint32_t,arp_t> arp_table {}; //ARP 表
  std::list<std::pair<InternetDatagram,Address>> arp_request_waiting_list_ {}; //未发送的数据报，因为不知道下一跳的以太网地址
  std::unordered_map<uint32_t,size_t> arp_requests_lefetime_ {}; //需要一个 unordered_map 数据结构存储 numeric IP 与该条 ARP 请求的声明周期的映射 arp_requests_lifetime_。


public:
  // Construct a network interface with given Ethernet (network-access-layer) and IP (internet-layer)
  // addresses
  //用给定的以太网(网络访问层)和IP(因特网层)地址构造一个网络接口
  NetworkInterface( const EthernetAddress& ethernet_address, const Address& ip_address );

  // Access queue of Ethernet frames awaiting transmission
  //等待传输的以太网帧的访问队列
  std::optional<EthernetFrame> maybe_send();

  // Sends an IPv4 datagram, encapsulated in an Ethernet frame (if it knows the Ethernet destination
  // address). Will need to use [ARP](\ref rfc::rfc826) to look up the Ethernet destination address
  // for the next hop.
  // ("Sending" is accomplished by making sure maybe_send() will release the frame when next called,
  // but please consider the frame sent as soon as it is generated.)
  /*//发送一个封装在以太网帧中的IPv4数据报(如果它知道以太网的目的地址)。需要使用[ARP](\ref rfc::rfc826)来查找下一跳的以太网目的地址。
  (“发送”是通过确保maybe_send()在下一次调用时释放帧来完成的，但请考虑在生成帧时立即发送帧。)*/

  void send_datagram( const InternetDatagram& dgram, const Address& next_hop );

  // Receives an Ethernet frame and responds appropriately.
  // If type is IPv4, returns the datagram.
  // If type is ARP request, learn a mapping from the "sender" fields, and send an ARP reply.
  // If type is ARP reply, learn a mapping from the "sender" fields.

  //接收一个以太网帧并做出相应的响应。
  //如果类型为IPv4，则返回数据报。
  //如果类型为ARP请求，从“sender”字段中学习映射，然后发送ARP应答。
  //如果类型为ARP应答，从“sender”字段学习映射。

  std::optional<InternetDatagram> recv_frame( const EthernetFrame& frame );

  // Called periodically when time elapses 当时间流逝时周期性调用
  void tick( size_t ms_since_last_tick );
};
