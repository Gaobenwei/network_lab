#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <random>

using namespace std;

/* TCPSender constructor (uses a random ISN if none given) TCPSender构造函数(如果没有给出则使用随机ISN)*/
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) ), initial_RTO_ms_( initial_RTO_ms )
{}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  return outstanding_count_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return retransmissions_count_;
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  // Your code here.
  if(messages_.empty())
  {
    return {};
  }
  if(!timer_.is_running())
  {
    timer_.start();
  }
  TCPSenderMessage msg=messages_.front();
  messages_.pop();
  return msg;

}

void TCPSender::push( Reader& outbound_stream )
{
  // Your code here
  //可用窗口大小内不断发送段
  uint16_t win_now_size=win_size_==0?1:win_size_;
  while(outstanding_count_<win_now_size)
  {
    TCPSenderMessage msg;
    //起始部分发送SYN标志
    if(syn_==false)
    {
      syn_=msg.SYN=true;
      outstanding_count_+=1;
    }
    //段的序列号
    msg.seqno=Wrap32::wrap(next_seqno_,isn_);
    //段的内容string，合适的大小，从read流中产生
    auto str_size=std::min(TCPConfig::MAX_PAYLOAD_SIZE,win_now_size-outstanding_count_);
    read(outbound_stream,str_size,msg.payload);
    outstanding_count_+=msg.payload.size();
    //是否发送结束标志
    if(!fin_&&outbound_stream.is_finished()&&outstanding_count_<win_now_size)
    {
      fin_=msg.FIN=true;
      outstanding_count_+=1;
    }
    //空段则不用发送
    if(msg.sequence_length()==0)
    {
      break;
    }
    //更新sender记录的信息
    messages_.push(msg);
    //测试是否需要，push立刻发送？
    next_seqno_+=msg.sequence_length();
    outstanding_segments_.push(msg);

    //结束标志or流中数据无缓冲的数据
    if(msg.FIN||outbound_stream.bytes_buffered()==0)
    {
      break;
    }
  }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  // Your code here.
  TCPSenderMessage msg;
  msg.seqno=Wrap32::wrap(next_seqno_,isn_);
  msg.FIN=msg.SYN=false;
  return msg;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  win_size_=msg.window_size;
  //win_size_=msg.window_size==0?1:msg.window_size; 有错误
  if(msg.ackno.has_value())
  {
    auto new_ackno=msg.ackno.value().unwrap(isn_,next_seqno_);
    //测试是否有用
    if(new_ackno>next_seqno_)
    {
      return;
    }
    rece_seqno_=new_ackno;
    while(!outstanding_segments_.empty())
    {
      auto& front=outstanding_segments_.front();
      if(front.seqno.unwrap(isn_,next_seqno_)+front.sequence_length()<=rece_seqno_)
      {
        outstanding_count_-=front.sequence_length();
        outstanding_segments_.pop();
        timer_.reset_RTO();
        if(!outstanding_segments_.empty())
        {
          timer_.start();
        }
        retransmissions_count_=0;
      }
      else
      {
        break;
      }
    }
    if(outstanding_segments_.empty())
    {
      timer_.stop();
    }
  }
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  timer_.tick(ms_since_last_tick);
  if(timer_.is_expired())
  {
    messages_.push(outstanding_segments_.front());
    if(win_size_!=0)
    {
      retransmissions_count_+=1;
      timer_.double_RTO();
    }
    timer_.start();
  }
}
