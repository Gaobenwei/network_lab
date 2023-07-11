#include "byte_stream.hh"

#include <cstdint>
#include <stdexcept>

/*
 * read: A helper function thats peeks and pops up to `len` bytes
 * from a ByteStream Reader into a string;
 * read:一个(提供的)辅助函数，从ByteStream Reader读取并弹出' len '字节到字符串;
 */
void read( Reader& reader, uint64_t len, std::string& out )
{
  out.clear();

  while ( reader.bytes_buffered() and out.size() < len ) // 循环直到读取到len个字节
  {
    auto view = reader.peek();

    if ( view.empty() ) {
      throw std::runtime_error( "Reader::peek() returned empty string_view" );
    }

    view = view.substr( 0, len - out.size() ); // Don't return more bytes than desired. // 不要返回比所需的更多的字节。
    out += view;
    reader.pop( view.size() );
  }
}

Reader& ByteStream::reader()
{
  static_assert( sizeof( Reader ) == sizeof( ByteStream ),
                 "Please add member variables to the ByteStream base, not the ByteStream Reader." );

  return static_cast<Reader&>( *this ); // NOLINT(*-downcast)
}

const Reader& ByteStream::reader() const
{
  static_assert( sizeof( Reader ) == sizeof( ByteStream ),
                 "Please add member variables to the ByteStream base, not the ByteStream Reader." );

  return static_cast<const Reader&>( *this ); // NOLINT(*-downcast)
}

Writer& ByteStream::writer()
{
  static_assert( sizeof( Writer ) == sizeof( ByteStream ),
                 "Please add member variables to the ByteStream base, not the ByteStream Writer." );

  return static_cast<Writer&>( *this ); // NOLINT(*-downcast)
}

const Writer& ByteStream::writer() const
{
  static_assert( sizeof( Writer ) == sizeof( ByteStream ),
                 "Please add member variables to the ByteStream base, not the ByteStream Writer." );

  return static_cast<const Writer&>( *this ); // NOLINT(*-downcast)
}
