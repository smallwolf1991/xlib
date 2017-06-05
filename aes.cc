﻿#include "aes.h"

#include <string.h>

using std::string;

AesKey::AesKey(const void* key, size_t size)
  {
  memset(_key, 0, sizeof(_key));
  if(size == 0)
    {
    const char* k = (const char*)key;
    while(k[size] != '\0') ++size;
    }
  if(size > sizeof(_key))  size = sizeof(_key);
  memcpy(_key, key, size);
  }

//! AES置换表
static const unsigned char sBox[] =
  {
  //0    1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
  0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76, //0
  0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0, //1
  0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15, //2
  0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75, //3
  0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84, //4
  0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF, //5
  0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8, //6
  0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2, //7
  0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73, //8
  0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB, //9
  0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79, //A
  0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08, //B
  0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A, //C
  0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E, //D
  0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF, //E
  0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16  //F
  };

//! AES逆置换表
static const unsigned char invsBox[] = 
  {
  //0    1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
  0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB, //0
  0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB, //1
  0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E, //2
  0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25, //3
  0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92, //4
  0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84, //5
  0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06, //6
  0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B, //7
  0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73, //8
  0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E, //9
  0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B, //A
  0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4, //B
  0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F, //C
  0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF, //D
  0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61, //E
  0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D  //F
  }; 

//! 每行字节数
static const size_t bytes_columns_size = 4;

//! 每列字节数
static const size_t bytes_row_size = 4;

//! 每组数据字节数
static const size_t bytes_block_size = bytes_row_size * bytes_columns_size;

//! 扩展密钥组数
static const size_t expand_key_size = 11;

//! 扩展密钥轮常量
static const unsigned char expand_key_round_const[expand_key_size] =
  {
  0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36
  };

//! 非线性字节替代
static void SubstituteBytes(unsigned char data[bytes_row_size][bytes_columns_size])
  {
  for(size_t row = 0; row < bytes_row_size; ++row)
    {
    for(size_t col = 0; col < bytes_columns_size; ++col)
      {
      auto& v = data[row][col];
      v = sBox[v];
      }
    }
  }

static void InvSubstituteBytes(unsigned char data[bytes_row_size][bytes_columns_size])
  {
  for(size_t row = 0; row < bytes_row_size; ++row)
    {
    for(size_t col = 0; col < bytes_columns_size; ++col)
      {
      auto& v = data[row][col];
      v = invsBox[v];
      }
    }
  }

//! 行移位变换
static void ShiftRows(unsigned char data[bytes_row_size][bytes_columns_size])
  {
  unsigned char v[bytes_columns_size];
  // 第一组不变换
  for(size_t row = 1; row < bytes_row_size; ++row)
    {
    for(size_t col = 0; col < bytes_columns_size; ++col)
      {
      v[col] = data[row][(col + row) % bytes_columns_size];
      }
    for(size_t col = 0; col < bytes_columns_size; ++col)
      {
      data[row][col] = v[col];
      }
    }
  }

static void InvShiftRows(unsigned char data[bytes_row_size][bytes_columns_size])
  {
  unsigned char v[bytes_columns_size];
  // 第一组不变换
  for(size_t row = 1; row < bytes_row_size; ++row)
    {
    for(size_t col = 0; col < bytes_columns_size; ++col)
      {
      v[col] = data[row][(col - row + bytes_columns_size) % bytes_columns_size];
      }
    for(size_t col = 0; col < bytes_columns_size; ++col)
      {
      data[row][col] = v[col];
      }
    }
  }

//! 矩阵变换算法
static unsigned char FFmul(const unsigned char a, const unsigned char b)
  {
  unsigned char v[bytes_columns_size];
  unsigned char ret = 0;
  // 由b生成一组数据
  v[0] = b;
  for(size_t col = 1; col < bytes_columns_size; ++col)
    {
    v[col] = v[col - 1] << 1;
    if(v[col - 1] & 0x80)
      {
      v[col] ^= 0x1B;
      }
    }
  // 由a计算出结果
  for(size_t col = 0; col < bytes_columns_size; ++col)
    {
    if((a >> col) & 0x01)
      {
      ret ^= v[col];
      }
    }
  return ret;
  }

//! 列混淆变换
static void MixColumns(unsigned char data[bytes_row_size][bytes_columns_size])
  {
  unsigned char v[bytes_columns_size];
  for(size_t col = 0; col < bytes_columns_size; ++col)
    {
    //提取一列数据
    for(size_t row = 0; row < bytes_row_size; ++row)
      {
      v[row] = data[row][col];
      }
    //列混淆
    for(size_t row = 0; row < bytes_row_size; ++row)
      {
      data[row][col] = FFmul(0x02, v[row])
        ^ FFmul(0x03, v[(row + 1) % bytes_row_size])
        ^ FFmul(0x01, v[(row + 2) % bytes_row_size])
        ^ FFmul(0x01, v[(row + 3) % bytes_row_size]);
      }
    }
  }

static void InvMixColumns(unsigned char data[bytes_row_size][bytes_columns_size])
  {
  unsigned char v[bytes_columns_size];
  for(size_t col = 0; col < bytes_columns_size; ++col)
    {
    //提取一列数据
    for(size_t row = 0; row < bytes_row_size; ++row)
      {
      v[row] = data[row][col];
      }
    //列混淆
    for(size_t row = 0; row < bytes_row_size; ++row)
      {
      data[row][col] = FFmul(0x0e, v[row])
        ^ FFmul(0x0B, v[(row + 1) % bytes_row_size])
        ^ FFmul(0x0D, v[(row + 2) % bytes_row_size])
        ^ FFmul(0x09, v[(row + 3) % bytes_row_size]);
      }
    }
  }

