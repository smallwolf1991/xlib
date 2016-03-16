#include "Critical.h"

#ifndef FOR_RING0

SysCritical::SysCritical()
  {
  InitializeCriticalSection(&cs);
  }

SysCritical::~SysCritical()
  {
  DeleteCriticalSection(&cs);
  }

void SysCritical::enter()
  {
  EnterCriticalSection(&cs);
  }

void SysCritical::leave()
  {
  LeaveCriticalSection(&cs);
  }


xCritical::xCritical()
  {
  sc.enter();
  }

xCritical::~xCritical()
  {
  sc.leave();
  }

#endif  //#ifndef FOR_RING0