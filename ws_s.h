/*!
  \file  ws_s.h
  \brief ws_s.h用于如ASCII与UNICODE相互转换

  - 使用内核函数转换，而不是使用转换API
  
  \version    5.2.1603.1409
  \note       For All

  \author     triones
  \date       2011-11-11
*/

#pragma once

#include "xlib_base.h"
#include <string>

//! UNICODE串转换ASCII串(版本一)
/*!
  \param  s       指向转换ASCII结果的缓冲区
  \param  max_s   指示转换ASCII结果的缓冲区的最大容量(以byte计)
  \param  ws      需要转换的UNICODE串
  \param  ws_len  需要转换的UNICODE串的长度(以宽字计)\n
                  ws_len < 0时，视ws为null结束的串
  \return         转换后的ASCII数

  \code
    #include "ws_s.h"
    char str[40];
    if(!ws2s(str, sizeof(str), L"文字"))
      {
      cout << "ws2s转换出错，LastError：" << GetLastError();
      }
    else
      {
      cout << "转换结果：" << str;
      }
  \endcode
*/
size_t ws2s(char*           s,
            const size_t    max_s,
            const wchar_t*  ws,
            const size_t    ws_len = -1);

//! UNICODE串转换ASCII串(版本二)
/*!
  \param  ws  需要转换的UNICODE串
  \return     转换后的对应ASCII串对象

  \code
    #include "ws_s.h"
    string s(ws2s(L"文字"));
    if(s.empty())
      {
      cout << "ws2s转换出错，LastError：" << GetLastError();
      }
    else
      {
      cout << "转换结果：" << s;
      }
    //或者，可以如下操作
    cout << "转换结果：" << ws2s(L"文字");
  \endcode
*/
std::string ws2s(const std::wstring& ws);

//! ASCII串转换UNICODE串(版本一)
/*!
  \param  ws      指向转换UNICODE结果的缓冲区
  \param  max_ws  指示转换UNICODE结果的缓冲区的最大容量(以宽字计)
  \param  s       需要转换的ASCII串
  \param  s_len   需要转换的ASCII串的长度(以宽字计)\n
                  s_len < 0时，视s为null结束的串
  \return         转换成功后的UNICODE数

  \code
    #include "ws_s.h"
    wchar_t str[40];
    if(!s2ws(str, _countof(str), "文字"))
      {
      wcout << L"s2ws转换出错，LastError：" << GetLastError();
      }
    else
      {
      wcout << L"转换结果：" << str;
      }
  \endcode
*/
size_t s2ws(wchar_t*        ws,
            const size_t    max_ws,
            const char*     s,
            const size_t    s_len = -1);

//! ASCII串转换UNICODE串(版本二)
/*!
  \param  s   需要转换的ASCII串
  \return     转换后的对应UNICODE串对象

  \code
    #include "ws_s.h"
    wstring ws(s2ws("文字"));
    if(ws.empty())
      {
      wcout << L"s2ws转换出错，LastError：" << GetLastError();
      }
    else
      {
      wcout << L"转换结果：" << ws;
      }
    //或者，可以如下操作
    wcout << L"转换结果：" << s2ws(L"文字");
  \endcode
*/
std::wstring s2ws(const std::string& s);