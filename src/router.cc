#include "router.hh"

#include <iostream>
#include <limits>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
// route_prefix:匹配数据报目的地址的“32位”IPv4地址前缀
// prefix_length:对于这条路由是适用的，多少高阶(最高有效)的route_prefix将需要匹配数据报的目的地址的相应位?
// next_hop:下一跳IP地址。如果网络直接连接到路由器(在这种情况下，下一跳地址应该是数据报的最终目的地)，则为空。
// interface_num:发送数据报的接口索引。
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";

  routing_table_.emplace_back(route_t{route_prefix,prefix_length,next_hop,interface_num});
}

void Router::route() 
{
  /*在接口之间路由报文。对于每个接口，使用maybe_receive()方法来使用每个传入的数据报，
  并将其在其中一个接口上发送到正确的下一跳。
  路由器根据与数据报目的地址匹配的前缀长度最长的路由选择出接口和下一跳。*/
  for(auto &net_interface:interfaces_)
  {
    auto maybe_datagram=net_interface.maybe_receive(); 
    if(maybe_datagram.has_value())
    {
      InternetDatagram datagram=maybe_datagram.value();
      auto dst_ip_addr=datagram.header.dst;
      //寻找最长前缀匹配的路由,先将最长标记置为结尾
      auto longest_match=routing_table_.end();
      for(auto iter=routing_table_.begin();iter!=routing_table_.end();++iter)
      {
        if(iter->prefix_length_==0||(iter->route_prefix_ ^ dst_ip_addr) >> (32 - iter->prefix_length_) ==0)
        {
          //更新最长前缀匹配路由
          if(longest_match==routing_table_.end()||iter->prefix_length_>longest_match->prefix_length_)
          {
            longest_match=iter;
          }
        }
      }

      //检查传入数据报的合法性，主要是检查TTL是否为0
      auto &ttll=datagram.header.ttl;
      if(longest_match!=routing_table_.end()&&ttll-- > 1)
      {
        //更新数据报的校验和,修改发送的地址
        datagram.header.compute_checksum();
        const auto &next_hop_addr=longest_match->next_hop_.has_value()?longest_match->next_hop_.value():Address::from_ipv4_numeric(dst_ip_addr);
        //发送数据报
        auto &outbound_interface=interface(longest_match->interface_num_);
        outbound_interface.send_datagram(datagram,next_hop_addr);
      }

    }
  }
}

