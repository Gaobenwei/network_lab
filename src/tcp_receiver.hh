#pragma once

#include "reassembler.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

class TCPReceiver
{
public:
  std::optional<Wrap32> isn {}; //用来记录零点
  /*
   * The TCPReceiver receives TCPSenderMessages, inserting their payload into the Reassembler
   * at the correct stream index.
   * 
   * TCPReceiver接收tcpsendmessages，在正确的流索引处插入有效载荷到Reassembler中。
   */
  void receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream );

  /* The TCPReceiver sends TCPReceiverMessages back to the TCPSender.  TCPReceiver将tcppreceivermessages发送回TCPSender。*/
  TCPReceiverMessage send( const Writer& inbound_stream ) const;
};
