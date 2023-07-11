#pragma once

#include <queue>
#include <stdexcept>
#include <string>
#include <string_view>

class Reader;
class Writer;

class ByteStream
{
protected:
  uint64_t capacity_; // 字节流在任何时候可以持有的最大字节数
  // Please add any additional state to the ByteStream here, and not to the Writer and Reader interfaces.
  //请在此向ByteStream添加任何附加状态，而不要向Writer和Reader接口添加。
  //提示:这根本不需要复杂的数据结构，但如果任何测试花费的时间超过一秒，那就表明您可能希望继续探索不同的方法。
  std::queue<std::string> data_queue_ {};
  std::queue<std::string_view> view_queue_ {};
  bool is_closed_{ false };
  bool has_error_{ false };
  uint64_t bytes_pushed_{ 0 }; // 已经推送的字节数
  uint64_t bytes_popped_{ 0 }; // 已经弹出的字节数
  uint64_t bytes_buffered_{ 0 }; // 缓冲区中的字节数

public:
  explicit ByteStream( uint64_t capacity );

  // Helper functions (provided) to access the ByteStream's Reader and Writer interfaces
  //访问ByteStream的Reader和Writer接口的辅助函数
  Reader& reader();
  const Reader& reader() const;
  Writer& writer();
  const Writer& writer() const;
};

//以下是作家的界面
class Writer : public ByteStream
{
public:
  void push( std::string data ); // Push data to stream, but only as much as available capacity allows.向数据流推送数据，但只能在可用容量允许的范围内推送。

  void close();     // Signal that the stream has reached its ending. Nothing more will be written. 信号流结束。将不再写入任何内容。
  void set_error(); // Signal that the stream suffered an error. 流发生错误的信号。

  bool is_closed() const;              // Has the stream been closed? 流是否已关闭？
  uint64_t available_capacity() const; // How many bytes can be pushed to the stream right now? 现在可以向流推送多少字节？
  uint64_t bytes_pushed() const;       // Total number of bytes cumulatively pushed to the stream 累计推送到流中的字节数
};

//这里是为读者提供的界面
class Reader : public ByteStream
{
public:
  std::string_view peek() const; // Peek at the next bytes in the buffer (but don't pop them) 查看缓冲区中的下一个字节（但不要弹出它们）
  void pop( uint64_t len );      // Remove `len` bytes from the buffer 从缓冲区中删除“len”字节

  bool is_finished() const; // Is the stream finished (closed and fully popped)? 流是否完成（关闭并完全弹出）？
  bool has_error() const;   // Has the stream had an error? 流是否有错误？

  uint64_t bytes_buffered() const; // Number of bytes currently buffered (pushed and not popped) 当前缓冲的字节数（已推送而不弹出）
  uint64_t bytes_popped() const;   // Total number of bytes cumulatively popped from stream 从流中累计弹出的字节数
};

/*
 * read: A (provided) helper function thats peeks and pops up to `len` bytes 
 * from a ByteStream Reader into a string;
 * read:一个(提供的)辅助函数，从ByteStream Reader读取并弹出' len '字节到字符串;
 */
void read( Reader& reader, uint64_t len, std::string& out );
