﻿#include "hex_str.h"

#include <string.h>

#include "xmsg.h"
#include "ws_s.h"
#include "ws_utf8.h"

using std::string;

//! 十六进制转换，小写索引
static const char* const gk_NumFmt_Low  = "0123456789abcdef";
//! 十六进制转换，大写索引
static const char* const gk_NumFmt_Up   = "0123456789ABCDEF";

string hex2str(const void* hexs, const size_t size, bool isup)
  {
  string asc;
  if(hexs == nullptr || size == 0)  return asc;
  const auto fmt = isup ? gk_NumFmt_Up : gk_NumFmt_Low;

  for(size_t i = 0; i < size; ++i)
    {
    const HEX_VALUE_STRUCT* hexchar = (const HEX_VALUE_STRUCT*)((size_t)hexs + i);
    asc.push_back(fmt[hexchar->high]);
    asc.push_back(fmt[hexchar->low]);
    }

  return asc;
  }

size_t str2hex(const void*    strs,
               const size_t   size,
               size_t*        lpreadlen,
               size_t         wantlen,
               const bool     errexit,
               const bool     errbreak)
  {
  size_t values = 0;
  size_t readlen = 0;

  if(lpreadlen == nullptr) lpreadlen = &readlen;
  *lpreadlen = 0;

  if(strs == nullptr || size == 0)  return 0;

#if !defined(_WIN64) && !defined(__amd64)
  const auto wl = gk_str2dword_len;
#else
  const auto wl = gk_str2qword_len;
#endif

  if((wantlen == 0) || (wantlen > wl))    wantlen = wl;

  const char* lp = (const char*)strs;
  for(size_t i = 0; i < size; ++i)
    {
    const char ch = lp[i];
    char tmpC = ch & 0xF;
    switch(ch)
      {
      case 'A':      case 'B':      case 'C':      case 'D':
      case 'E':      case 'F':
      case 'a':      case 'b':      case 'c':      case 'd':
      case 'e':      case 'f':
        {
        tmpC += 9;  //注意这里没有break
        }
      case '0':      case '1':      case '2':      case '3':
      case '4':      case '5':      case '6':      case '7':
      case '8':      case '9':
        {
        --wantlen;
        values = values << 0x4;
        values += tmpC;
        if(wantlen == 0)
          {
          ++(*lpreadlen);
          return values;
          }
        break;
        }
      default:
        if(errexit)
          {
          *lpreadlen = 0;
          return 0;
          }
        if(errbreak)  return values;
        break;
      }
    ++(*lpreadlen);
    }
  return values;
  }

string str2hexs(const void*   strs,
                const size_t  size,
                size_t*       lpreadlen,
                const bool    errexit,
                const bool    errbreak)
  {
  string rets;

  size_t readlen = 0;
  if(lpreadlen == nullptr) lpreadlen = &readlen;
  *lpreadlen = 0;

  if(strs == nullptr || size == 0)  return 0;

  bool pick_high = true;      //指示当前提取的是高位还是低位
  unsigned char readch = 0;   //存放临时的提取值
  size_t realreadlen = 0;     //存放实际读取数

  const char* lp = (const char*)strs;
  for(size_t i = 0; i < size; ++i)
    {
    const char ch = lp[i];
    char tmpC = ch & 0xF;
    switch(ch)
      {
      case 'A':      case 'B':      case 'C':      case 'D':
      case 'E':      case 'F':
      case 'a':      case 'b':      case 'c':      case 'd':
      case 'e':      case 'f':
        {
        tmpC += 9;  //注意这里没有break
        }
      case '0':      case '1':      case '2':      case '3':
      case '4':      case '5':      case '6':      case '7':
      case '8':      case '9':
        {
        if(pick_high)
          {
          readch = tmpC << 0x4;
          }
        else
          {
          readch = (readch & 0xF0) + tmpC;
          rets.push_back(readch);
          realreadlen = *(lpreadlen) + 1;
          }
        pick_high = !pick_high;
        break;
        }
      default:
        if(errexit)
          {
          *lpreadlen = 0;
          rets.clear();
          return rets;
          }
        if(errbreak)
          {
          //读取不完整
          if(!pick_high)
            {
            *(lpreadlen) = realreadlen;
            }
          return rets;
          }
        break;
      }
    ++(*lpreadlen);
    }

  return rets;
  }

