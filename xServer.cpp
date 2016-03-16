#include "xServer.h"

#ifndef FOR_RING0

using namespace std;

//注意，服务器绑定只能NULL而不能是127.0.0.1，否则不响应它机
xTCPServ::xTCPServ(const unsigned short ports)
:_Mybase(AddrInfo(INADDR_ANY,ports))
  {
  needclose = false;
  filter_container tmpv;
  for(int i = 0;i < filter_end;++i)
    {
    filter.push_back(tmpv);
    }
  s.insert(this);
  }

xTCPServ::~xTCPServ()
  {
  closeit();
  }

void xTCPServ::close()
  {
  needclose = true;
  }

void xTCPServ::setfilter(enum_deal index,func_deal func,bool addordel)
  {
  try
    {
    if(index >= filter_end)      return;
    filter_container& con = filter[index];
    if(addordel)
      {
      con.push_back(func);
      }
    else
      {
      for(filter_container::iterator it = con.begin();it != con.end();++it)
        {
        if(*it == func)
          {
          con.erase(it);
          break;
          }
        }
      }
    }
  catch(...)
    {
    xerr << __FUNCTION__ << "★★ 发生异常!";
    }
  }

bool xTCPServ::wait()
  {
  try
    {
    if(needclose || s.empty())  return false;
    //组织fd_set
    sline.clear();
    sline << (u_int)s.size();
    for(_Mybase* ns : s)
      {
      sline << ns->hSocket();
      }
    //监听
    if(SOCKET_ERROR == select((int)s.size(),(fd_set*)sline.c_str(),nullptr,nullptr,nullptr))
      {
      xerr << __FUNCTION__ << "★ 数据监听失败!";
      return false;
      }
    //处理响应，内部接受connect
    for(_Mybase* ns : s)
      {
      if(FD_ISSET(ns->hSocket(),(fd_set*)sline.c_str()))
        {
        recvit(ns);
        }
      }
    //判断连接、加入连接队列
    for(_Mybase* ns : waitsock)
      {
      aptit(ns);
      }
    //枚举连接（枚举必须放在“删除连接”前）
    for(_Mybase* ns : s)
      {
      enumit(ns);
      }
    //删除连接
    for(_Mybase* ns : delsock)
      {
      delit(ns);
      }
    delsock.clear();
    waitsock.clear();
    return true;
    }
  catch(TCHAR* msg)
    {
    xerr << __FUNCTION__ << "★★ " << ws2s(msg);
    }
  catch(...)
    {
    xerr << __FUNCTION__ << "★★ 发生未知异常!";
    }
  return false;
  }

void xTCPServ::delit(_Mybase* ns)
  {
  try
    {
    if(ns == nullptr || ns == this)    return;
    filter_container& vec = filter[deal_del];
    for(func_deal func : vec)
      {
      func(ns);
      }
    sock_container::iterator it = s.find(ns);
    if(it != s.end())      s.erase(it);
    delete ns;
    }
  catch(...)
    {
    xerr << __FUNCTION__ << "★★ 处理" << IpString(ns->addrto) << "时发生异常!";
    }
  }

void xTCPServ::catchit(_Mybase* ns)
  {
  try
    {
    if(ns == nullptr || ns == this)    return;
    filter_container& vec = filter[deal_excpt];
    if(vec.empty())
      {
      delit(ns);
      return;
      }
    for(func_deal func : vec)
      {
      if(func(ns))  delit(ns);
      }
    }
  catch(...)
    {
    xerr << __FUNCTION__ << "★★ 处理" << IpString(ns->addrto) << "时发生异常!";
    //这里再次尝试无条件结束连接，如果失败将会生成多重异常
    delit(ns);
    }
  }

void xTCPServ::aptit(_Mybase* ns)
  {
  try
    {
    if(ns == nullptr || ns == this)    return;
    filter_container& vec = filter[deal_apt];
    for(func_deal func : vec)
      {
      if(func(ns))
        {
        delete ns;
        return;
        }
      }
    s.insert(ns);
    }
  catch(...)
    {
    xerr << __FUNCTION__ << "★★ 处理" << IpString(ns->addrto) << "时发生异常!";
    catchit(ns);
    }
  }

void xTCPServ::enumit(_Mybase* ns)
  {
  try
    {
    if(ns == nullptr || ns == this)    return;
    if(delsock.find(ns) != delsock.end())  return;
    filter_container& vec = filter[deal_enum];
    for(func_deal func : vec)
      {
      if(func(ns))    delsock.insert(ns);
      }
    }
  catch(...)
    {
    xerr << __FUNCTION__ << "★★ 处理" << IpString(ns->addrto) << "时发生异常!";
    catchit(ns);
    }
  }

void xTCPServ::recvit(_Mybase* ns)
  {
  try
    {
    if(ns == nullptr)    return;
    //如果是服务器收到消息，则是连接消息
    if(ns == this)
      {
      //接受连接，放入等待队列
      //必须先apt后，addrfrom才有值，为保准确，需要先做
      waitsock.insert(this->apt());
      return;
      }
    int rets = (*ns).recv();
    if(rets <= 0)
      {
      //连接断开,放入删除队列中
      delsock.insert(ns);
      return;
      }
    filter_container& vec = filter[deal_recv];
    for(func_deal func : vec)
      {
      if(func(ns))
        {
        delsock.insert(ns);
        break;
        }
      }
    }
  catch(...)
    {
    xerr << __FUNCTION__ << "★★ 处理" << IpString(ns->addrto) << "时发生异常!";
    catchit(ns);
    }
  }

void xTCPServ::closeit()
  {
  try
    {
    netline nline;
    filter_container& vec = filter[deal_close];
    //反向删除
    for(sock_container::reverse_iterator it = s.rbegin(); it != s.rend(); ++it)
      {
      if(*it == this)  continue;
      for(func_deal func : vec)
        {
        func(*it);
        }
      delete *it;
      }
    s.clear();
    }
  catch(...)
    {
    xerr << __FUNCTION__ << "★★ 发生异常!";
    }
  }




xUDPServer::xUDPServer(const unsigned short ports)
:_Mybase(AddrInfo(INADDR_ANY,ports))
  {
  needclose = false;
  }

void xUDPServer::close()
  {
  needclose = true;
  }

bool xUDPServer::wait()
  {
  try
    {
    if(needclose)  return false;
    //组织fd_set
    fd_set fs;
    fs.fd_count = 1;
    fs.fd_array[0] = hSocket();
    //监听
    if(SOCKET_ERROR == select(fs.fd_count,&fs,nullptr,nullptr,nullptr))
      {
      xerr << __FUNCTION__ << "★ 数据监听失败!";
      return false;
      }

    int rets = recv();
    if(rets <= 0)   throw runtime_error("xUDPServer 接收数据失败");
    for(func_deal func : filter)
      {
      if(func(this)) break;
      }
    return true;
    }
  catch(TCHAR* msg)
    {
    xerr << __FUNCTION__ << "★★ " << ws2s(msg);
    }
  catch(...)
    {
    xerr << __FUNCTION__ << "★★ 发生未知异常!";
    }
  return false;
  }

void xUDPServer::setfilter(func_deal func,bool addordel)
  {
  try
    {
    if(addordel)
      {
      filter.push_back(func);
      }
    else
      {
      for(filter_container::iterator it = filter.begin();it != filter.end();++it)
        {
        if(*it == func)
          {
          filter.erase(it);
          break;
          }
        }
      }
    }
  catch(...)
    {
    xerr << __FUNCTION__ << "★★ 发生异常!";
    }
  }

#endif  //#ifndef FOR_RING0