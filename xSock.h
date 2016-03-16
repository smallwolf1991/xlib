/*!
  \file  xSock.h
  \brief xSock.h定义了网络通讯的基本封装

  \version    3.0.1211.1316
  \note       Only For Ring3

  \author     triones
  \date       2011-01-27
*/

#pragma once

#ifndef FOR_RING0

#include "xWSA.h"
#include "xline.h"
#include "xlog.h"

//! 模板，用于指定协议建立socket
template<int TYPES> class xSocket
  {
  public:
    //! 自动创建socket
    xSocket()
      {
      _hSocket = socket(PF_INET,TYPES,0);
      if(hSocket() == INVALID_SOCKET)      throw std::runtime_error("建立Socket失败");
      }
    //! 指定接受已经存在的socket
    xSocket(const SOCKET hS)
      :_hSocket(hS)
      {
      if(hSocket() == INVALID_SOCKET)      throw std::runtime_error("初始化Socket失败");
      }
    virtual ~xSocket()
      {
      if(hSocket() != INVALID_SOCKET)
        {
        shutdown(hSocket(),SD_BOTH);
        closesocket(hSocket());
        }
      }
    //! 返回SOCKET句柄
    SOCKET hSocket() const
      {
      return _hSocket;
      }
  private:
    xSocket(const xSocket&);
    xSocket& operator=(const xSocket&);
  private:
    SOCKET      _hSocket;
    const xWSA  wsa;
  };

