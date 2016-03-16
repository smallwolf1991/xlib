/*!
  \file  tean.h
  \brief tean.h定义了TEAN(TX Protocol)算法

  \section description 算法描述
  - 算法在TEA算法基础上增加了交织填充及交织加密技术。
  - 原始数据大小n，数据结尾附加7字节的0，n+7；
  - 最后一个字节的原始数据与之成组，一组数据8字节。n+7-8；
  - 剩余数据从尾部开始成组。(n+7-8)/8；
  - 余下未能成组的数据用最少3字节随机数填充，成一或二组数据头。((n+7-8)%8+3+x)%8==0；
  - 第一字节最低三位写入补充填充数x(0~7)；
  - 每组数据与上组密文异或，data ^= cipher；
  - 结果用TEA算法加密，cipher = TEA_E(data,key)；
  - 结果与上组运算后的数据异或得到密文，cipher ^= pre_data；
  - 循环加密。加密后的密文至少16字节，长度len%8==0；
  - 解密过程反之。上组运算后数据与密文异或，pre_data ^= cipher；
  - 结果用TEA算法解密，pre_data = TEA_D(pre_data,key)；
  - 明文由结果与上组密文异或得到，data = pre_data ^ pre_cipher

  \version    2.1.1603.1111
  \note       For All

  \author     triones
  \date       2013-01-08
*/
#pragma once

#include "xline.h"

typedef DWORD TEAN_UL;  //!< TEAN算法使用32bit

#pragma warning(push)
#pragma warning(disable:4201)   //warning C4201: 使用了非标准扩展 : 无名称的结构/联合
//! 分组数据，两个32 bit值
class TEAN_DATA
  {
  public:
    TEAN_DATA();
    TEAN_DATA(const TEAN_UL a,const TEAN_UL b);
    TEAN_DATA(const void* datas);
  public:
    union
      {
      struct
        {
        TEAN_UL A;
        TEAN_UL B;
        };
      unsigned char Data[sizeof(TEAN_UL) * 2];
      };
  };

//! 密钥结构，128 bit
class TEAN_KEY
  {
  public:
    TEAN_KEY();
    TEAN_KEY(const TEAN_UL k1,
             const TEAN_UL k2,
             const TEAN_UL k3,
             const TEAN_UL k4);
    TEAN_KEY(const void* key);
  public:
    union
      {
      struct
        {
        TEAN_UL K1;
        TEAN_UL K2;
        TEAN_UL K3;
        TEAN_UL K4;
        };
      unsigned char Key[sizeof(TEAN_UL) * 4];
      };
  };
#pragma warning(pop)

TEAN_DATA TeaEncipher(const TEAN_DATA&    encrypt_data,
                      const TEAN_KEY&     key,
                      const TEAN_UL       delta,
                      const size_t        xtea_round);

TEAN_DATA TeaDecipher(const TEAN_DATA&    decrypt_data,
                      const TEAN_KEY&     key,
                      const TEAN_UL       delta,
                      const size_t        xtea_round);

netline TeaEncrypt(const void*        encrypt_data,
                   const size_t       encrypt_data_size,
                   const TEAN_KEY&    encrypt_key,
                   const TEAN_UL      delta,
                   const size_t       xtea_round);

netline TeaDecrypt(const void*        decrypt_data,
                   const size_t       decrypt_data_size,
                   const TEAN_KEY&    decrypt_key,
                   const TEAN_UL      delta,
                   const size_t       xtea_round);

//! 分组数据加密，返回加密结果
TEAN_DATA TeanEncipher(const TEAN_DATA&   encrypt_data,
                       const TEAN_KEY&    key);

//! 分组数据解密，返回解密结果
TEAN_DATA TeanDecipher(const TEAN_DATA&   decrypt_data,
                       const TEAN_KEY&    key);

//! TEAN加密
/*!
  \param encrypt_data        需要加密的数据指针
  \param encrypt_data_size   需要加密数据的长度 >=1
  \param encrypt_key         参与加密数据的密钥
  \return                       返回数据加密结果，当param2 < 1时，返回空串

  \code
    string aa;
    while(cin>>aa)
      {
      netline v = TeanEncrypt(aa.c_str(),aa.size(),key);
      cout << hex2show(v) << endl;
      }
  \endcode
*/
netline TeanEncrypt(const void*       encrypt_data,
                    const size_t      encrypt_data_size,
                    const TEAN_KEY&   encrypt_key);

//! TEAN解密
/*!
  \param decrypt_data        需要解密的数据指针
  \param decrypt_data_size   需要解密数据的长度，>=16且%8==0
  \param decrypt_key         参与解密数据的密钥
  \return                       返回数据解密结果，当param2不符合要求或数据头错误时，返回空串

  \code
    netline v = TeanDecrypt(aa.c_str(),aa.size(),key);
    cout << hex2show(v);
  \endcode
*/
netline TeanDecrypt(const void*       decrypt_data,
                    const size_t      decrypt_data_size,
                    const TEAN_KEY&   decrypt_key);

netline XTeanEncrypt(const void*        encrypt_data,
                     const size_t       encrypt_data_size,
                     const TEAN_KEY&    encrypt_key);

netline XTeanDecrypt(const void*        decrypt_data,
                     const size_t       decrypt_data_size,
                     const TEAN_KEY&    decrypt_key);

//! XxTea算法使用的Word
typedef unsigned long XXTEA_DATA;

//! XXTEA加密
/*!
  \param encrypt_data        需要加密的数据指针
  \param encrypt_data_size   需要加密数据的长度，注意XXTEA需要长度为long的倍数
  \param encrypt_key         参与加密数据的密钥
  \return                       返回数据加密结果，当param2 < 4时，返回空串

  \code
    string aa;
    while(cin>>aa)
      {
      auto v = XxTeaEncrypt(aa.c_str(),aa.size(),key);
      cout << hex2show(v) << endl;
      }
  \endcode
  */
std::string XxTeaEncrypt(const void*        encrypt_data,
                         const size_t       encrypt_data_size,
                         const TEAN_KEY&    encrypt_key);
//! XXTEA解密
/*!
  \param decrypt_data        需要解密的数据指针
  \param decrypt_data_size   需要解密数据的长度，注意XXTEA需要长度为long的倍数
  \param decrypt_key         参与解密数据的密钥
  \return                       返回数据解密结果，当param2 < 4时，返回空串

  \code
    auto v = XxTeaDecrypt(aa.c_str(),aa.size(),key);
    cout << hex2show(v);
  \endcode
  */
std::string XxTeaDecrypt(const void*        decrypt_data,
                         const size_t       decrypt_data_size,
                         const TEAN_KEY&    decrypt_key);