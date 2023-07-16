#pragma once

#include <cstdint>

/*
 * The Wrap32 type represents a 32-bit unsigned integer that:
 *    - starts at an arbitrary "zero point" (initial value), and
 *    - wraps back to zero when it reaches 2^32 - 1.
 */

/*Wrap32类型表示一个32位无符号整数，它表示:
从任意的“零点”(初始值)开始，并在达到2^32 - 1时返回零。*/

class Wrap32
{
protected:
  uint32_t raw_value_ {};

public:
  explicit Wrap32( uint32_t raw_value ) : raw_value_( raw_value ) {}

  /* Construct a Wrap32 given an absolute sequence number n and the zero point.  给定绝对序号n和零点，构造一个Wrap32。*/
  static Wrap32 wrap( uint64_t n, Wrap32 zero_point );

  /*
   * The unwrap method returns an absolute sequence number that wraps to this Wrap32, given the zero point
   * and a "checkpoint": another absolute sequence number near the desired answer.
   *unwrap方法返回一个绝对序列号，该序列号包装到这个Wrap32，给定零点和一个“检查点”:另一个接近所需答案的绝对序列号。

   * There are many possible absolute sequence numbers that all wrap to the same Wrap32.
   * The unwrap method should return the one that is closest to the checkpoint.
   * 
   * 有许多可能的绝对序列号，所有包装到相同的Wrap32。
unwrap方法应该返回离检查点最近的那个。
   */
  uint64_t unwrap( Wrap32 zero_point, uint64_t checkpoint ) const;

  Wrap32 operator+( uint32_t n ) const { return Wrap32 { raw_value_ + n }; }
  bool operator==( const Wrap32& other ) const { return raw_value_ == other.raw_value_; }
};
