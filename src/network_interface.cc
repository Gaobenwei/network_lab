#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

using namespace std;

// ethernet_address: Ethernet (what ARP calls "hardware") address of the interface
// ip_address: IP (what ARP calls "protocol") address of the interface
// ethernet_address：接口的以太网（ARP 称为 "硬件"）地址
// ip_address：接口的 IP（ARP 称为 "协议"）地址
NetworkInterface::NetworkInterface( const EthernetAddress& ethernet_address, const Address& ip_address )
  : ethernet_address_( ethernet_address ), ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ ) << " and IP address "
       << ip_address.ip() << "\n";
}

// dgram: the IPv4 datagram to be sent
// next_hop: the IP address of the interface to send it to (typically a router or default gateway, but
// may also be another host if directly connected to the same network as the destination)
// dgram:要发送的IPv4数据报
// next_hop:发送到的接口的IP地址(通常是路由器或默认网关，但如果直接连接到与目的地相同的网络，也可以是另一个主机)

// Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) by using the
// Address::ipv4_numeric() method.
// 注意：可以使用 // Address::ipv4_numeric() 方法将地址类型转换为 uint32_t（原始 32 位 IP 地址）。

void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  auto const addr_numeric=next_hop.ipv4_numeric();
  //ARP表存储了映射信息，我们直接发送数据报
  if(arp_table.contains(addr_numeric))
  {
    EthernetFrame frame;
    frame.header.src=ethernet_address_;
    frame.header.dst=arp_table.at(addr_numeric).eth_addr;
    frame.header.type=EthernetHeader::TYPE_IPv4;
    frame.payload=serialize(dgram);
    outbound_frames_.push(frame);  
  }
  else 
  {
    if(arp_requests_lefetime_.find(addr_numeric)==arp_requests_lefetime_.end())//ARP表没有这样的映射，且我们没有发送一个ARP请求的目标ip,即
    {
      //下一跳ipv4地址不在arp等待列表中
      ARPMessage arp_request;
      arp_request.opcode=ARPMessage::OPCODE_REQUEST;
      
      arp_request.sender_ethernet_address=ethernet_address_; //发送方以太网地址
      arp_request.sender_ip_address =ip_address_.ipv4_numeric();
      arp_request.target_ip_address=addr_numeric;
      arp_request.target_ethernet_address={};

      //组装一个arp帧
      EthernetFrame frame;
      frame.header.src=ethernet_address_;
      frame.header.dst=ETHERNET_BROADCAST;
      frame.header.type=EthernetHeader::TYPE_ARP;
      frame.payload=serialize(arp_request);
      outbound_frames_.push(frame);

      //将数据报加入arp等待列表
      arp_requests_lefetime_.emplace(std::make_pair(addr_numeric,ARP_REQUEST_TIMEOUT_MS));
    }
    //我们需要将数据报存储在列表中。知道eth地址后，就可以将相应的dgram排在队列中。
    arp_request_waiting_list_.emplace_back(std::pair<InternetDatagram,Address>(dgram,next_hop));
  }

}

