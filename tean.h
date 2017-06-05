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
  - 结果用TEA算法加密，cipher = TEA_E(data, key)；
  - 结果与上组运算后的数据异或得到密文，cipher ^= pre_data；
  - 循环加密。加密后的密文至少16字节，长度len%8==0；
  - 解密过程反之。上组运算后数据与密文异或，pre_data ^= cipher；
  - 结果用TEA算法解密，pre_data = TEA_D(pre_data, key)；
  - 明文由结果与上组密文异或得到，data = pre_data ^ pre_cipher

  \version    3.1.1706.0215
  \note       For All

  \author     triones
  \date       2013-01-08
*/
#ifndef _XLIB_TEAN_H_
#define _XLIB_TEAN_H_

#include <string>

#include "xlib_base.h"

typedef uint32  TEAN_WORD;  //!< TEAN算法使用32bit

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4201)   //warning C4201: 使用了非标准扩展 : 无名称的结构/联合
#endif
//! 分组数据，两个32 bit值
class TEAN_DATA
  {
  public:
    TEAN_DATA();
    TEAN_DATA(const TEAN_WORD a, const TEAN_WORD b);
    TEAN_DATA(const void* datas);
    template<typename T> operator std::basic_string<T>()
      {
      return std::basic_string<T>((const T*)Data, sizeof(Data) / sizeof(T));
      }
  public:
    union
      {
      struct
        {
        TEAN_WORD A;
        TEAN_WORD B;
        };
      unsigned char Data[sizeof(TEAN_WORD) * 2];
      };
  };

//! 密钥结构，128 bit
class TEAN_KEY
  {
  public:
    TEAN_KEY();
    TEAN_KEY(const TEAN_WORD k1,
             const TEAN_WORD k2,
             const TEAN_WORD k3,
             const TEAN_WORD k4);
    TEAN_KEY(const void* key, size_t size = 0);
    template<typename T> TEAN_KEY(const std::basic_string<T>& key)
      {
      memset(Key, 0, sizeof(Key));
      size_t size = key.size() * sizeof(T);
      if(size > sizeof(Key)) size = sizeof(Key);
      memcpy(Key, key.c_str(), size);
      }
  public:
    union
      {
      struct
        {
        TEAN_WORD K1;
        TEAN_WORD K2;
        TEAN_WORD K3;
        TEAN_WORD K4;
        };
      unsigned char Key[sizeof(TEAN_WORD) * 4];
      };
  };
#ifdef _WIN32
#pragma warning(pop)
#endif  // _WIN32

TEAN_DATA TeaEncipher(const TEAN_DATA&    data,
                      const TEAN_KEY&     key,
                      const TEAN_WORD     delta,
                      const size_t        xtea_round);

TEAN_DATA TeaDecipher(const TEAN_DATA&    data,
                      const TEAN_KEY&     key,
                      const TEAN_WORD     delta,
                      const size_t        xtea_round);

std::string TeaEncrypt(const void*        data,
                       const size_t       size,
                       const TEAN_KEY&    key,
                       const TEAN_WORD    delta,
                       const size_t       xtea_round);

std::string TeaDecrypt(const void*        data,
                       const size_t       size,
                       const TEAN_KEY&    key,
                       const TEAN_WORD    delta,
                       const size_t       xtea_round);

//! 分组数据加密，返回加密结果
TEAN_DATA TeanEncipher(const TEAN_DATA& data, const TEAN_KEY& key);

//! 分组数据解密，返回解密结果
TEAN_DATA TeanDecipher(const TEAN_DATA& data, const TEAN_KEY& key);

//! TEAN加密
/*!
  \param data        需要加密的数据指针
  \param size        需要加密的数据大小
  \param key         参与加密数据的密钥
  \return            返回数据加密结果

  \code
    string aa;
    while(cin >> aa)
      {
      auto v = TeanEncrypt(aa, key);
      cout << hex2show(v);
      }
  \endcode
*/
std::string TeanEncrypt(const void* data, const size_t size, const TEAN_KEY& key);

