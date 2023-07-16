#pragma once

#include "wrapping_integers.hh"

#include <optional>

/*
 * The TCPReceiverMessage structure contains the information sent from a TCP receiver to its sender.
 *
 * It contains two fields:
 *
 * 1) The acknowledgment number (ackno): the *next* sequence number needed by the TCP Receiver.
 *    This is an optional field that is empty if the TCPReceiver hasn't yet received the Initial Sequence Number.
 *
 * 2) The window size. This is the number of sequence numbers that the TCP receiver is interested
 *    to receive, starting from the ackno if present. The maximum value is 65,535 (UINT16_MAX from
 *    the <cstdint> header).
 */

/*TCPReceiverMessage结构包含了从TCP接收方发送到发送方的信息。
*包含两个字段:

* 1)确认号(ackno): TCP接收方需要的下一个序列号。这是一个可选字段，如果tcreceiver还没有接收到初始序列号，则该字段为空。
* 2)窗口大小。这是TCP接收方有兴趣接收的序列号的数量，从存在的确认号开始。最大值是65,535(头部的UINT16_MAX)。*/

struct TCPReceiverMessage
{
  std::optional<Wrap32> ackno {};
  uint16_t window_size {};
};
