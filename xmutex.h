/*!
  \file  xmutex.h
  \brief xmutex.h定义了通用的互斥体

  \version    1.0.1303.0110
  \note       For All

  \author     triones
  \date       2013-03-01
*/

#pragma once

#include "xlib_base.h"

#ifdef FOR_RING0

//! Ring0下的mutex，暂未查到如何释放mutex资源
class xmutex
  {
  public:
    xmutex();
    ~xmutex();
  public:
    bool wait();
  public:
    xmutex(const xmutex&) = delete;
    xmutex& operator=(const xmutex&) = delete;
  private:
    KMUTEX mutex;
  };

#else

//! Ring3下的mutex,与Ring0有一定区别
class xmutex
  {
  public:
    xmutex(BOOL bInitialOwner = FALSE, LPCTSTR lpName = nullptr);
    ~xmutex();
  public:
    bool wait(const DWORD time = INFINITE);
  public:
    xmutex(const xmutex&) = delete;
    xmutex& operator=(const xmutex&) = delete;
  private:
    const HANDLE handle;
  };

#endif