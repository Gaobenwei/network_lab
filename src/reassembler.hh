#pragma once

#include "byte_stream.hh"

#include <string>
#include <list>
class Reassembler
{

  private:
  // Your private member declarations here.
  //在这里声明您的私有成员。  
  // ///uint64_t first_unprocessed_index {0}; //第一个未pop的字节的索引
  uint64_t first_unassembled_index {0}; //期望的下一个字节的索引，此索引之前的字节已经组装好了
  std::list<std::pair<uint64_t,std::string>> buffer_{};
  // //std::map<uint64_t,std::string> buffer_map{};
  // //uint64_t first_unacceptable_index {0}; //第一个不可接受的字节的索引
  // //uint64_t capacity_{0}; //流的容量
  // //uint64_t available_capacity_ {0}; //流的可用容量
  uint64_t buffer_size_ {0}; //缓冲区的大小
  bool is_last_if {false};
  void insert_into_buffer(uint64_t first_index,std::string&& data,bool is_last_substring); //将子字符串插入缓冲区,有效但之前字节未接收的子字符串
  void flush_buffer(Writer& output); //将缓冲区中的字节pop


public:

  /*
  插入一个新的子字符串被重新组装成字节流。
  * ' first_index ':子字符串的第一个字节的索引
  * ' data ':子字符串本身
  * ' is_last_substring ':此子字符串表示流的结束
  * ' output ':对Writer的可变引用
  这四个参数都是调用insert()时提供的。在类Reassemble中,不用存储这些参数，因为它们只是用于调用insert()的一次。
  Reassembler的工作是将索引的子字符串(可能是乱序的，也可能是重叠的)重新组装回原始字节流。
  一旦Reassembler学习到流中的下一个字节，它就应该将其写入输出。
  如果Reassembler了解到适合流可用容量的字节，但还不能写入(因为之前的字节仍然未知)，它应该将它们存储在内部，直到空白被填满。
  Reassembler应该丢弃超出流可用容量的任何字节(即，即使早先的空白被填充也无法写入的字节)。
  重装程序应在写入最后一个字节后关闭数据流。
  */
  /*
   * Insert a new substring to be reassembled into a ByteStream.
   *   `first_index`: the index of the first byte of the substring
   *   `data`: the substring itself
   *   `is_last_substring`: this substring represents the end of the stream
   *   `output`: a mutable reference to the Writer
   *
   * The Reassembler's job is to reassemble the indexed substrings (possibly out-of-order
   * and possibly overlapping) back into the original ByteStream. As soon as the Reassembler
   * learns the next byte in the stream, it should write it to the output.
   *
   * If the Reassembler learns about bytes that fit within the stream's available capacity
   * but can't yet be written (because earlier bytes remain unknown), it should store them
   * internally until the gaps are filled in.
   *
   * The Reassembler should discard any bytes that lie beyond the stream's available capacity
   * (i.e., bytes that couldn't be written even if earlier gaps get filled in).
   *
   * The Reassembler should close the stream after writing the last byte.
   */
  void insert( uint64_t first_index, std::string data, bool is_last_substring, Writer& output );

  // How many bytes are stored in the Reassembler itself?
  uint64_t bytes_pending() const;
};
