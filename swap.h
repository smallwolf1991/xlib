﻿/*!
  \file  swap.h
  \brief swap.h定义了swap的相关模版。

  - bswap在开启完全优化情况下，会被编译成只有一条bswap的汇编指令

  \version    2.0.1611.1410
  \note       For All

  \author     triones
  \date       2014-01-07
*/
#ifndef _XLIB_SWAP_H_
#define _XLIB_SWAP_H_

#include "xlib_base.h"

#ifdef _WIN32
#define bswap_16 _byteswap_ushort
#define bswap_32 _byteswap_ulong
#define bswap_64 _byteswap_uint64
#else
#include <byteswap.h>
#endif

//! bswap模板，是bswap函数的实现细节
template<size_t N> inline void bswap_type(unsigned char* mem)
  {
  size_t len = N;
  while(len != 0)
    {
    switch(len)
      {
      case 1:bswap_type<1>(mem); return;
      case 2:
      case 3:bswap_type<2>(mem); return;
      case 4:
      case 5:bswap_type<4>(mem); return;
      case 6:
      case 7:bswap_type<4>(mem); bswap_type<2>(mem + 4); return;
      default:
        bswap_type<8>(mem);
        len -= 8;
        mem += 8;
      }
    }
  }
template<> inline void bswap_type<1>(unsigned char*)
  {
  }
template<> inline void bswap_type<2>(unsigned char* mem)
  {
  uint16* lp = (uint16*)mem;
  *lp = bswap_16(*lp);
  }
template<> inline void bswap_type<4>(unsigned char* mem)
  {
  uint32* lp = (uint32*)mem;
  *lp = bswap_32(*lp);
  }
template<> inline void bswap_type<8>(unsigned char* mem)
  {
  uint64* lp = (uint64*)mem;
  *lp = bswap_64(*lp);
  }

//! 用于翻转数据。
/*!
  \param    values  任意类型数据。
  \return           翻转后的原类型数据。

  \code
    bswap(0x12345678) == 0x78563412;
    bswap((short)0x1234) == 0x3412;
  \endcode
*/
template<typename T> inline T bswap(T const& values)
  {
  T v = values;
  bswap_type<sizeof(T)>((unsigned char*)&v);
  return v;
  }

//! 当A > B时，对调两值，返回真，否则不变，返回假
/*!
  \param    a   任意类型非常量值
  \param    b   任意类型非常量值
  \return       返回是否对调两值

  \code
    void* a = 0x5;
    void* b = 0x1;
    seqswap(a, b); //返回true，并且a == 1，b == 5
  \endcode
*/
template<typename T> inline bool seqswap(T& a, T& b)
  {
  if(a > b)
    {
    T t = a;
    a = b;
    b = t;
    return true;
    }
  return false;
  }
template<> inline bool seqswap(void*& a, void*& b)
  {
  return seqswap((unsigned char*&)a, (unsigned char*&)b);
  }

#endif  // _XLIB_SWAP_H_
