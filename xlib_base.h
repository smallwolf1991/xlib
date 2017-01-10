/*!
  \file   xlib_base.h
  \brief  xlib_base.h用于基础包含

  - 异常处理宏定义，及Ring0下基础包含，Ring3下基础包含
  - Ring0下全局的new delete操作符重载。
  - 由于operator new与operator delete的特殊性，必须把这组操作独立出来，
    并置放于多数头文件声明之前，以确保内核内存分配的正确性。
  - 有了这个头文件，内核编程中也可以使用类等动态对象
  - 默认使用非分页内存

  \version    3.0.1611.1415
  \note       For All

  \author     triones
  \date       2010-07-01
*/
#ifndef _XLIB_BASE_H_
#define _XLIB_BASE_H_

#include <stdlib.h>

#include "xlib_link.h"
#include "xlib_test.h"

#ifdef _WIN32

#ifdef FOR_RING0

#ifdef __cplusplus
extern "C"
  {
#endif
#pragma warning(push)
#pragma warning(disable:4201)//使用了非标准扩展 : 无名称的结构/联合
#include <ntifs.h>
#include <windef.h>
#pragma warning(pop)
#ifdef __cplusplus
  } 
#endif

//! 以下是new与delete的RING0级重载
void* __cdecl operator new(size_t size);
void* __cdecl operator new(size_t size, POOL_TYPE pooltype);
void* __cdecl operator new[](size_t size);
void* __cdecl operator new[](size_t size, POOL_TYPE pooltype);
void  __cdecl operator delete(void* mem);
void  __cdecl operator delete[](void* mem);

#include "xlib_struct_ring0.h"
#include "xlib_struct.h"

#define XLIB_TRY     __try
#define XLIB_CATCH   __except(EXCEPTION_EXECUTE_HANDLER)

#pragma warning(disable:4127) // warning C4127: 条件表达式是常量
#pragma warning(disable:4100) // warning C4100: 未引用的形参
#else   // FOR_RING0

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "xlib_struct_ring3.h"
#include "xlib_struct.h"

//! ring3下，建议使用XLIB_TRY代替try
#define XLIB_TRY     try
//! ring3下，建议使用XLIB_CATCH代替catch
#define XLIB_CATCH   catch(...)

#endif  // FOR_RING0

#else   // _WIN32

#include "xlib_def.h"
#define XLIB_TRY     try
#define XLIB_CATCH   catch(...)

#endif  // _WIN32

#endif  // _XLIB_BASE_H_