/*!
  \file  aes.h
  \brief aes.h定义了aes算法

  - 参考来自http://mingcn.cnblogs.com/archive/2010/10/31/aes_c.html
  - 源作者：xelz

  \version    1.0.1310.2109
  \note       For All

  \author     triones
  \date       2013-10-21
*/

#pragma once

#include "xline.h"

class AesKey
  {
  public:
    //! 指定key串，如果数据不足，其后以0补充
    AesKey(const void* key,size_t len = 0);
  public:
    unsigned char _key[0x4*0x4];
  };

//! AES加密
/*!
  \param encrypt_data          需要加密的数据指针
  \param encrypt_data_size     需要加密数据的长度 >=1
  \param encrypt_key           参与加密数据的密钥
  \return                      返回数据加密结果，当param2 < 1时，返回空串

  \code
    string aa;
    while(cin>>aa)
      {
      line v = AesEncrypt(aa.c_str(),aa.size(),key);
      cout << hex2show(v) << endl;
      }
  \endcode
*/
line AesEncrypt(const void*     encrypt_data,
                const size_t    encrypt_data_size,
                const AesKey&   encrypt_key);

//! AES加密
/*!
  \param decrypt_data          需要解密的数据指针
  \param decrypt_data_size     需要解密数据的长度
  \param decrypt_key           参与解密数据的密钥
  \return                      返回数据解密结果

  \code
    netline v = AesDecrypt(aa.c_str(),aa.size(),key);
    cout << hex2show(v) << endl;
  \endcode
*/
line AesDecrypt(const void*     decrypt_data,
                const size_t    decrypt_data_size,
                const AesKey&   decrypt_key);