//! TEAN解密
/*!
  \param data        需要解密的数据指针
  \param size        需要加密的数据大小(长度>=16且%8==0)
  \param key         参与解密数据的密钥
  \return            返回数据解密结果，当长度不符合要求或数据头错误时，返回空串

  \code
    auto v = TeanDecrypt(aa, key);
    cout << hex2show(v);
  \endcode
*/
std::string TeanDecrypt(const void* data, const size_t size, const TEAN_KEY& key);

std::string XTeanEncrypt(const void* data, const size_t size, const TEAN_KEY& key);

std::string XTeanDecrypt(const void* data, const size_t size, const TEAN_KEY& key);

//! XxTea算法使用的Word
typedef uint32 XXTEA_DATA;

//! XXTEA加密
/*!
  \param data        需要加密的数据指针
  \param size        需要加密的数据大小(注意XXTEA需要长度为int的倍数且>1，不成块部分则丢弃)
  \param key         参与加密数据的密钥
  \return            返回数据加密结果

  \code
    string aa;
    while(cin >> aa)
      {
      auto v = XxTeaEncrypt(aa, key);
      cout << hex2show(v) << endl;
      }
  \endcode
  */
std::string XxTeaEncrypt(const void* data, const size_t size, const TEAN_KEY& key);

//! XXTEA解密
/*!
  \param data        需要解密的数据指针
  \param size        需要加密的数据大小(注意XXTEA需要长度为int的倍数且>1，不成块部分则不加入处理)
  \param key         参与解密数据的密钥
  \return            返回数据解密结果，当param2 < 4时，返回空串

  \code
    auto v = XxTeaDecrypt(aa, key);
    cout << hex2show(v);
  \endcode
  */
std::string XxTeaDecrypt(const void* data, const size_t size, const TEAN_KEY& key);

//////////////////////////////////////////////////////////////////////////
template<typename T>
std::string TeaEncrypt(const std::basic_string<T>&    data,
                       const TEAN_KEY&                key,
                       const TEAN_WORD                delta,
                       const size_t                   xtea_round)
  {
  return TeaEncrypt(data.c_str(), data.size() * sizeof(T), key, delta, xtea_round);
  }

template<typename T>
std::string TeaDecrypt(const std::basic_string<T>&    data,
                       const TEAN_KEY&                key,
                       const TEAN_WORD                delta,
                       const size_t                   xtea_round)
  {
  return TeaDecrypt(data.c_str(), data.size() * sizeof(T), key, delta, xtea_round);
  }

template<typename T>
std::string TeanEncrypt(const std::basic_string<T>&   data,
                        const TEAN_KEY&               key)
  {
  return TeanEncrypt(data.c_str(), data.size() * sizeof(T), key);
  }

template<typename T>
std::string TeanDecrypt(const std::basic_string<T>&   data,
                        const TEAN_KEY&               key)
  {
  return TeanDecrypt(data.c_str(), data.size() * sizeof(T), key);
  }

template<typename T>
std::string XTeanEncrypt(const std::basic_string<T>&  data,
                         const TEAN_KEY&              key)
  {
  return XTeanEncrypt(data.c_str(), data.size() * sizeof(T), key);
  }

template<typename T>
std::string XTeanDecrypt(const std::basic_string<T>&  data,
                         const TEAN_KEY&              key)
  {
  return XTeanDecrypt(data.c_str(), data.size() * sizeof(T), key);
  }

template<typename T>
std::string XxTeaEncrypt(const std::basic_string<T>&  data,
                         const TEAN_KEY&              key)
  {
  return XxTeaEncrypt(data.c_str(), data.size() * sizeof(T), key);
  }

template<typename T>
std::string XxTeaDecrypt(const std::basic_string<T>&  data,
                         const TEAN_KEY&              key)
  {
  return XxTeaDecrypt(data.c_str(), data.size() * sizeof(T), key);
  }

#endif  // _XLIB_TEAN_H_