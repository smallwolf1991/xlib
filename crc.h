/*!
  \file  crc.h
  \brief crc.h定义了crc16算法、crc32算法、crc64算法

  \version    3.0.1402.1811
  \note       For All

  \author     triones
  \date       2013-03-19
*/

#pragma once

#include "xlib_base.h"
#include <string>
/*!
  生成指定数据的crc16
  \param    buf     指定需要计算crc16的缓冲
  \param    size    指定需要计算crc16的缓冲大小
  \return           返回crc16值

  \code
    unsigned __int16 c = crc16("012345",6);
  \endcode
*/
unsigned __int16 crc16(const void* buf, const size_t size);

/*!
  生成指定数据的crc32
  \param    buf     指定需要计算crc32的缓冲
  \param    size    指定需要计算crc32的缓冲大小
  \return           返回crc32值

  \code
    unsigned __int32 c = crc32("012345",6);
  \endcode
*/
unsigned __int32 crc32(const void* buf,const size_t size);

/*!
  生成指定数据的crc64
  \param    buf     指定需要计算crc64的缓冲
  \param    size    指定需要计算crc64的缓冲大小
  \return           返回crc64值

  \code
    unsigned __int64 c = crc64("012345",6);
  \endcode
*/
unsigned __int64 crc64(const void* buf,const size_t size);

//! crc16模版
template<typename T> unsigned __int16 crc16(const std::basic_string<T>& s)
  {
  return crc16(s.c_str(), s.size() * sizeof(T));
  }
//! crc32模版
template<typename T> unsigned __int32 crc32(const std::basic_string<T>& s)
  {
  return crc32(s.c_str(), s.size() * sizeof(T));
  }
//! crc64模版
template<typename T> unsigned __int64 crc64(const std::basic_string<T>& s)
  {
  return crc64(s.c_str(), s.size() * sizeof(T));
  }