string escape(const void* strs, const size_t size)
  {
  const char* lp = (const char*)strs;
  string msg;
  for(size_t i = 0; i < size; ++i)
    {
    if(lp[i] != '\\')
      {
      msg.push_back(lp[i]);
      continue;
      }

    switch(lp[++i])
      {
      case '0':   msg.push_back('\0');  break;
      case 'a':   msg.push_back('\a');  break;
      case 'b':   msg.push_back('\b');  break;
      case 'f':   msg.push_back('\f');  break;
      case 'n':   msg.push_back('\n');  break;
      case 'r':   msg.push_back('\r');  break;
      case 't':   msg.push_back('\t');  break;
      case 'v':   msg.push_back('\v');  break;
      case '\\':  msg.push_back('\\');  break;
      case '\'':  msg.push_back('\'');  break;
      case '\"':  msg.push_back('\"');  break;
      case '\?':  msg.push_back('\?');  break;
      case '\0':  msg.push_back('\\'); --i; break;
      case 'x' :
        {
        ++i;
        size_t readlen = 0;
        size_t tmpI = str2hex((void*)&lp[i], 1, &readlen, 0, false, true);
        switch(readlen)
          {
          case 0:msg.push_back('x');break;
          case 1:
          case 2:msg.append((const char*)&tmpI, sizeof(uint8));break;
          case 3:
          case 4:msg.append((const char*)&tmpI, sizeof(uint16)); break;
#if !defined(_WIN64) && !defined(__amd64)
          default:msg.append((const char*)&tmpI, sizeof(uint32));break;
#else
          case 5: case 6: case 7: case 8:
            msg.append((const char*)&tmpI, sizeof(uint32));break;
          default:
            msg.append((const char*)&tmpI, sizeof(uint64)); break;
#endif
          }
        i += (readlen - 1);
        }
        break;
      default:
        msg.push_back('\\');
        msg.push_back(lp[i]);
      }
    }
  return msg;
  }

//! 每行显示数据个数
static const intptr_t gk_max_line_byte = 0x10;

//! 输出hex2show的前缀格式化数据，注意会修改传入的size
static void hex2show_prefix(xmsg&                 msg,
                            const unsigned char*  data,
                            intptr_t&             size,
                            const char*           fmt,
                            const size_t          prews)
  {
  msg.append(prews, ' ');  //空格前缀
  msg << (void*)data;      //地址前缀
  msg.append("┃");        //地址与16进制数据分隔符

  for(intptr_t i = 0; i < gk_max_line_byte; --size)
    {
    if(size > 0)    // HEX格式化输出
      {
      msg.push_back(fmt[(data[i] >> 4) & 0xF]);
      msg.push_back(fmt[(data[i] >> 0) & 0xF]);
      }
    else            // 无数据补齐
      {
      msg.append("  ");
      }
    switch(++i)
      {
      case 4:  case 8:  case 0xC:
        msg.push_back('|'); break;
      case 0x10:
        msg.append("┃"); break;
      default:
        msg.push_back(' ');
      }
    }
  }

//! 使unicode字符输出可视化，返回true表示接受字符，否则不接受，一律输出'.'
static bool hex2show_unicode_visualization(xmsg& msg, const charucs2_t wc)
  {
  //控制字符一律输出'.'
  if(wc < L' ' || (wc >= 0x7F && wc <= 0xA0))
    {
    msg.push_back('.');
    return true;
    }

  //对于'?'做特殊预处理以应对转换无法识别字符
  if(wc == L'?')
    {
    msg.push_back('?');
    return true;
    }

  //尝试转换ASCII，失败则输出'.'
  const auto ch(ws2s(ucs2string(1, wc)));
  if(ch.empty())
    {
    msg.push_back('.');
    return false;
    }

  //转换成功，但无法显示，也输出'.'
  if(*ch.begin() == '?')
    {
    msg.push_back('.');
    return false;
    }

  //正常输出
  msg << ch;
  return true;
  }

//! 注意会修改传入的usechar
static void hex2show_unicode(xmsg&                msg,
                             const unsigned char* data,
                             const intptr_t       fix_len,
                             const bool           last,
                             size_t&              usechar)
  {

  intptr_t lp = 0;

  while(lp < fix_len)
    {
    //在数据最后一行的最后一个byte，无法进行向后匹配完整字符，一律输出'.'
    if(last && (lp + 1) >= fix_len)
      {
      msg.push_back('.');
      lp += sizeof(char);
      break;
      }

    const charucs2_t wc = *(charucs2_t*)&data[lp + usechar];

    if(hex2show_unicode_visualization(msg, wc))
      {
      lp += sizeof(charucs2_t);
      }
    else
      {
      lp += sizeof(char);
      }
    }

  usechar = 0;
  if(lp > fix_len)  usechar = lp - fix_len;
  }

