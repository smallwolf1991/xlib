/*!
  \file  xlib_def.h
  \brief xlib_def.h定义了一些库常用宏。

  \version    1.0.1401.07
  \note       For All

  \author     triones
  \date       2014-01-07
*/
#pragma once

#ifndef _WIN64
typedef unsigned __int32    TUINT;
typedef __int32             TINT;
typedef unsigned long       TULONG;
typedef long                TLONG;
#else
typedef unsigned __int64    TUINT;
typedef __int64             TINT;
typedef unsigned long long  TULONG;
typedef long long           TLONG;
#endif

/*!
  \brief      从指定地址开始读取一个指定类型的值。
  \param in   mem   该名字指定了一个内存地址。
  \return     从mem指定的内存地址处取出的一个指定类型的值。

  \code
    T tmpL = mkT(0x400000);   //取值
    mkT(0x10000) = 1;         //赋值
  \endcode
*/
#define mkQ(mem)    (*(unsigned __int64*)(mem))
#define mkD(mem)    (*(unsigned __int32*)(mem))
#define mkW(mem)    (*(unsigned __int16*)(mem))
#define mkB(mem)    (*(unsigned __int8*)(mem))
#define mkP(mem)    (*(void**)(mem))
#define mkTL(mem)   (*(TULONG*)(mem))

//! 计算指定类型的数据长度
#define sizeQ       (sizeof(unsigned __int64))
#define sizeD       (sizeof(unsigned __int32))
#define sizeW       (sizeof(unsigned __int16))
#define sizeB       (sizeof(unsigned __int8))
#define sizeP       (sizeof(void*))
#define sizeTL      (sizeof(TULONG))