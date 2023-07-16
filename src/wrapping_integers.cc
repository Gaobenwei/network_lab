#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  return Wrap32 {zero_point.raw_value_ + static_cast<uint32_t>(n)};
  //return zero_point+n;
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  //不只是不是bug，完全没有考虑raw_value_<checkpoint32.raw_value_
  Wrap32 checkpoint32=Wrap32::wrap(checkpoint,zero_point);
  uint64_t diff=raw_value_-checkpoint32.raw_value_;
  static constexpr uint64_t a=1UL<<32;
  static constexpr uint64_t b=1UL<<31;
  if(diff==0)
    return checkpoint;
  if(diff<=b)  //右边更近
    return checkpoint+diff;
  diff=a-diff;
  if(checkpoint<diff)
  {
    return checkpoint+a-diff;
  }
  return checkpoint-diff;
}
