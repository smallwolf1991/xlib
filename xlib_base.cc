﻿#include "xlib_base.h"

#ifdef _WIN32

#ifdef FOR_RING0

//注意#undef POOL_TAGGING无效，并不能还原API定义
#ifdef POOL_TAGGING
#   undef ExAllocatePool
#   undef ExAllocatePoolWithQuota
#   undef ExFreePool
#endif

void* __cdecl operator new(size_t size)
  {
  return ExAllocatePool(NonPagedPool, size);
  }

void* __cdecl operator new(size_t size, POOL_TYPE pooltype)
  {
  return ExAllocatePool(pooltype, size);
  }

void* __cdecl operator new[](size_t size)
  {
  return ExAllocatePool(NonPagedPool, size);
  }

void* __cdecl operator new[](size_t size, POOL_TYPE pooltype)
  {
  return ExAllocatePool(pooltype, size);
  }

void __cdecl operator delete(void* mem)
  {
  if(mem == nullptr)  return;
  ExFreePool(mem);
  }

void __cdecl operator delete[](void* mem)
  {
  if(mem == nullptr)  return;
  ExFreePool(mem);
  }

#endif  // FOR_RING0

#endif  // _WIN32