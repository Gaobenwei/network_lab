#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  // Your code here.
  //传入的数据为空
  if(data.empty())
  {
    if(is_last_substring) //
    {
      output.close();
    } 
    return;
  }
  //输出流已满
  if(output.available_capacity()==0) 
  {
    return;
  }

  auto const last_index=first_index + data.size(); //该字符串最后一个字节的下标
  auto const first_unacceptable=first_unassembled_index+output.available_capacity();

  //数据不在[first_unassembled_index, first_unavailable]中,在某一个状态下，字符串只有头尾都位于空闲窗口内才能被接受
  if(last_index<=first_unassembled_index|| first_index>=first_unacceptable)
    return ;

  // 如果部分数据超出容量，则将其截断
  if(first_unacceptable<last_index)
  {
    data=data.substr(0,first_unacceptable - first_index);
    //被截断，是不可能是last_substring的
    is_last_substring=false;
  }

  // 无序字节，保存在缓冲区并返回
  if(first_index>first_unassembled_index)
  {
    insert_into_buffer(first_index,std::move(data),is_last_substring);
    return ;
  }

  // 删除无用的数据前缀（即已经组装好的字节）
  if(first_index < first_unassembled_index)
  {
    data=data.substr(first_unassembled_index-first_index);
  }

  // /*以上首先计算出插入字符串的收尾，在计算出缓冲区即期望的首地址到首次不接受地址
  // 之后，先排除区域两侧的，再截断超过末尾的，位于乱序中间的插入缓冲区，截断已经push的头部分
  // 到此得到了first_index==first_unassembled_index*/

  first_unassembled_index+=data.size();
  output.push(std::move(data));
  if(is_last_substring)
  {
    output.close();
  }

  if(!buffer_.empty()&&buffer_.begin()->first<=first_unassembled_index)
  {
    flush_buffer(output);
  }
 
} 

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return buffer_size_;
}

void Reassembler::insert_into_buffer( const uint64_t first_index, std::string&& data, const bool is_last_substring )
{
  //尽量按照first_index的小到大顺序插入
  //插入的时候，也面临着重复的问题，需要进行判断
  auto begin_index=first_index;
  auto end_index=first_index+data.size();

  //寻找到插入本次字符串的最佳首尾，类似一个线段上截取片段，避免重复的区域
  for(auto iter=buffer_.begin();iter!=buffer_.end()&&begin_index<end_index;)
  {
    //跳过线段上小的区域
    if(iter->first<=begin_index)
    {
      begin_index=std::max(begin_index,iter->first+iter->second.size());
      ++iter;
      continue;
    }

    //一次合适的插入
    if(begin_index==first_index&&end_index<=iter->first)
    {
      buffer_size_+=data.size();
      buffer_.emplace(iter,std::make_pair(first_index,std::move(data)));
      return;
    }

    auto const right_index=std::min(end_index,iter->first);
    auto len=right_index-begin_index;
    buffer_size_+=len;
    buffer_.emplace(iter,std::make_pair(begin_index,data.substr(begin_index-first_index,len))); //注意这里截取字符串不能0开头，应为有可能begin_index已经变动过了
    begin_index=right_index; //更新begin_index，这样处理后，[begin_index,end_index)这段区间就是已存在的区间，下一次循环就会结束
  }

  if(begin_index<end_index) //这种情况跳出循环是到了buffer的末尾，需要在buffer的末尾插入
  {
    buffer_size_+=end_index-begin_index;
    buffer_.emplace_back(std::make_pair(begin_index,data.substr(begin_index-first_index)));
  }

  if(is_last_substring)
  {
    is_last_if=true;
  }
}





void Reassembler::flush_buffer( Writer& output )
{
  for(auto it=buffer_.begin();it!=buffer_.end();)
  {
    if(it->first>first_unassembled_index)
    {
      break;
    } 

    //it->first<=first_unassembled_index
    const auto end=it->first+it->second.size();
    if(end <= first_unassembled_index)
    {
      buffer_size_-=it->second.size();
    }
    else
    {
      auto data=std::move(it->second);
      buffer_size_-=data.size();
      if(it->first<first_unassembled_index)
      {
        data=data.substr(first_unassembled_index-it->first);
      }
      first_unassembled_index+=data.size();
      output.push(std::move(data));
    }
    it=buffer_.erase(it);
  }
  if(buffer_.empty()&&is_last_if)
  {
    output.close();
  }
}
