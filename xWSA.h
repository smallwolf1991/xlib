/*!
  \file  xWSA.h
  \brief xWSA.h定义了使用网络的基本封装

  \version    2.1.1607.2013
  \note       Only For Ring3

  \author     triones
  \date       2012-06-08
*/

#pragma once

#ifndef FOR_RING0

#include <winsock2.h>
#pragma comment (lib,"ws2_32.lib")
#include <string>

//! 用于保证sock环境初始化，注意构造失败抛出异常
class xWSA
  {
  public:
    xWSA();
  };

//! 由指定IP地址/IP地址串、端口/端口字串，组织sockaddr_in结构
/*!
  解析错误时，抛出runtime_error异常

  \code
    sockaddr_in addr = AddrInfo("127.0.1.1","4210");
  \endcode
*/
sockaddr_in  AddrInfo(const char* host,const char* ports);

/*!
  解析错误时，抛出runtime_error异常

  \code
    sockaddr_in addr = AddrInfo(L"127.0.1.1",L"4210");
  \endcode
*/
sockaddr_in  AddrInfo(const wchar_t* host,const wchar_t* ports);

/*!

  \code
    sockaddr_in addr = AddrInfo(0x7F000001,4210);
  \endcode
*/
sockaddr_in  AddrInfo(const unsigned long host,const unsigned short ports);

//! 由指定sockaddr_in结构，输出“#.#.#.#:#”串
std::string IpString(const sockaddr_in& addr);

#endif  //#ifndef FOR_RING0