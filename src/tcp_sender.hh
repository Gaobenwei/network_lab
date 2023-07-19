#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"


class Timer
{
private:
  //初始RTO,当时RTO
  uint64_t initial_RTO_ ;
  uint64_t now_RTO_ ;
  //计时
  uint64_t time_ {0};
  //是否运行
  bool is_running_ {false};
public:
  explicit Timer(uint64_t rto):initial_RTO_(rto),now_RTO_(rto){}
  //启动，停止
  void start()
  {
    is_running_=true;
    time_=0;
  }

  void stop()
  {
    is_running_=false;
  }

  //是否运行，超时
  bool is_running()const{return is_running_;}
  bool is_expired()const
  {
    return (is_running_&& (time_>=now_RTO_));
  }

  //now_RTO_ 变化
  void double_RTO()
  {
    now_RTO_*=2;
  }

  void reset_RTO()
  {
    now_RTO_=initial_RTO_;
  }

  //计时
  void tick(const uint64_t ms_since_last_tick)
  {
    if(is_running_)
    {
      time_+=ms_since_last_tick;
    }
  }
};

class TCPSender
{
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;
  //SYN包是否已发送
  bool syn_ {false};
  //FIN包是否已发送
  bool fin_ {false};
 
  //连续重传计数
  uint64_t retransmissions_count_ {0};
  
  //接收确认信息知道的窗口大小、确认号
  uint16_t win_size_ {1};
  
  //根据ack等调整的已确认序列号 [0,1,2,......,rece_seqno_-1]已被确认，rece_seqno_还未被确认
  uint64_t rece_seqno_ {0};
  
  //下一个要发送的数据索引,[0,1,2....next_seqno_-1]都已发送，next_seqno_是下一个即将发送的
  uint64_t next_seqno_ {0};
  
  //已发送未确认的数据大小。sequence_numbers_in_flight() 函数需要返回这个数值
  uint64_t outstanding_count_ {0};
  
  //跟踪已发送的但是没有收到确认的报文
  std::queue<TCPSenderMessage> outstanding_segments_ {};
  //要发送的报文段
  std::queue<TCPSenderMessage> messages_ {};

  //重传定时器的类，记录一个segment是否超时
  Timer timer_{initial_RTO_ms_};

public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN 
  用给定的默认重传超时和可能的ISN构造TCP发送方*/
  TCPSender( uint64_t initial_RTO_ms, std::optional<Wrap32> fixed_isn );

  /* Push bytes from the outbound stream 从出站流推送字节*/
  void push( Reader& outbound_stream );

  /* Send a TCPSenderMessage if needed (or empty optional otherwise) 如果需要，发送tcpsendmessage(否则为空可选)*/
  std::optional<TCPSenderMessage> maybe_send();

  /* Generate an empty TCPSenderMessage 生成空tcpsendmessage*/
  TCPSenderMessage send_empty_message() const;

  /* Receive an act on a TCPReceiverMessage from the peer's receiver 
  接收对端接收方发送的TCPReceiverMessage的动作*/
  void receive( const TCPReceiverMessage& msg );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called. 
  自上次调用tick()方法以来，时间已经过给定的毫秒数。*/
  void tick( uint64_t ms_since_last_tick );

  /* Accessors for use in testing 用于测试的访问器*/
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding? 有多少序列号是未完成的?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened? 发生了多少次连续的重传?
};

