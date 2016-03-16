#include "xmutex.h"


#ifdef FOR_RING0

xmutex::xmutex()
  {
  KeInitializeMutex(&mutex,0);
  }

xmutex::~xmutex()
  {
  ;
  }

bool xmutex::wait()
  {
  const NTSTATUS ret =
    KeWaitForSingleObject(&mutex,Executive,KernelMode,FALSE,nullptr);
  KeReleaseMutex(&mutex,FALSE);
  return STATUS_SUCCESS == ret;
  }

#else           //#ifdef FOR_RING0

xmutex::xmutex(BOOL bInitialOwner, LPCTSTR lpName)
:handle(CreateMutex(nullptr,bInitialOwner,lpName))
  {
  ;
  }

xmutex::~xmutex()
  {
  CloseHandle(handle);
  }

bool xmutex::wait(const DWORD time)
  {
  const DWORD ret = WaitForSingleObject(handle,time);
  ReleaseMutex(handle);
  return WAIT_OBJECT_0 == ret;
  }

#endif          //#ifdef FOR_RING0