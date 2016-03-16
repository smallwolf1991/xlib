/*!
  \file  xrand.h
  \brief xrand.h定义了随机数的生成函数(随机平均率无法保证)

  \version    2.0.1311.2917
  \note       For All

  \author     triones
  \date       2013-01-08
*/

#pragma once

#include "xlib_base.h"

/*!
  用于生成随机数
  \param in mod   指定取模，默认为0，不取模
  \return         返回随机数

  \code
    TULONG randvalue = xrand();
  \endcode
*/
TULONG xrand(const TULONG mod = 0);