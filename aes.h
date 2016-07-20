/*!
  \file  aes.h
  \brief aes.h定义了aes算法

  - 参考来自http://mingcn.cnblogs.com/archive/2010/10/31/aes_c.html
  - 源作者：xelz

  \version    1.1.1607.1917
  \note       For All

  \author     triones
  \date       2013-10-21
*/

#pragma once

#include "xlib_base.h"
#include <string>

class AesKey
  {
  public:
    /*!
      指定key串。16 byte （128 bit）\n
      当len == 0时，视key为0结尾的ASCII字符串\n
      如果数据不足，其后以0补充
    */
    AesKey(const void* key,size_t len = 0);
  public:
    unsigned char _key[0x4*0x4];
  };

//! AES加密
/*!
  \param data           需要加密的数据指针
  \param size           需要加密数据的长度 >=1
  \param key            参与加密数据的密钥
  \return               返回数据加密结果，当param2 < 1时，返回空串

  \code
    string aa;
    while(cin>>aa)
      {
      auto v = AesEncrypt(aa.c_str(),aa.size(), key);
      cout << hex2show(v);
      }
  \endcode
*/
std::string AesEncrypt(const void*          data,
                       const size_t         size,
                       const AesKey&        key);

//! AES解密
/*!
  \param data           需要解密的数据指针
  \param size           需要解密数据的长度
  \param key            参与解密数据的密钥
  \return               返回数据解密结果

  \code
    auto v = AesDecrypt(aa.c_str(),aa.size(), key);
    cout << hex2show(v);
  \endcode
*/
std::string AesDecrypt(const void*          data,
                       const size_t         size,
                       const AesKey&        key);

//////////////////////////////////////////////////////////////////////////
template<typename T>
std::string AesEncrypt(const std::basic_string<T>&    data,
                       const AesKey&                  key)
  {
  return AesEncrypt(data.c_str(), data.size() * sizeof(T), key);
  }

template<typename T>
std::string AesDecrypt(const std::basic_string<T>&    data,
                       const AesKey&                  key)
  {
  return AesDecrypt(data.c_str(), data.size() * sizeof(T), key);
  }