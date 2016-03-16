/*!
  \file  xNetDl.h
  \brief xNetDl.h定义了下载指定url数据的函数

  \version    1.2.1501.2315
  \note       Only For Ring3

  \author     triones
  \date       2013-10-19
*/

#pragma once

#include "xlib_base.h"

#ifndef FOR_RING0

#include <string>

/*!
  指定url，返回数据\n
  指定except为true时，失败则抛出runtime_error异常。\n
  否则返回空数据，失败信息输出到debugview
*/
std::string DownloadUrl(LPCTSTR url, const bool except = true);

#endif  //#ifndef FOR_RING0