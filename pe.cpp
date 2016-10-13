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
  XLIB_TRY
    {
    const IMAGE_DOS_HEADER* doshead = GetDosHead();
    if(doshead == nullptr) return nullptr;
    return (IMAGE_NT_HEADERS*)
      ((size_t)doshead + (size_t)doshead->e_lfanew);
    }
  XLIB_CATCH
    {
    return nullptr;
    }
  }

void* pe::EntryPoint() const
  {
  XLIB_TRY
    {
    const IMAGE_NT_HEADERS* pehead = GetPeHead();
    if(pehead == nullptr) return nullptr;
    return (void*)(pehead->OptionalHeader.AddressOfEntryPoint + 
      (size_t)GetDosHead());
    }
  XLIB_CATCH
    {
    return nullptr;
    }
  }

xblk pe::GetImage() const
  {
  XLIB_TRY
    {
    const IMAGE_NT_HEADERS* pehead = GetPeHead();
    if(pehead == nullptr) return xblk(nullptr, nullptr);
    return xblk(
      (void*)GetDosHead(),
      pehead->OptionalHeader.SizeOfImage);
    }
  XLIB_CATCH
    {
    return xblk(nullptr, nullptr);
    }
  }

xblk pe::GetCode() const
  {
  XLIB_TRY
    {
    const IMAGE_NT_HEADERS* pehead = GetPeHead();
    if(pehead == nullptr) return xblk(nullptr, nullptr);
    return xblk(
      (void*)(pehead->OptionalHeader.BaseOfCode + (size_t)GetDosHead()),
      pehead->OptionalHeader.SizeOfCode);
    }
  XLIB_CATCH
    {
    return xblk(nullptr, nullptr);
    }
  }

HMODULE pe::Module() const
  {
  return _hMod;
  }

bool pe::IsPE() const
  {
  XLIB_TRY
    {
    const IMAGE_DOS_HEADER* doshead = GetDosHead();
    if(doshead->e_magic != 'ZM')  return false;
    const IMAGE_NT_HEADERS* pehead = GetPeHead();
    return pehead->Signature == 'EP';
    }
  XLIB_CATCH
    {
    return false;
    }
  }