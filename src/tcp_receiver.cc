#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  // Your code here.
  // 确认号(ackno): TCP接收方需要的下一个序列号。这是一个可选字段，如果tcreceiver还没有接收到初始序列号，则该字段为空。
  if(!isn.has_value()) //为空，代表着未收到包，理应收到开始的包,用来记录零点
  {
    if(!message.SYN)//但是如果收到的不是开始包，返回
    {
      return;
    }
    isn=message.seqno;
  }
  /*要获取first_index(流索引)，我们需要
* 1。转换消息。序列到绝对序列
* 2。将绝对序列转换为流索引
在TCP实现中，将使用第一个未组装索引作为检查点*/
  //auto const seqno=message.seqno;
  auto const checkpoint=inbound_stream.bytes_pushed();
  auto const abs_seqno=message.seqno.unwrap(isn.value(),checkpoint);
  auto const first_index=message.SYN?0:abs_seqno-1;

  reassembler.insert(first_index,message.payload.release(),message.FIN,inbound_stream);

}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  // Your code here.
  TCPReceiverMessage resc;
  //计算窗口大小
  auto const win_size=inbound_stream.available_capacity();
  resc.window_size=win_size<UINT16_MAX?win_size:UINT16_MAX;
  //计算确认号，期望收到的序列号
  if(isn.has_value())
  {
    auto const a=inbound_stream.bytes_pushed();
    auto const abs_se=a+1+inbound_stream.is_closed();  //注意关注返回消息是否包括结束标志，
    resc.ackno=Wrap32::wrap(abs_se,isn.value());
  }
  // auto const a=inbound_stream.bytes_pushed();
  //    auto const abs_se=a+1+inbound_stream.is_closed();  //注意关注返回消息是否包括结束标志，
  // resc.ackno=Wrap32::wrap(abs_se,isn.value());


  return resc;
}
