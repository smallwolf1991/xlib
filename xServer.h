/*!
  \file  xServer.h
  \brief xServer.h定义了服务器的基本封装

  \version    1.0.1201.2811
  \note       Only For Ring3

  \author     triones
  \date       2011-01-27
*/

#pragma once

#ifndef FOR_RING0

#include "xSock.h"
#include "ws_s.h"
#include "xline.h"
#include <set>
#include <vector>

//! 服务器类
class xTCPServ : public xSock<SOCK_STREAM>
  {
  public:
    typedef xSock<SOCK_STREAM>        _Mybase;
    typedef xTCPServ                  _Myt;
    typedef std::set<_Mybase*>        sock_container;
    typedef bool (*func_deal)(_Mybase* ns);
    typedef std::vector<func_deal>    filter_container;
    enum enum_deal
      {
      /*!
        结束通讯，删除连接处理回调\n
        连接处于删除过程，不可逆转。忽略返回值\n
        如果删除过程中发生异常，没有异常回调通知
      */
      deal_del,

      /*!
        异常处理回调\n
        通讯过程中捕捉到异常时，将有机会处理是否忽略或强制结束通讯\n
        判定强制结束通讯则返回true，否则返回false
      */
      deal_excpt,
      
      /*!
        接受连接处理回调。\n
        判定不能接受连接则返回true，否则返回false
      */
      deal_apt,

      /*!
        枚举连接处理回调。\n
        判定非法则返回true，否则返回false
      */
      deal_enum,

      /*!
        通讯处理回调。\n
        判定非法则返回true，否则返回false
      */
      deal_recv,

      /*!
        服务器关闭处理回调。关闭时，有机会处理每个连接
      */
      deal_close,

      filter_end
      };
  public:
    xTCPServ(const unsigned short ports);
    ~xTCPServ();
    void close();               //! 用以驱动服务器关闭 
    void setfilter(enum_deal index,func_deal func,bool addordel);//! 设置或删除过滤函数
    bool wait();                //! 用于执行一次通讯处理
  private:
    void delit(_Mybase* ns);    //! 连接解除
    void catchit(_Mybase* ns);  //! 异常捕捉
    void aptit(_Mybase* ns);    //! 接受连接
    void enumit(_Mybase* ns);   //! 连接枚举
    void recvit(_Mybase* ns);   //! 连接通讯
    void closeit();             //! 服务器关闭
  private:
    bool                  needclose;    //! 用以决定服务器是否关闭
    sock_container        s;            //! 保存当前连接
    line                  sline;        //! 动态的fd_set
    sock_container        waitsock;     //! 保存当前等待加入连接的socket
    sock_container        delsock;      //! 保存当前等待关闭的socket
    std::vector<filter_container> filter; //! 过滤函数组
  };


class xUDPServer : public xSock<SOCK_DGRAM>
  {
  public:
  public:
    typedef xSock<SOCK_DGRAM>         _Mybase;
    typedef xUDPServer                _Myt;
    typedef bool (*func_deal)(_Mybase* ns);
    typedef std::vector<func_deal>    filter_container;
  public:
  public:
    xUDPServer(const unsigned short ports);
    void close(); 
    /*!
      通讯处理回调\n
      判定非法则返回true，否则返回false
    */
    void setfilter(func_deal func,bool addordel);
    bool wait();        //! 用于执行一次通讯处理
  private:
    bool                needclose;      //! 用以决定服务器是否关闭
    filter_container    filter;         //! 过滤函数组
  };

#endif  //#ifndef FOR_RING0