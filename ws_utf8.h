/*!
  \file  ws_utf8.h
  \brief ws_utf8.h用于如UTF8与UNICODE相互转换

  - 转换使用自己实现的过程

  \version    3.1.1405.0711
  \note       For All

  \author     triones
  \date       2013-03-07
*/

#pragma once

#include "xlib_base.h"
#include <string>

typedef unsigned char*                    p_utf8;
typedef std::basic_string<unsigned char>  xutf8;

/*!
  转换一个unicode字符为一个utf8字符
  \param    utf8      utf8结果缓冲，可为nullptr
  \param    unicode   unicode字符
  \return             返回转换字节数，返回0表示失败
*/
size_t unicode_byte2utf8_byte(p_utf8               utf8,
                              const unsigned long  unicode);

/*!
  转换一个utf8字符为一个unicode字符，非法utf8字符或不完整utf8字符则无法转换
  \param    unicode   结果缓冲，可为nullptr
  \param    utf8      utf8字符指针
  \return             返回读取utf8字节数，返回0表示失败
*/
size_t utf8_byte2unicode_byte(unsigned long*   unicode,
                              const p_utf8     utf8);

//! UNICODE串转换UTF8串(版本一)
/*!
  \param  utf8    指向转换UTF8结果的缓冲区
  \param  max_utf8  指示转换UTF8结果的缓冲区的最大容量(以byte计)
  \param  ws      需要转换的UNICODE串
  \param  ws_len  需要转换的UNICODE串的长度(以宽字计)\n
                  ws_len < 0时，视ws为null结束的串
  \return         转换写入缓冲的字节数，失败返回0

  \code
    #include "ws_s.h"
    char str[40];
    if(ws2utf8(str, sizeof(str), L"文字") == 0)
      {
      cout << "ws2utf8转换出错，LastError：" << GetLastError();
      }
  \endcode
*/
size_t ws2utf8(p_utf8         utf8,
               const size_t   max_utf8,
               const wchar_t* ws,
               const size_t   ws_len = -1);

//! UNICODE串转换UTF8串(版本二)
/*!
  \param    ws  需要转换的UNICODE串
  \return       转换后的对应UTF8串对象

  \code
    #include "ws_s.h"
    xutf8 s(ws2utf8(L"文字"));
    if(s.empty())
      {
      cout << "ws2utf8转换出错，LastError：" << GetLastError();
      }
  \endcode
*/
xutf8 ws2utf8(const std::wstring& ws);

//! UTF8串转换UNICODE串(版本一)
/*!
  \param  ws      指向转换UNICODE结果的缓冲区
  \param  max_ws  指示转换UNICODE结果的缓冲区的最大容量(以宽字计)
  \param  utf8    需要转换的UTF8串
  \param  utf8_len  需要转换的UTF8串的长度(以byte计)\n
                  utf8_len < 0时，视s为null结束的串
  \return         转换成功与否
*/
size_t utf82ws(wchar_t*       ws,
               const size_t   max_ws,
               const p_utf8   utf8,
               const size_t   utf8_len = -1);

//! UTF8串转换UNICODE串(版本二)
/*!
  \param    utf8  需要转换的UTF8串
  \return         转换后的对应UNICODE串对象
*/
std::wstring utf82ws(const xutf8& utf8);
std::wstring utf82ws(const std::string& utf8);