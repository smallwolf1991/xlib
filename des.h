/*!
  \file  des.h
  \brief des.h定义了des算法

  - 参考来自http://pangzi.is-programmer.com/posts/25161.html
  - 参考来自http://www.cnblogs.com/erwin/archive/2009/04/14/1435288.html

  \version    1.0.1504.2814
  \note       For All

  \author     triones
  \date       2013-10-21
*/

#pragma once

#include "xline.h"

//! DES加密
/*!
  \param encrypt_data        需要加密的数据指针
  \param encrypt_data_size   需要加密数据的长度 >=1
  \param encrypt_key         参与加密数据的密钥
  \return                         返回数据加密结果，当param2 < 1时，返回空串

  \code
    string aa;
    while(cin>>aa)
      {
      line v = DesEncrypt(aa.c_str(),aa.size(),key);
      cout << hex2show(v) << endl;
      }
  \endcode
*/
line DesEncrypt(const void*     encrypt_data,
                const size_t    encrypt_data_size,
                const char*     encrypt_key);

//! DES加密
/*!
  \param decrypt_data        需要解密的数据指针
  \param decrypt_data_size   需要解密数据的长度
  \param decrypt_key         参与解密数据的密钥
  \return                         返回数据解密结果

  \code
    line v = DesDecrypt(aa.c_str(), aa.size(), key);
    cout << hex2show(v) << endl;
  \endcode
*/
line DesDecrypt(const void*     decrypt_data,
                const size_t    decrypt_data_size,
                const char*     decrypt_key);