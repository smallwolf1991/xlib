#include "xmsg.h"
#include <stdio.h>
#include <stdarg.h>
#include "ws_s.h"

using namespace std;

#pragma warning(push)
#pragma warning(disable:4127)  //warning C4127: 条件表达式是常量
xmsg& xmsg::prt(const char* const fmt,...)
  {
  va_list ap;
  va_start(ap,fmt);
  while(true)
    {
    if(capacity() - size() <= 1)
      {
      reserve(capacity() + 0x10);
      }
    const size_t rst = capacity() - size();
#ifdef FOR_RING0
    char* lpend = end();
#else
    char* lpend = const_cast<char*>(end()._Ptr);
#endif
#if defined(FOR_RING0) && !defined(_WIN64)
    //目前WIN7以下Ring0只能使用不安全的函数，否则编译出现链接错误
    const size_t rt = _vsnprintf(lpend,rst,fmt,ap);  //数据不足，返回-1
#else
    const size_t rt = _vsnprintf_s(lpend, rst, rst - 1, fmt, ap);
#endif
    if(rt < rst)
      {
      append(lpend, rt);
      break;
      }
    reserve(capacity() + 0x10);
    }
  va_end(ap);
  return *this;
  }
#pragma warning(pop)

xmsg& xmsg::operator<<(const char& v)
  {
  return prt("%c",v);
  }

xmsg& xmsg::operator<<(const unsigned char& v)
  {
  return prt("%02X",v);
  }

xmsg& xmsg::operator<<(const short& v)
  {
  return prt("%hd",v);
  }

xmsg& xmsg::operator<<(const unsigned short& v)
  {
  return prt("%04X",v);
  }

xmsg& xmsg::operator<<(const int& v)
  {
  return prt("%d",v);
  }

xmsg& xmsg::operator<<(const unsigned int& v)
  {
  return prt("%08X",v);
  }

xmsg& xmsg::operator<<(const __int64& v)
  {
  return prt("%lld", v);
  }

xmsg& xmsg::operator<<(const unsigned __int64& v)
  {
  return prt("%08X%08X", (int)(v >> 32), (int)v);
  }

xmsg& xmsg::operator<<(const char* v)
  {
  return prt("%s", v);
  }

xmsg& xmsg::operator<<(const unsigned char* v)
  {
  return prt("%s", v);
  }

xmsg& xmsg::operator<<(const bool& v)
  {
  const char* const tmp = v ? ":true." : ":false.";
  return this->operator <<(tmp);
  }

xmsg& xmsg::operator<<(const wchar_t& v)
  {
  wchar_t tmpws[2] = { v, L'\0' };
  return prt("%s", ws2s(tmpws).c_str());
  }

xmsg& xmsg::operator<<(const wchar_t* v)
  {
  return prt("%s", ws2s(v).c_str());
  }

xmsg& xmsg::operator<<(const float& v)
  {
  return prt("%f", v);
  }

xmsg& xmsg::operator<<(const double& v)
  {
  return prt("%f", v);
  }

xmsg& xmsg::operator<<(const void* v)
  {
  return prt("%p",v);
  }

xmsg& xmsg::operator<<(const string& v)
  {
  return prt("%s", v.c_str());
  }

xmsg& xmsg::operator<<(const wstring& v)
  {
  return this->operator<<(v.c_str());
  }

xmsg& xmsg::operator<<(const xutf8& v)
  {
  return this->operator<<(utf82ws(v));
  }

xmsg& xmsg::operator<<(xmsg& (*pfn)(xmsg&))
  {
  return pfn(*this);
  }