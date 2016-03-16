#include "pe.h"

#include "ws_s.h"
#include "syssnap.h"

using namespace std;

pe::pe(const HMODULE hMod)
:_hMod(hMod)
  {
  if(hMod != nullptr) return;
#ifndef FOR_RING0
  _hMod = GetModuleHandle(nullptr);
#else
  SysDriverSnap sds;
  for(const SYSTEM_MODULE& st : sds)
    {
    _hMod = (HMODULE)st.ImageBaseAddress;
    break;
    }
#endif
  }

pe::pe(LPCTSTR name)
:_hMod(nullptr)
  {
#ifndef FOR_RING0
  _hMod = GetModuleHandle(name);
#else
# ifdef  UNICODE
  const string s(ws2s(name));
  name = (LPCTSTR)s.c_str();
# endif
  SysDriverSnap sds;
  for(const SYSTEM_MODULE& st : sds)
    {
    if(0 == _stricmp((const char*)name, (const char*)(st.Name + st.NameOffset)))
      {
      _hMod = (HMODULE)st.ImageBaseAddress;
      break;
      }
    }
#endif
  }

const IMAGE_DOS_HEADER* pe::GetDosHead() const
  {
  return (IMAGE_DOS_HEADER*)_hMod;
  }

const IMAGE_NT_HEADERS* pe::GetPeHead() const
  {
  const IMAGE_DOS_HEADER* doshead = GetDosHead();
  return (IMAGE_NT_HEADERS*)
    ((size_t)doshead + (size_t)doshead->e_lfanew);
  }

void* pe::EntryPoint() const
  {
  const IMAGE_NT_HEADERS* pehead = GetPeHead();
  return (void*)(pehead->OptionalHeader.AddressOfEntryPoint + 
    (size_t)GetDosHead());
  }

xblk pe::GetImage() const
  {
  const IMAGE_NT_HEADERS* pehead = GetPeHead();
  return xblk(
    (void*)GetDosHead(),
    pehead->OptionalHeader.SizeOfImage);
  }

xblk pe::GetCode() const
  {
  const IMAGE_NT_HEADERS* pehead = GetPeHead();
  return xblk(
    (void*)(pehead->OptionalHeader.BaseOfCode + (size_t)GetDosHead()),
    pehead->OptionalHeader.SizeOfCode);
  }

HMODULE pe::Module() const
  {
  return _hMod;
  }

bool pe::IsPE() const
  {
  const IMAGE_DOS_HEADER* doshead = GetDosHead();
  if(doshead->e_magic != 'ZM')  return false;
  const IMAGE_NT_HEADERS* pehead = GetPeHead();
  return pehead->Signature == 'EP';
  }