//! 轮密钥加变换
static void AddRoundKey(unsigned char       data[bytes_row_size][bytes_columns_size],
                        const unsigned char key[bytes_row_size][bytes_columns_size])
  {
  for(size_t row = 0; row < bytes_row_size; ++row)
    {
    for(size_t col = 0; col < bytes_columns_size; ++col)
      {
      data[row][col] ^= key[row][col];
      }
    }
  }

//! 密钥扩展，将输入的密钥扩展为11组128位密钥组
static void KeyExpansion(const unsigned char* const key,
                         unsigned char              expandkey[expand_key_size][bytes_row_size][bytes_columns_size])
  {
  //第0组为输入密钥本身
  for(size_t row = 0; row < bytes_row_size; ++row)
    {
    for(size_t col = 0; col < bytes_columns_size; ++col)
      {
      expandkey[0][row][col] = key[row + col * bytes_row_size];
      }
    }
  //其后第n组第i列 为 第n-1组第i列 与 第n组第i-1列之和（模2加法，1<= i <=3）
  for(size_t i = 1; i < expand_key_size; ++i)
    {
    for(size_t col = 0; col < bytes_columns_size; ++col)
      {
      unsigned char v[bytes_columns_size];

      for(size_t row = 0; row < bytes_row_size; ++row)
        {
        v[row] = col ? expandkey[i][row][col - 1] : expandkey[i - 1][row][3];
        }

      if(col == 0)
        {
        const auto t = v[0];
        for(size_t row = 0; row < bytes_row_size - 1; ++row)
          {
          v[row] = sBox[ v[(row + 1) % bytes_row_size] ];
          }
        v[3] = sBox[t];
        v[0] ^= expand_key_round_const[i];
        }

      for(size_t row = 0; row < bytes_row_size; ++row)
        {
        expandkey[i][row][col] = expandkey[i - 1][row][col] ^ v[row];
        }
      }
    }
  }

string AesEncrypt(const void*     encrypt_data,
                  const size_t    encrypt_size,
                  const AesKey&   encrypt_key)
  {
  unsigned char data[bytes_row_size][bytes_columns_size];
  unsigned char expandkey[expand_key_size][bytes_row_size][bytes_columns_size];
  unsigned char okdata[bytes_block_size];
  string ret;
  const unsigned char* lp_encrypt = (const unsigned char*)encrypt_data;

  KeyExpansion(encrypt_key._key, expandkey);

  for(size_t encrypted = bytes_block_size;
      encrypted <= encrypt_size;
      encrypted += bytes_block_size)
    {
    for(size_t row = 0; row < bytes_row_size; ++row)
      {
      for(size_t col = 0; col < bytes_columns_size; ++col)
        {
        data[row][col] = lp_encrypt[row + col * bytes_columns_size];
        }
      }

    AddRoundKey(data, expandkey[0]);

    for(size_t i = 1; i < expand_key_size; ++i)
      {
      SubstituteBytes(data);
      ShiftRows(data);
      if(i != (expand_key_size - 1))
        MixColumns(data);
      AddRoundKey(data, expandkey[i]);
      }

    for(size_t row = 0; row < bytes_row_size; ++row)
      {
      for(size_t col = 0; col < bytes_columns_size; ++col)
        {
        okdata[row + col * bytes_columns_size] = data[row][col];
        }
      }
    ret.append((const char*)okdata, sizeof(okdata));
    lp_encrypt += bytes_block_size;
    }

  return ret;
  }

string AesDecrypt(const void*     decrypt_data,
                  const size_t    decrypt_size,
                  const AesKey&   decrypt_key)
  {
  unsigned char data[bytes_row_size][bytes_columns_size];
  unsigned char expandkey[expand_key_size][bytes_row_size][bytes_columns_size];
  unsigned char okdata[bytes_block_size];
  string ret;
  const unsigned char* lp_decrypt = (const unsigned char*)decrypt_data;

  KeyExpansion(decrypt_key._key, expandkey);

  for(size_t encrypted = bytes_block_size;
      encrypted <= decrypt_size;
      encrypted += bytes_block_size)
    {
    for(size_t row = 0; row < bytes_row_size; ++row)
      {
      for(size_t col = 0; col < bytes_columns_size; ++col)
        {
        data[row][col] = lp_decrypt[row + col * bytes_columns_size];
        }
      }

    AddRoundKey(data, expandkey[expand_key_size - 1]);

    for(intptr_t i = expand_key_size - 2; i >= 0; --i)
      {
      InvShiftRows(data);
      InvSubstituteBytes(data);
      AddRoundKey(data, expandkey[i]);
      if(i > 0)
        InvMixColumns(data);
      }

    for(size_t row = 0; row < bytes_row_size; ++row)
      {
      for(size_t col = 0; col < bytes_columns_size; ++col)
        {
        okdata[row + col * bytes_columns_size] = data[row][col];
        }
      }
    ret.append((const char*)okdata, sizeof(okdata));
    lp_decrypt += bytes_block_size;
    }

  return ret;
  }

//////////////////////////////////////////////////////////////////////////

#ifdef _XLIB_TEST_

ADD_XLIB_TEST(AES)
  {
  SHOW_TEST_INIT;

  bool done = false;

  const string key("012345678ABCDEF");
  const string data("1234567812345678");
  const string enc("\xAC\x69\xF1\xA1\xE1\x4A\x0C\x37\x37\x91\xEB\xDE\xAA\x7B\x08\xF4");

  SHOW_TEST_HEAD("AesEncrypt");
  done = (enc == AesEncrypt(data, key));
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("AesDecrypt");
  done = (data == AesDecrypt(enc, key));
  SHOW_TEST_RESULT(done);
  }

#endif  // _XLIB_TEST_