//! 应用于TCP及UDP的基本通讯
template<int TYPES> class xSock
  {
  public:
    typedef xSock<TYPES>      _Myt;
  public:
    template<typename T> _Myt& operator<<(const T& argvs)
      {
      sendbuf << argvs;
      return *this;
      }
    template<typename T> _Myt& operator>>(T& argvs)
      {
      recvbuf >> argvs;
      return *this;
      }
  public:
    //! 返回SOCKET句柄
    SOCKET hSocket() const
      {
      return _socket.hSocket();
      }
    /*!
      当IP地址为0时，视作建立服务器端。否则视作建立客户端。\n
      建立服务器端将自动绑定本机端口。(注意与127.0.0.1区别)\n
      TCP协议建立服务器端，自动开启监听。\n
      TCP协议建立客户端时，自动连接指定IP。\n
      客户端默认收发延时50ms。服务器端无限制
    */
    xSock(const sockaddr_in& addr):_socket(),addrto(addr)
      {
      if(TYPES == SOCK_DGRAM)   recvbuf.reserve(0x400);
      if(addrto.sin_addr.S_un.S_addr == INADDR_ANY)
        {
        if(bind(hSocket(),(sockaddr*)&addrto,sizeof(addrto)))
          throw std::runtime_error("xSock 绑定失败");
        if(TYPES == SOCK_STREAM)
          {
          if(listen(hSocket(),0))
            throw std::runtime_error("xSock 监听失败");
          }
        }
      else
        {
        opt(SOL_SOCKET,SO_SNDTIMEO,(char*)&timeout,sizeof(timeout));
        opt(SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout));
        if(TYPES == SOCK_STREAM)
          {
          if(connect(hSocket(),(sockaddr*)&addrto,sizeof(addrto)))
            throw std::runtime_error("xSock 连接失败");
          }
        }
      }
    //! 指定SOCKET，作为客户端建立xSock
    xSock(const SOCKET hS,const sockaddr_in& addr)
      :_socket(hS),addrto(addr)
      {
      opt(SOL_SOCKET,SO_SNDTIMEO,(char*)&timeout,sizeof(timeout));
      opt(SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout));
      }
    //! 用于服务器端accept
    _Myt* apt()
      {
      int namelen = sizeof(addrfrom);
      const SOCKET hS = accept(hSocket(),(sockaddr*)&addrfrom,&namelen);
      if(hS == INVALID_SOCKET)    throw std::runtime_error("xSock 接受连接失败");
      return new _Myt(hS,addrfrom);
      }
    //! 用于setsockopt
    void opt(int level,int optname,const char FAR * optval,int optlen)
      {
      if(setsockopt(hSocket(),level,optname,optval,optlen))
        throw  std::runtime_error("xSock 设置失败");
      }
    //! 用于基本发送，注意不抛出异常
    bool send(IN const char FAR * buf, IN int len)
      {
      while(len > 0)
        {
        int sendlen;
        if(TYPES == SOCK_STREAM)
          sendlen = ::send(hSocket(),buf,len,0);
        else
          sendlen = ::sendto(hSocket(),buf,len,0,
          (const sockaddr *)&addrto,sizeof(addrto));

        if(sendlen <= 0)
          {
          xerr << "xSock send Err:" << WSAGetLastError();
          return false;
          }
        len -= sendlen;
        buf += sendlen;
        }
      return true;
      }
    //! 用于基本发送，主要应用于结构体
    template<typename T> bool send(T const& v)
      {
      return send((char*)&v,sizeof(T));
      }
    //! 发送数据，注意发送失败，抛出异常。发送成功，清除缓冲
    _Myt& send()
      {
      if(!send((char*)sendbuf.c_str(),(int)sendbuf.size()))
        throw std::runtime_error("xSock 发送数据失败");
      sendbuf.clear();
      return *this;
      }
    //! 发送数据，会附加数据头。
    _Myt& sendmkhead()
      {
      sendbuf.mkhead();
      return send();
      }
    //! 用于基本接收，注意需要根据返回值自行处理
    int recv(char FAR * buf, IN int len)
      {
      int recvlen;
      if(TYPES == SOCK_STREAM)
        {
        recvlen = ::recv(hSocket(),buf,len,0);
        }
      else
        {
        int addrfromlen = sizeof(addrfrom);
        memset(&addrfrom,0,sizeof(addrfrom));
        recvlen = ::recvfrom(hSocket(),buf,len,0,
          (sockaddr*)&addrfrom,&addrfromlen);
        }
      return recvlen;
      }
    //! 用于基本接收，主要应用于结构体
    template<typename T> int recv(T& v)
      {
      return recv((char*)&v,sizeof(T));
      }
    //! 接收数据
    int recv()
      {
      int mainrecvlen = 0;
      while(mainrecvlen >= 0)
        {
        if(recvbuf.capacity() - recvbuf.size() == 0)
          recvbuf.reserve(recvbuf.capacity() + 0x40);
        unsigned char* lprecv = const_cast<unsigned char*>(recvbuf.end()._Ptr);
        const int recvlen = recv((char*)lprecv, (int)(recvbuf.capacity() - recvbuf.size()));
        if(recvlen == 0)
          {
          xtrace << "xSock recv close";
          return 0;
          }
        if(recvlen == SOCKET_ERROR)
          {
          int codes = WSAGetLastError();
          xtrace << "xSock recv Err:" << codes;
          switch(codes)
            {
            case WSAETIMEDOUT:  return mainrecvlen;
            case WSAECONNRESET:  return mainrecvlen;
            case WSAEMSGSIZE:
              //UDP会出现这个错误，此时数据已经被截断
              {
              const int len = (int)(recvbuf.capacity() - recvbuf.size());
              recvbuf.append(lprecv, mainrecvlen);
              recvbuf.reserve(recvbuf.capacity() + 0x40);
              return mainrecvlen + len;
              }
            case WSAENOTCONN:    throw std::runtime_error("xSock 作为服务器不能接收数据");
            default:            throw std::runtime_error("xSock 接收数据失败");
            }
          }
        recvbuf.append(lprecv,recvlen);
        mainrecvlen += recvlen;
        if(recvbuf.capacity() - recvbuf.size() != 0)
          break;//如果缓冲被填满，可能还有数据需要接收
        }
      return mainrecvlen;
      }
  public:
    netline     sendbuf;
    netline     recvbuf;
    //因查询的需要，又不愿意写独立的访问函数，故意放置为public
    sockaddr_in addrto;
    sockaddr_in addrfrom;
    static const int timeout = 50;  //默认收发延时50ms
  private:
    const xSocket<TYPES> _socket;
  };

typedef xSock<SOCK_STREAM>    xTCP;
typedef xSock<SOCK_DGRAM>     xUDP;

typedef xUDP& (*xUDP_func)(xUDP&);
typedef xTCP& (*xTCP_func)(xTCP&);

//! 重载使xSock能以流形式操作数据，这个重载无法做在类内部
xUDP& operator<<(xUDP& s,xUDP_func pfn);
xTCP& operator<<(xTCP& s,xTCP_func pfn);

//! 重载使xSock能以流形式发送数据
xUDP& xsend(xUDP& s);
xTCP& xsend(xTCP& s);
xUDP& xxsend(xUDP& s);
xTCP& xxsend(xTCP& s);

#endif  //#ifndef FOR_RING0