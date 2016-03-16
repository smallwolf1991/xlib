#include "aes.h"

AesKey::AesKey(const void* key,size_t len)
  {
  memset(_key,0,sizeof(_key));
  if(len == 0)
    {
    const char* k = (const char*)key;
    while(k[len] != '\0') ++len;
    }
  if(len > (size_t)sizeof(_key))  len = sizeof(_key);
  memcpy(_key,key,len);
  }

//! AES置换表
static const unsigned char sBox[] =
  {
  //0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f
  0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76, //0 
  0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0, //1
  0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15, //2 
  0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75, //3 
  0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84, //4 
  0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf, //5
  0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8, //6 
  0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2, //7 
  0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73, //8 
  0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb, //9 
  0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79, //a
  0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08, //b
  0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a, //c 
  0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e, //d
  0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf, //e 
  0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16  //f
  };

//! AES逆置换表
static const unsigned char invsBox[] = 
  {
  //0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f
  0x52,0x09,0x6a,0xd5,0x30,0x36,0xa5,0x38,0xbf,0x40,0xa3,0x9e,0x81,0xf3,0xd7,0xfb, //0 
  0x7c,0xe3,0x39,0x82,0x9b,0x2f,0xff,0x87,0x34,0x8e,0x43,0x44,0xc4,0xde,0xe9,0xcb, //1
  0x54,0x7b,0x94,0x32,0xa6,0xc2,0x23,0x3d,0xee,0x4c,0x95,0x0b,0x42,0xfa,0xc3,0x4e, //2 
  0x08,0x2e,0xa1,0x66,0x28,0xd9,0x24,0xb2,0x76,0x5b,0xa2,0x49,0x6d,0x8b,0xd1,0x25, //3 
  0x72,0xf8,0xf6,0x64,0x86,0x68,0x98,0x16,0xd4,0xa4,0x5c,0xcc,0x5d,0x65,0xb6,0x92, //4 
  0x6c,0x70,0x48,0x50,0xfd,0xed,0xb9,0xda,0x5e,0x15,0x46,0x57,0xa7,0x8d,0x9d,0x84, //5 
  0x90,0xd8,0xab,0x00,0x8c,0xbc,0xd3,0x0a,0xf7,0xe4,0x58,0x05,0xb8,0xb3,0x45,0x06, //6 
  0xd0,0x2c,0x1e,0x8f,0xca,0x3f,0x0f,0x02,0xc1,0xaf,0xbd,0x03,0x01,0x13,0x8a,0x6b, //7
  0x3a,0x91,0x11,0x41,0x4f,0x67,0xdc,0xea,0x97,0xf2,0xcf,0xce,0xf0,0xb4,0xe6,0x73, //8 
  0x96,0xac,0x74,0x22,0xe7,0xad,0x35,0x85,0xe2,0xf9,0x37,0xe8,0x1c,0x75,0xdf,0x6e, //9
  0x47,0xf1,0x1a,0x71,0x1d,0x29,0xc5,0x89,0x6f,0xb7,0x62,0x0e,0xaa,0x18,0xbe,0x1b, //a
  0xfc,0x56,0x3e,0x4b,0xc6,0xd2,0x79,0x20,0x9a,0xdb,0xc0,0xfe,0x78,0xcd,0x5a,0xf4, //b 
  0x1f,0xdd,0xa8,0x33,0x88,0x07,0xc7,0x31,0xb1,0x12,0x10,0x59,0x27,0x80,0xec,0x5f, //c 
  0x60,0x51,0x7f,0xa9,0x19,0xb5,0x4a,0x0d,0x2d,0xe5,0x7a,0x9f,0x93,0xc9,0x9c,0xef, //d 
  0xa0,0xe0,0x3b,0x4d,0xae,0x2a,0xf5,0xb0,0xc8,0xeb,0xbb,0x3c,0x83,0x53,0x99,0x61, //e 
  0x17,0x2b,0x04,0x7e,0xba,0x77,0xd6,0x26,0xe1,0x69,0x14,0x63,0x55,0x21,0x0c,0x7d  //f
  }; 

//! 每行字节数
static const size_t bytes_columns_size = 4;

//! 每列字节数
static const size_t bytes_row_size = 4;

//! 扩展密钥组数
static const size_t expand_key_size = 11;

//! 扩展密钥轮常量
static const unsigned char expand_key_round_const[expand_key_size] =
  {
  0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36
  };


//! 非线性字节替代
static void SubstituteBytes(unsigned char data[bytes_row_size][bytes_columns_size])
  {
  for(size_t row = 0; row < bytes_row_size; ++row)
    {
    for(size_t col = 0; col < bytes_columns_size; ++col)
      {
      unsigned char& v = data[row][col];
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
      unsigned char& v = data[row][col];
      v = invsBox[v];
      }
    }
  }

