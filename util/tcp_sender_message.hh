#pragma once

#include "buffer.hh"
#include "wrapping_integers.hh"

#include <string>

/*
 * The TCPSenderMessage structure contains the information sent from a TCP sender to its receiver.
 *
 * It contains four fields:
 *
 * 1) The sequence number (seqno) of the beginning of the segment. If the SYN flag is set, this is the
 *    sequence number of the SYN flag. Otherwise, it's the sequence number of the beginning of the payload.
 *
 * 2) The SYN flag. If set, it means this segment is the beginning of the byte stream, and that
 *    the seqno field contains the Initial Sequence Number (ISN) -- the zero point.
 *
 * 3) The payload: a substring (possibly empty) of the byte stream.
 *
 * 4) The FIN flag. If set, it means the payload represents the ending of the byte stream.
 */

/*tcpsendmessage结构包含从TCP发送方发送到接收方的信息。它包含四个字段:
1）段开头的序号(seqno)。如果设置了SYN标志，则表示SYN标志的序列号。否则，它就是负载开头的序列号。

2）SYN标志。如果设置，则意味着该段是字节流的开始，并且seqno字段包含初始序列号(ISN)——零点。
3）负载:字节流的子字符串(可能为空)。

4）FIN旗。如果设置，则表示有效负载表示字节流的结束。*/

struct TCPSenderMessage
{
  Wrap32 seqno { 0 };
  bool SYN { false };
  Buffer payload {};
  bool FIN { false };

  // How many sequence numbers does this segment use? 这个片段使用了多少个序列号?
  size_t sequence_length() const { return SYN + payload.size() + FIN; }
};
