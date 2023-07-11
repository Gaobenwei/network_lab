#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

void Writer::push( string data )
{
  // Your code here.
  if(available_capacity()==0||data.empty())
    return;
  auto const len=std::min(available_capacity(),data.size());
  if(len<data.size())
  {
    data=data.substr(0,len);
  }
  data_queue_.push(std::move(data));
  //view_queue_.push(std::string_view(data_queue_.back()));
  bytes_pushed_+=len;
  bytes_buffered_+=len;
}

void Writer::close()
{
  // Your code here.
  is_closed_=true;  
}

void Writer::set_error()
{
  // Your code here.
  has_error_=true;
}

bool Writer::is_closed() const
{
  // Your code here.
  return is_closed_;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_-bytes_buffered_;
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return bytes_pushed_;
}

string_view Reader::peek() const
{
  // Your code here.
  if(data_queue_.empty())
    return {};
  return data_queue_.front();
}

bool Reader::is_finished() const
{
  // Your code here.
  if(is_closed_&&bytes_buffered_==0)
    return true;
  return false;
}

bool Reader::has_error() const
{
  // Your code here.
  return has_error_;
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  if(len==0)
    return;
  auto n=std::min(len,bytes_buffered_);
  for(;n!=0;)
  {
    auto size=data_queue_.front().size();
    if(n>=size)
    {
      n-=size;
      bytes_popped_+=size;
      bytes_buffered_-=size;
      //view_queue_.pop();
      data_queue_.pop();
    }
    else
    {
      bytes_popped_+=n;
      bytes_buffered_-=n;
      //view_queue_.front().remove_prefix(n);
      data_queue_.front().erase(0,n);
      return;
    }
  }
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return bytes_buffered_;
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return bytes_popped_;
}