//! 行移位变换
static void ShiftRows(unsigned char data[bytes_row_size][bytes_columns_size])
  {
  unsigned char v[bytes_columns_size];
  //第一组不变换
  for(size_t row = 1; row < bytes_row_size; ++row)
    {
    for(size_t col = 0; col < bytes_columns_size; ++col)
      {
      v[col] = data[row][(col+row) % bytes_columns_size];
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
  //第一组不变换
  for(size_t row = 1; row < bytes_row_size; ++row)
    {
    for(size_t col = 0; col < bytes_columns_size; ++col)
      {
      v[col] = data[row][(col-row+bytes_columns_size) % bytes_columns_size];
      }
    for(size_t col = 0; col < bytes_columns_size; ++col)
      {
      data[row][col] = v[col];
      }
    }
  }

//! 矩阵变换算法
static unsigned char FFmul(const unsigned char a,const unsigned char b)
  {
  unsigned char v[bytes_columns_size];
  unsigned char ret = 0;
  //由b生成一组数据
  v[0] = b;
  for(size_t col = 1; col < bytes_columns_size; ++col)
    {
    v[col] = v[col-1] << 1;
    if(v[col-1] & 0x80)
      {
      v[col] ^= 0x1B;
      }
    }
  //由a计算出结果
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
        ^ FFmul(0x03, v[(row+1)%4])
        ^ FFmul(0x01, v[(row+2)%4])
        ^ FFmul(0x01, v[(row+3)%4]);
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
        ^ FFmul(0x0b, v[(row+1)%4])
        ^ FFmul(0x0d, v[(row+2)%4])
        ^ FFmul(0x09, v[(row+3)%4]);
      }
    }
  }

//! 轮密钥加变换
static void AddRoundKey(unsigned char data[bytes_row_size][bytes_columns_size],
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
                         unsigned char expandkey[expand_key_size][bytes_row_size][bytes_columns_size])
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
    for(size_t col=0; col < bytes_columns_size; ++col)
      {
      unsigned char v[bytes_columns_size];

      for(size_t row = 0; row < bytes_row_size; ++row)
        {
        v[row] = col ? expandkey[i][row][col-1] : expandkey[i-1][row][3];
        }

      if(col == 0)
        {
        const unsigned char t = v[0];
        for(size_t row = 0; row < bytes_row_size - 1; ++row)
          {
          v[row] = sBox[ v[(row + 1) % bytes_row_size] ];
          }
        v[3] = sBox[t];
        v[0] ^= expand_key_round_const[i];
        }

      for(size_t row = 0; row < bytes_row_size; ++row)
        {
        expandkey[i][row][col] = expandkey[i-1][row][col] ^ v[row];
        }
      }
    }
  }

line AesEncrypt(const void*     encrypt_data,
                const size_t    encrypt_data_size,
                const AesKey&   encrypt_key)
  {
  unsigned char data[bytes_row_size][bytes_columns_size];
  unsigned char expandkey[expand_key_size][bytes_row_size][bytes_columns_size];
  unsigned char okdata[bytes_row_size*bytes_columns_size];
  line ret;
  const unsigned char* lp_encrypt = (const unsigned char*)encrypt_data;

  KeyExpansion(encrypt_key._key,expandkey);

  for(size_t encrypted = 0;
    encrypted < encrypt_data_size;
    encrypted += (bytes_row_size*bytes_columns_size))
    {
    for(size_t row = 0; row < bytes_row_size; ++row)
      {
      for(size_t col = 0; col < bytes_columns_size; ++col)
        {
        data[row][col] = lp_encrypt[row + col * bytes_columns_size];
        }
      }

    AddRoundKey(data,expandkey[0]);

    for(size_t i = 1; i < expand_key_size; ++i)
      {
      SubstituteBytes(data);
      ShiftRows(data);
      if(i != (expand_key_size - 1))
        MixColumns(data);
      AddRoundKey(data,expandkey[i]);
      }

    for(size_t row = 0; row < bytes_row_size; ++row)
      {
      for(size_t col = 0; col < bytes_columns_size; ++col)
        {
        okdata[row + col * bytes_columns_size] = data[row][col];
        }
      }
    ret.append(okdata,sizeof(okdata));
    lp_encrypt += bytes_row_size * bytes_columns_size;
    }

  return ret;
  }

line AesDecrypt(const void*     decrypt_data,
                const size_t    decrypt_data_size,
                const AesKey&   decrypt_key)
  {
  unsigned char data[bytes_row_size][bytes_columns_size];
  unsigned char expandkey[expand_key_size][bytes_row_size][bytes_columns_size];
  unsigned char okdata[bytes_row_size*bytes_columns_size];
  line ret;
  const unsigned char* lp_decrypt = (const unsigned char*)decrypt_data;

  KeyExpansion(decrypt_key._key,expandkey);

  for(size_t encrypted = 0;
    encrypted < decrypt_data_size;
    encrypted += (bytes_row_size*bytes_columns_size))
    {
    for(size_t row = 0; row < bytes_row_size; ++row)
      {
      for(size_t col = 0; col < bytes_columns_size; ++col)
        {
        data[row][col] = lp_decrypt[row + col * bytes_columns_size];
        }
      }

    AddRoundKey(data,expandkey[expand_key_size-1]);

    for(intptr_t i = expand_key_size - 2; i >= 0; --i)
      {
      InvShiftRows(data);
      InvSubstituteBytes(data);
      AddRoundKey(data,expandkey[i]);
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
    ret.append(okdata,sizeof(okdata));
    lp_decrypt += bytes_row_size * bytes_columns_size;
    }

  return ret;
  }