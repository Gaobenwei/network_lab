#include "socket.hh"

#include <cstdlib>
#include <iostream>
#include <span>
#include <string>

using namespace std;

void get_URL( const string& host, const string& path )
{
/*you will need to connect to the "http" service on the computer whose name is in the "host" string,then request the URL path given in the "path" string.

Then you will need to print out everything the server sends back,(not just one call to read() --everything) until you reach the "eof" (end of file)*/

  /*您需要连接到名称在“host”字符串中的计算机上的“http”服务，然后请求“path”字符串中给出的URL路径。
  然后，您需要打印出服务器发回的所有内容(不仅仅是对read()的一次调用——所有内容)，直到到达“eof”(文件结束)为止。*/

  //创建一个socket对象
  TCPSocket socket;
  //连接到主机
  socket.connect(Address(host, "http"));
  //发送请求
  socket.write("GET " + path + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

  while(!socket.eof())
  {
    //判断是否到达文件末尾，如果没有到达文件末尾，就一直读取
    std::string data;
    socket.read(data);
    cout << data;
  }
  //关闭socket
  socket.close();

  cerr << "Function called: get_URL(" << host << ", " << path << ")\n";
  cerr << "Warning: get_URL() has not been implemented yet.\n";
}

int main( int argc, char* argv[] )
{
  try {
    if ( argc <= 0 ) {
      abort(); // For sticklers: don't try to access argv[0] if argc <= 0.
    }

    auto args = span( argv, argc );

    // The program takes two command-line arguments: the hostname and "path" part of the URL.
    // Print the usage message unless there are these two arguments (plus the program name
    // itself, so arg count = 3 in total).
    //程序接受两个命令行参数:主机名和URL的"path"部分。
    //打印usage消息，除非有这两个参数(加上程序名)
    //自身，所以arg count = 3)。
    if ( argc != 3 ) {
      cerr << "Usage: " << args.front() << " HOST PATH\n";
      cerr << "\tExample: " << args.front() << " stanford.edu /class/cs144\n";
      return EXIT_FAILURE;
    }

    // Get the command-line arguments. 获取命令行参数。
    const string host { args[1] };
    const string path { args[2] };

    // Call the student-written function. 调用学生编写的函数。
    get_URL( host, path );
  } 
  catch ( const exception& e ) 
  {
    cerr << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
