﻿/*!
  \file  md5.h
  \brief md5.h定义了md5算法

  \version    2.0.1612.1614
  \note       For All

  \author     triones
  \date       2013-01-04
*/
#ifndef _XLIB_MD5_H_
#define _XLIB_MD5_H_

#include <string>

#include "xlib_base.h"

typedef uint8   MD5_BYTE;
typedef uint32  MD5_DWORD;

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4201)   //warning C4201: 使用了非标准扩展 : 无名称的结构/联合
#endif
//! MD5值是一个128位的散列值，由四个32位分组级联组成
class MD5_VALUE
  {
  public:
    MD5_VALUE();
    MD5_VALUE(const MD5_DWORD a,
              const MD5_DWORD b,
              const MD5_DWORD c,
              const MD5_DWORD d);
    MD5_VALUE(const void* data);
    operator std::string();
  public:
    union
      {
      struct
        {
        MD5_DWORD A;
        MD5_DWORD B;
        MD5_DWORD C;
        MD5_DWORD D;
        };
      MD5_BYTE Data[sizeof(MD5_DWORD) * 4];
      };
  };
#ifdef _WIN32
#pragma warning(pop)
#endif

//! md5算法
/*!
  \param data   需要计算MD5的数据指针
  \param size   需要计算MD5的数据大小
  \return       返回md5结果

  \code
    string aa;
    while(cin >> aa)
      {
      auto v = md5(aa);
      cout << hex2str(string(v)) << endl;
      }
    //MD5 ("") = d41d8cd98f00b204e9800998ecf8427e
　　//MD5 ("a") = 0cc175b9c0f1b6a831c399e269772661
　　//MD5 ("abc") = 900150983cd24fb0d6963f7d28e17f72
　　//MD5 ("message digest") = f96b697d7cb7938d525a2f31aaf161d0
　　//MD5 ("abcdefghijklmnopqrstuvwxyz") = c3fcd3d76192e4007dfb496cca67e13b
　　//MD5 ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz") =
　　//f29939a25efabaef3b87e2cbfe641315
  \endcode
*/
MD5_VALUE md5(const void* data, const size_t size);

//////////////////////////////////////////////////////////////////////////
template<typename T> MD5_VALUE md5(const std::basic_string<T>& s)
  {
  return md5(s.c_str(), s.size() * sizeof(T));
  }

#endif  // _XLIB_MD5_H_