//! 注意会修改传入的usechar
static void hex2show_utf8(xmsg&                 msg,
                          const unsigned char*  data,
                          const intptr_t        fix_len,
                          const bool            last,
                          size_t&               usechar)
  {
  intptr_t lp = 0;

  while(lp < fix_len)
    {
    //尝试转换unicode
    unsigned long unicode;
    const intptr_t k = utf8_byte2unicode_byte(&unicode, (const p_utf8)&data[lp + usechar]);

    //转换失败一律输出'.'
    if(k <= 0)
      {
      msg.push_back('.');
      lp += sizeof(char);
      continue;
      }

    //在数据最后一行，无法进行向后匹配完整字符，一律输出'.'
    if(last && (lp + k > fix_len))
      {
      msg.append(fix_len - lp, '.');
      lp = fix_len;
      break;
      }

    //转换超出正常unicode范围，一律输出'.'
    if(unicode > 0xFFFF)
      {
      msg.push_back('.');
      lp += sizeof(char);
      continue;
      }

    const charucs2_t wc = (charucs2_t)unicode;

    if(hex2show_unicode_visualization(msg, wc))
      {
      lp += k;
      }
    else
      {
      lp += sizeof(char);
      }
    }

  usechar = 0;
  if(lp > fix_len)  usechar = lp - fix_len;
  }

static void hex2show_ascii(xmsg&                msg,
                           const unsigned char* data,
                           const intptr_t       fix_len,
                           const bool           last,
                           size_t&              usechar)
  {
  intptr_t lp = 0;

  while(lp < fix_len)
    {
    const unsigned char ch = *(unsigned char*)&data[lp + usechar];

    //控制字符一律输出'.'
    if(ch < ' ' || ch == 0x7F)
      {
      msg.push_back('.');
      lp += sizeof(char);
      continue;
      }

    //可显示字符输出
    if(ch < 0x7F)
      {
      msg.push_back(ch);
      lp += sizeof(char);
      continue;
      }

    //在数据最后一行的最后一个byte，无法进行向后匹配完整字符，一律输出'.'
    if(last && (lp + 1) >= fix_len)
      {
      msg.push_back('.');
      lp += sizeof(char);
      break;
      }

    //尝试转换unicode，转换失败一律输出'.'
    const auto unicode(s2ws(string((const char*)&data[lp + usechar], sizeof(charucs2_t))));
    if(unicode.empty())
      {
      msg.push_back('.');
      lp += sizeof(char);
      continue;
      }

    if(hex2show_unicode_visualization(msg, *unicode.begin()))
      {
      lp += sizeof(charucs2_t);
      }
    else
      {
      lp += sizeof(char);
      }
    }

  usechar = 0;
  if(lp > fix_len)  usechar = lp - fix_len;
  }

string hex2show(const void*         data,
                const size_t        len,
                const Hex2showCode  code,
                const bool          isup,
                const size_t        prews)
  {
  xmsg msg;
  const char* fmt = isup ? gk_NumFmt_Up : gk_NumFmt_Low;

  size_t usechar = 0;
  const unsigned char* lpdata = (const unsigned char*)data;
  intptr_t size = len;

  do 
    {
    hex2show_prefix(msg, lpdata, size, fmt, prews);

    //默认情况下输出字符gk_max_line_byte大小，但需要去除之前预支的字符
    intptr_t fix_len = gk_max_line_byte - usechar;

    //在数据最后一行，输出大小需要控制
    const bool last = size < 0;
    if(last)
      {
      fix_len += size;
      }

    //严格错误判定
    if(fix_len >= 0)
      {
      switch(code)
        {
        case HC_UNICODE:
          hex2show_unicode(msg, lpdata, fix_len, last, usechar);
          break;
        case HC_UTF8:
          hex2show_utf8(msg, lpdata, fix_len, last, usechar);
          break;
        case HC_ASCII:
        default:
          hex2show_ascii(msg, lpdata, fix_len, last, usechar);
        }
      }

    lpdata += gk_max_line_byte;
    msg.push_back('\r');
    msg.push_back('\n');

    }while(size > 0);

  return msg;
  }

#ifdef _XLIB_TEST_

ADD_XLIB_TEST(HEX_STR)
  {
  SHOW_TEST_INIT;

  bool done = false;

  const char* const buf = "12345678";
  const char* const hex = "3132333435363738";
  string rets;

  SHOW_TEST_HEAD("hex2str");
  done = (hex2str(string(buf)) == hex);
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("str2hex");
  done = (0x12345678 == str2hex(string(buf)));
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("hex2show");
  rets = hex2show(string("0123456789ABCDEF"));
  rets.erase(rets.begin(), rets.begin() + sizeof(void*) * gk_str2byte_len);
  done = (rets == "┃30 31 32 33|34 35 36 37|38 39 41 42|43 44 45 46┃0123456789ABCDEF\r\n");
  SHOW_TEST_RESULT(done);
  }

#endif  // _XLIB_TEST_