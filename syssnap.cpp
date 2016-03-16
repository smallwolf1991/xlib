#include "syssnap.h"
#include "xlib_nt.h"

SysSnap::SysSnap(SYSTEM_INFORMATION_CLASS SystemInformationClass)
  {
  NTSTATUS dwRet;
  ULONG_PTR len = 0;
  while((dwRet = ZwQuerySystemInformation(SystemInformationClass,
    (PVOID)c_str(), (ULONG_PTR)capacity(), &len)) == STATUS_INFO_LENGTH_MISMATCH)
    {
    reserve(capacity() + len + 0x10);
    }
  if(dwRet == STATUS_SUCCESS)
    {
    append(c_str(), len);
    }
  else
    {
    clear();
    }
  }



SysProcessSnap::const_iterator::const_iterator(const SYSTEM_PROCESS_INFORMATION* p)
: _p(p)
  {
  ;
  }

bool SysProcessSnap::const_iterator::operator==(const SysProcessSnap::const_iterator& it) const
  {
  return (_p == it._p);
  }

bool SysProcessSnap::const_iterator::operator!=(const SysProcessSnap::const_iterator& it) const
  {
  return (_p != it._p);
  }

SysProcessSnap::const_iterator& SysProcessSnap::const_iterator::operator++()
  {
  if(_p != nullptr)
    {
    if((_p->NextEntryOffset) == 0)
      {
      _p = nullptr;
      }
    else
      {
      _p = (const SYSTEM_PROCESS_INFORMATION*)((const unsigned char*)_p + _p->NextEntryOffset);
      }
    }
  return *this;
  }

const SYSTEM_PROCESS_INFORMATION* SysProcessSnap::const_iterator::operator->() const
  {
  return _p;
  }

const SYSTEM_PROCESS_INFORMATION& SysProcessSnap::const_iterator::operator*() const
  {
  return *_p;
  }

SysProcessSnap::SysProcessSnap() :SysSnap(SystemProcessInformation)
  {
  ;
  }

SysProcessSnap::const_iterator SysProcessSnap::begin() const
  {
  if(empty())  return nullptr;
  return (const SYSTEM_PROCESS_INFORMATION*)(c_str());
  }

SysProcessSnap::const_iterator SysProcessSnap::end() const
  {
  return nullptr;
  }



SysThreadSnap::SysThreadSnap(HANDLE PID)
:_p(nullptr)
  {
  for(const SYSTEM_PROCESS_INFORMATION& p : (*(SysProcessSnap*)this))
    {
    if(p.ProcessId == PID)
      {
      _p = &p;
      }
    }
  }

const SYSTEM_THREAD* SysThreadSnap::begin() const
  {
  if(_p == nullptr) return nullptr;
  return _p->Threads;
  }

const SYSTEM_THREAD* SysThreadSnap::end() const
  {
  if(_p == nullptr) return nullptr;
  return &(_p->Threads[_p->NumberOfThreads]);
  }



SysDriverSnap::SysDriverSnap()
:SysSnap(SystemModuleInformation)
  {
  ;
  }

const SYSTEM_MODULE* SysDriverSnap::begin() const
  {
  if(empty())  return nullptr;
  const PSYSTEM_MODULE_INFORMATION p = (PSYSTEM_MODULE_INFORMATION)c_str();
  if(p->NumberOfModules <= 0) return nullptr;
  return p->Information;
  }

const SYSTEM_MODULE* SysDriverSnap::end() const
  {
  if(empty())  return nullptr;
  const PSYSTEM_MODULE_INFORMATION p = (PSYSTEM_MODULE_INFORMATION)c_str();
  if(p->NumberOfModules <= 0) return nullptr;
  return &(p->Information[p->NumberOfModules]);
  }