// frame: the incoming Ethernet frame // 帧：传入的以太网帧
optional<InternetDatagram> NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  //  代码应忽略任何不以网络接口为目标的帧
  if(frame.header.dst!=ethernet_address_&&frame.header.dst!=ETHERNET_BROADCAST)
  {
    return nullopt;
  }
  /* IP datagrams */
  if(frame.header.type==EthernetHeader::TYPE_IPv4)
  {
    InternetDatagram dgram;
    if(parse(dgram,frame.payload)==true)
    {
      return dgram;
    }
    else
    {
      return nullopt;
    }
  }
  /* ARP datagrams */
  if(frame.header.type==EthernetHeader::TYPE_ARP)
  {
    ARPMessage msg;
    if(parse(msg,frame.payload)==false)
    {
      return nullopt;
    }

    //是否是请求我本身的ARP请求
    const bool is_arp_request=msg.opcode==ARPMessage::OPCODE_REQUEST&&msg.target_ip_address==ip_address_.ipv4_numeric();

    if(is_arp_request) //发送适当的ARP回复
    {
      ARPMessage arp_reply;
      arp_reply.opcode=ARPMessage::OPCODE_REPLY;
      arp_reply.sender_ethernet_address=ethernet_address_;
      arp_reply.sender_ip_address=ip_address_.ipv4_numeric();
      arp_reply.target_ethernet_address=msg.sender_ethernet_address;
      arp_reply.target_ip_address=msg.sender_ip_address;

      EthernetFrame frame1;
      frame1.header.src=ethernet_address_;
      frame1.header.dst=msg.sender_ethernet_address;
      frame1.header.type=EthernetHeader::TYPE_ARP;
      frame1.payload=serialize(arp_reply);
      outbound_frames_.push(frame1);
    }

    //是否是一个我需要的ARP响应回复
    const bool is_arp_response=msg.opcode==ARPMessage::OPCODE_REPLY&&msg.target_ip_address==ip_address_.ipv4_numeric();
    if(is_arp_request||is_arp_response)
    {
      arp_t ar;
      ar.eth_addr=msg.sender_ethernet_address;
      ar.ttl=ARP_TIMEOUT_MS;
      arp_table.emplace(std::make_pair(msg.sender_ip_address,ar));
      //检查arp等待列表中是否有数据报可以发送
      for(auto iter=arp_request_waiting_list_.begin();iter!=arp_request_waiting_list_.end();)
      {
        if(iter->second.ipv4_numeric()==msg.sender_ip_address)
        {
          send_datagram(iter->first,iter->second);
          iter=arp_request_waiting_list_.erase(iter);
        }
        else
        {
          iter++;
        }
      }
      // 收到了这个ARP消息，所以能够知道此消息的发送方的以太网地址和ip的映射，因此可以删除请求ip的ARP请求
      arp_requests_lefetime_.erase(msg.sender_ip_address);
    }
  }

  return nullopt;
}

// ms_since_last_tick: the number of milliseconds since the last call to this method 
// ms_since_last_tick: 上次调用此方法后的毫秒数
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  /*删除ARP表中过期的ARP项*/
  //修复:如果我们已经擦除当前iter的数据，不要使用'iter++' !
  for(auto iter=arp_table.begin();iter!=arp_table.end();)
  {
    if(iter->second.ttl<=ms_since_last_tick)
    {
      iter=arp_table.erase(iter);
    }
    else
    {
      iter->second.ttl-=ms_since_last_tick;
      iter++;
    }  
  }

  /*删除过期ARP请求*/
  for(auto iter=arp_requests_lefetime_.begin();iter!=arp_requests_lefetime_.end();++iter)
  {
    /*重新发送ARP请求，如果此请求已过期*/
    if(iter->second<=ms_since_last_tick)
    {
      ARPMessage msg;
      msg.opcode=ARPMessage::OPCODE_REQUEST;
      msg.sender_ethernet_address=ethernet_address_;
      msg.sender_ip_address=ip_address_.ipv4_numeric();
      msg.target_ip_address=iter->first;
      msg.target_ethernet_address={};

      EthernetFrame frame;
      frame.header.src=ethernet_address_;
      frame.header.dst=ETHERNET_BROADCAST;
      frame.header.type=EthernetHeader::TYPE_ARP;
      frame.payload=serialize(msg);
      outbound_frames_.push(frame);
      //重置该组件的ARP ttl
      iter->second=ARP_REQUEST_TIMEOUT_MS;
    }
    else
    {
      iter->second-=ms_since_last_tick;
    }
  }

}

optional<EthernetFrame> NetworkInterface::maybe_send()
{
  if(outbound_frames_.empty())
  {
    return nullopt;
  }
  else
  {
    EthernetFrame frame=outbound_frames_.front();
    outbound_frames_.pop();
    return frame;
  }
}
