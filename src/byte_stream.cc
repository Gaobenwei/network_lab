#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

void Writer::push( string data )
{
  // Your code here.
  if(data.empty()||available_capacity()==0)
    return;
  auto n=std::min(data.size(),available_capacity());
  if(n<data.size())  //要对string data进行截断
  {
    data=data.substr(0,n);
  }
  data_queue_.push_back(std::move(data));
  //view_queue_.push(data_queue_.back());
  view_queue_.emplace_back(data_queue_.back().c_str(),n);
  bytes_buffered_+=n;
  bytes_pushed_+=n;
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
  //Your code here.
  if(data_queue_.empty())
    return {};
  return data_queue_.front();
  /* bug1以下会出错*/
  // if(view_queue_.empty())
  //   return {};
  // return view_queue_.front();
  
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
  // if(len==0)
  //   return;
  // auto n=std::min(len,bytes_buffered_);
  // for(;n!=0;)
  // {
  //   auto size=data_queue_.front().size();
  //   if(n>=size)
  //   {
  //     n-=size;
  //     bytes_popped_+=size;
  //     bytes_buffered_-=size;
  //     //view_queue_.pop();
  //     data_queue_.pop();
  //   }
  //   else
  //   {
  //     bytes_popped_+=n;
  //     bytes_buffered_-=n;
  //     //view_queue_.front().remove_prefix(n);
  //     data_queue_.front().erase(0,n);
  //     return;
  //   }
  // }

  if(len==0)
    return;
  auto n=std::min(len,bytes_buffered_);
  while(n!=0)
  {
    auto size=view_queue_.front().size();
    //原因在此，size如果是用的data_queue_的size，那么就会出错,因为data_queue_的size是string的size，而不是string_view的size，两者的不同之处在于string_view的size是不包含'\0'的，（这句话是copilot说的，有待查证）
    if(n>=size)
    {
      n-=size;
      bytes_popped_+=size;
      bytes_buffered_-=size;
      view_queue_.pop_front();
      data_queue_.pop_front();
    }
    else{
      bytes_popped_+=n;
      bytes_buffered_-=n;
      //应该是这里出错了，否定这个猜测
      view_queue_.front().remove_prefix(n);
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
