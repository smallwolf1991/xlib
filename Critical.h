/*!
  \file  Critical.h
  \brief Critical.h定义了一些系统对象封装

  \version    1.0.1105.2311
  \note       Only For Ring3

  \author     triones
  \date       2011-05-23
*/

#pragma once

#ifndef FOR_RING0

#include "xlib_base.h"

//! 用于设置临界区
class  SysCritical
  {
  public:
    SysCritical();
    ~SysCritical();
  public:
    void  enter();
    void  leave();
  public:
    SysCritical(const SysCritical&) = delete;
    SysCritical& operator=(const SysCritical&) = delete;
  private:
    CRITICAL_SECTION cs;
  };

//! 用于设置自动临界区
class xCritical
  {
  public:
    xCritical();
    ~xCritical();
  public:
    xCritical(const xCritical&) = delete;
    xCritical& operator=(const xCritical&) =delete;
  private:
    SysCritical sc;
  };

#endif  //#ifndef FOR_RING0