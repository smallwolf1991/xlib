/*!
  \file  des.h
  \brief des.h定义了des算法

  - 参考来自http://pangzi.is-programmer.com/posts/25161.html
  - 参考来自http://www.cnblogs.com/erwin/archive/2009/04/14/1435288.html

  \version    1.1.1607.1917
  \note       For All

  \author     triones
  \date       2013-10-21
*/

#pragma once

#include "xlib_base.h"
#include <string>

//! DES加密
/*!
  \param data         需要加密的数据指针
  \param size         需要加密数据的长度 >=1
  \param key          参与加密数据的密钥，密钥要求8 byte( 64 bit )
  \return             返回数据加密结果，当param2 < 1时，返回空串

  \code
    string aa;
    while(cin>>aa)
      {
      auto v = DesEncrypt(aa.c_str(),aa.size(), key);
      cout << hex2show(v) << endl;
      }
  \endcode
*/
std::string DesEncrypt(const void*     data,
                       const size_t    size,
                       const char*     key);

//! DES解密
/*!
  \param data        需要解密的数据指针
  \param size        需要解密数据的长度
  \param key         参与解密数据的密钥，密钥要求8 byte( 64 bit )
  \return            返回数据解密结果

  \code
    auto v = DesDecrypt(aa.c_str(), aa.size(), key);
    cout << hex2show(v) << endl;
  \endcode
*/
std::string DesDecrypt(const void*     data,
                       const size_t    size,
                       const char*     key);

//////////////////////////////////////////////////////////////////////////
template<typename T>
std::string DesEncrypt(const std::basic_string<T>&    data,
                       const char*                    key)
  {
  return DesEncrypt(data.c_str(), data.size() * sizeof(T), key);
  }

template<typename T>
std::string DesDecrypt(const std::basic_string<T>&    data,
                       const char*                    key)
  {
  return DesDecrypt(data.c_str(), data.size() * sizeof(T), key);
  }