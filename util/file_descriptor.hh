#pragma once

#include "buffer.hh"
#include <cstddef>
#include <limits>
#include <memory>
#include <vector>

// A reference-counted handle to a file descriptor文件描述符的引用计数句柄
class FileDescriptor
{
  // FDWrapper: A handle on a kernel file descriptor.内核文件描述符的句柄。
  // FileDescriptor objects contain a std::shared_ptr to a FDWrapper.对象包含一个std::shared_ptr到FDWrapper。
  class FDWrapper
  {
  public:
    int fd_;                    // The file descriptor number returned by the kernel内核返回的文件描述符号
    bool eof_ = false;          // Flag indicating whether FDWrapper::fd_ is at EOF指示FDWrapper::fd_是否处于EOF的标志
    bool closed_ = false;       // Flag indicating whether FDWrapper::fd_ has been closed FDWrapper::fd_是否已关闭的标志
    bool non_blocking_ = false; // Flag indicating whether FDWrapper::fd_ is non-blocking FDWrapper::fd_是否阻塞的标志
    unsigned read_count_ = 0;   // The number of times FDWrapper::fd_ has been read 读取FDWrapper::fd_的次数
    unsigned write_count_ = 0;  // The numberof times FDWrapper::fd_ has been written FDWrapper::fd_被写入的次数

    // Construct from a file descriptor number returned by the kernel
    explicit FDWrapper( int fd );
    // Closes the file descriptor upon destruction
    ~FDWrapper();
    // Calls [close(2)](\ref man2::close) on FDWrapper::fd_
    void close();

    template<typename T>
    T CheckSystemCall( std::string_view s_attempt, T return_value ) const;

    // An FDWrapper cannot be copied or moved FDWrapper不能被复制或移动
    FDWrapper( const FDWrapper& other ) = delete;
    FDWrapper& operator=( const FDWrapper& other ) = delete;
    FDWrapper( FDWrapper&& other ) = delete;
    FDWrapper& operator=( FDWrapper&& other ) = delete;
  };

  // A reference-counted handle to a shared FDWrapper 共享FDWrapper的引用计数句柄
  std::shared_ptr<FDWrapper> internal_fd_;

  // private constructor used to duplicate the FileDescriptor (increase the reference count) 用于复制FileDescriptor的私有构造函数(增加引用计数)
  explicit FileDescriptor( std::shared_ptr<FDWrapper> other_shared_ptr );

protected:
  // size of buffer to allocate for read() 为read()分配的缓冲区大小
  static constexpr size_t kReadBufferSize = 16384;

  void set_eof() { internal_fd_->eof_ = true; }
  void register_read() { ++internal_fd_->read_count_; }   // increment read count
  void register_write() { ++internal_fd_->write_count_; } // increment write count 增量写计数

  template<typename T>
  T CheckSystemCall( std::string_view s_attempt, T return_value ) const;  // check return value of system call 检查系统调用的返回值

public:
  // Construct from a file descriptor number returned by the kernel 从内核返回的文件描述符号构造
  explicit FileDescriptor( int fd );

  // Free the std::shared_ptr; the FDWrapper destructor calls close() when the refcount goes to zero.
  //释放std::shared_ptr;当refcount变为0时，FDWrapper析构函数调用close()。
  ~FileDescriptor() = default;

  // Read into `buffer`
  void read( std::string& buffer );
  void read( std::vector<std::string>& buffers );

  // Attempt to write a buffer 写缓冲区
  // returns number of bytes written 返回写入的字节数
  size_t write( std::string_view buffer );
  size_t write( const std::vector<std::string_view>& buffers );
  size_t write( const std::vector<Buffer>& buffers );

  // Close the underlying file descriptor 关闭底层文件描述符
  void close() { internal_fd_->close(); }

  // Copy a FileDescriptor explicitly, increasing the FDWrapper refcount 显式复制一个FileDescriptor，增加FDWrapper的refcount
  FileDescriptor duplicate() const;

  // Set blocking(true) or non-blocking(false) 设置阻塞(true)或非阻塞(false)
  void set_blocking( bool blocking );

  // Size of file
  off_t size() const;

  // FDWrapper accessors 访问器
  int fd_num() const { return internal_fd_->fd_; }                        // underlying descriptor number 底层描述符号
  bool eof() const { return internal_fd_->eof_; }                         // EOF flag state
  bool closed() const { return internal_fd_->closed_; }                   // closed flag state
  unsigned int read_count() const { return internal_fd_->read_count_; }   // number of reads
  unsigned int write_count() const { return internal_fd_->write_count_; } // number of writes

  // Copy/move constructor/assignment operators
  // FileDescriptor can be moved, but cannot be copied implicitly (see duplicate())
  //复制/移动构造函数/赋值操作符
  // FileDescriptor可以移动，但不能隐式复制(参见duplicate())
  FileDescriptor( const FileDescriptor& other ) = delete;            // copy construction is forbidden
  FileDescriptor& operator=( const FileDescriptor& other ) = delete; // copy assignment is forbidden
  FileDescriptor( FileDescriptor&& other ) = default;                // move construction is allowed
  FileDescriptor& operator=( FileDescriptor&& other ) = default;     // move assignment is allowed
};
