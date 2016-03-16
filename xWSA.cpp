#include "xWSA.h"

#include "swap.h"

#ifndef FOR_RING0

#include <ws2tcpip.h>

class xWSA_base
  {
  private:
    xWSA_base();
    ~xWSA_base();
    friend static void xWSA_init();
  };

xWSA_base::xWSA_base()
  {
  WSADATA wsaData;
  if(WSAStartup(MAKEWORD(2,2), &wsaData))
    throw std::runtime_error("WSAStartup失败");
  }

xWSA_base::~xWSA_base()
  {
  WSACleanup();
  }

static void xWSA_init()
  {
  static xWSA_base xwb;
  }

xWSA::xWSA()
  {
  xWSA_init();
  }

sockaddr_in  AddrInfo(const char* host,const char* ports)
  {
  ADDRINFOA* res;

  ADDRINFOA hints;
  memset(&hints, 0, sizeof(hints));

  xWSA wsa;

  if(GetAddrInfoA(host,ports,&hints,&res))
    {
    throw std::runtime_error("GetAddrInfo失败");
    }

  sockaddr_in addrto;
  memcpy(&addrto,res->ai_addr,sizeof(addrto));

  FreeAddrInfoA(res);

  return addrto;
  }

sockaddr_in  AddrInfo(const wchar_t* host,const wchar_t* ports)
  {
  ADDRINFOW* res;

  ADDRINFOW hints;
  memset(&hints,0,sizeof(hints));

  xWSA wsa;

  if(GetAddrInfoW(host,ports,&hints,&res))
    {
    throw std::runtime_error("GetAddrInfo失败");
    }

  sockaddr_in addrto;
  memcpy(&addrto,res->ai_addr,sizeof(addrto));

  FreeAddrInfoW(res);

  return addrto;
  }

sockaddr_in  AddrInfo(const unsigned long host,const unsigned short ports)
  {
  sockaddr_in addrto;
  memset(&addrto,0,sizeof(addrto));

  addrto.sin_family = AF_INET;
  addrto.sin_addr.s_addr = htonl(host);
  addrto.sin_port = htons(ports);

  return addrto;
  }

xmsg IpString(const sockaddr_in& addr)
  {
  return  xmsg()
    << (int)addr.sin_addr.s_net   << '.'
    << (int)addr.sin_addr.s_host << '.'
    << (int)addr.sin_addr.s_lh << '.'
    << (int)addr.sin_addr.s_impno << ':'
    << (int)htons(addr.sin_port);
  }

#endif  //#ifndef FOR_RING0