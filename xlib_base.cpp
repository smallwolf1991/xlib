#include "xlib_base.h"

#ifdef FOR_RING0

//注意#undef POOL_TAGGING无效
#   ifdef POOL_TAGGING
#     undef ExAllocatePool
#     undef ExAllocatePoolWithQuota
#     undef ExFreePool
#   endif

void* __cdecl operator new(size_t size)
  {
  return ExAllocatePool(NonPagedPool,size);
  }

void* __cdecl operator new(size_t size,POOL_TYPE pooltype)
  {
  return ExAllocatePool(pooltype,size);
  }

void* __cdecl operator new[](size_t size)
  {
  return ExAllocatePool(NonPagedPool,size);
  }

void* __cdecl operator new[](size_t size,POOL_TYPE pooltype)
  {
  return ExAllocatePool(pooltype,size);
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

#endif    //#ifdef FOR_RING0