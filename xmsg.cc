#include "xmsg.h"

#include <stdio.h>
#include <stdarg.h>

using std::string;
using std::basic_string;

xmsg::xmsg()
  {
  ;
  }

xmsg::xmsg(const string& s)
  {
  assign(s);
  }

xmsg::xmsg(const basic_string<unsigned char>& s)
  {
  assign((const char*)s.c_str(), s.size());
  }

xmsg::xmsg(const ucs2string& s)
  {
  assign(ws2s(s));
  }

#ifdef _WIN32
#   pragma warning(push)
#   pragma warning(disable:4127)  //warning C4127: 条件表达式是常量
#endif  // _WIN32
xmsg& xmsg::prt(const char* const fmt, ...)
  {
  va_list ap;
  va_start(ap, fmt);
  while(true)
    {
    if(capacity() - size() <= 1)
      {
      reserve(capacity() + 0x10);
      }
    const size_t rst = capacity() - size();
#ifdef _WIN32
#   ifdef FOR_RING0
    char* lpend = end();
#   else    // FOR_RING0
    char* lpend = const_cast<char*>(end()._Ptr);
#   endif   // FOR_RING0
#else   // _WIN32
    char* lpend = const_cast<char*>(c_str()) + size();
#endif  // _WIN32
#ifdef _WIN32
#   if defined(FOR_RING0) && !defined(_WIN64)
    //目前WIN7以下Ring0只能使用不安全的函数，否则编译出现链接错误
    const size_t rt = _vsnprintf(lpend, rst, fmt, ap);  //数据不足，返回-1
#   else    // FOR_RING0 _WIN64
    const size_t rt = _vsnprintf_s(lpend, rst, rst - 1, fmt, ap);
#   endif   // FOR_RING0 _WIN64
#else   // _WIN32
    const size_t rt = vsnprintf(lpend, rst, fmt, ap);  //数据不足，返回-1
#endif  // _WIN32
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
#ifdef _WIN32
#   pragma warning(pop)
#endif  // _WIN32

xmsg& xmsg::operator<<(const char& v)
  {
  return prt("%c", v);
  }

xmsg& xmsg::operator<<(const unsigned char& v)
  {
  return prt("%02X", v);
  }

xmsg& xmsg::operator<<(const short& v)
  {
  //在g++下，需要两个参数才正常，原因不明
  return prt("%hd", v, v);
  }

xmsg& xmsg::operator<<(const unsigned short& v)
  {
  return prt("%04X", v);
  }

xmsg& xmsg::operator<<(const int& v)
  {
  return prt("%d", v);
  }

xmsg& xmsg::operator<<(const unsigned int& v)
  {
  return prt("%08X", v);
  }

xmsg& xmsg::operator<<(const int64& v)
  {
  return prt("%lld", v);
  }

xmsg& xmsg::operator<<(const uint64& v)
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
  const char* const tmp = v ? ":true" : ":false";
  return this->operator <<(tmp);
  }

xmsg& xmsg::operator<<(const charucs2_t& v)
  {
  return prt("%s", ws2s(ucs2string(1, v)).c_str());
  }

xmsg& xmsg::operator<<(const charucs2_t* v)
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
#ifdef _WIN32
  return prt("%p", v);
#else   // _WIN32
#   ifdef __amd64
  return operator<<((uint64)v);
#   else    // _amd64
  return operator<<((uint32)v);
#   endif   // _amd64
#endif  // _WIN32
  }

xmsg& xmsg::operator<<(const string& v)
  {
  return prt("%s", v.c_str());
  }

xmsg& xmsg::operator<<(const ucs2string& v)
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

#ifdef _XLIB_TEST_

ADD_XLIB_TEST(XMSG)
  {
  SHOW_TEST_INIT;

  bool done = false;

  xmsg msg;

  SHOW_TEST_HEAD("string");
  msg = xmsg("123");
  done = (msg == string("123"));
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("string");
  msg = xmsg(std::basic_string<unsigned char>((const unsigned char*)"123"));
  done = (msg == string("123"));
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("ucs2string");
  msg = xmsg(s2ws("123"));
  done = (msg == string("123"));
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("char");
  msg.clear();
  msg << '1';
  done = (msg == string("1"));
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("uchar");
  msg.clear();
  msg << (unsigned char)'1';
  done = (msg == string("31"));
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("short");
  msg.clear();
  msg << (short)1234;
  done = (msg == string("1234"));
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("ushort");
  msg.clear();
  msg << (unsigned short)0x1234;
  done = (msg == string("1234"));
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("int");
  msg.clear();
  msg << (int)1234;
  done = (msg == string("1234"));
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("uint");
  msg.clear();
  msg << (unsigned int)0x12345678;
  done = (msg == string("12345678"));
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("char*");
  msg.clear();
  msg << "123";
  done = (msg == string("123"));
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("uchar*");
  msg.clear();
  msg << (unsigned char*)"123";
  done = (msg == string("123"));
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("bool");
  msg.clear();
  msg << true;
  done = (msg == string(":true"));
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("charucs2_t");
  msg.clear();
  msg << (charucs2_t)0x31;
  done = (msg == string("1"));
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("charucs2_t*");
  msg.clear();
  msg << s2ws("123").c_str();
  done = (msg == string("123"));
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("float");
  msg.clear();
  msg << (float)1.0;
  done = (msg == string("1.000000"));
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("double");
  msg.clear();
  msg << (double)1.0;
  done = (msg == string("1.000000"));
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("void*");
  msg.clear();
  msg << (void*)0x12345678;
#if defined(_WIN64) || defined(__amd64)
  done = (msg == string("0000000012345678"));
#else   // _WIN64 __amd64
  done = (msg == string("12345678"));
#endif  // _WIN64 __amd64
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("string");
  msg.clear();
  msg << string("123");
  done = (msg == string("123"));
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("xutf8");
  msg.clear();
  msg << ws2utf8(s2ws(string("AA转换测试BB")));
  done = (msg == string("AA转换测试BB"));
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("ustring");
  msg.clear();
  msg << ucs2string(s2ws("123"));
  done = (msg == string("123"));
  SHOW_TEST_RESULT(done);
  }

#endif  // _XLIB_TEST_