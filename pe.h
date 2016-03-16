/*!
  \file  pe.h
  \brief pe.h定义了解析PE文件的类

  \version    1.1.1405.0811
  \note       For All

  \author     triones
  \date       2012-07-20
*/

#pragma once

#include "xblk.h"

class pe
  {
  public:
    pe(const HMODULE hMod = nullptr);
    pe(LPCTSTR name);
    const IMAGE_DOS_HEADER* GetDosHead() const;
    const IMAGE_NT_HEADERS* GetPeHead() const;
    void*   EntryPoint()  const;
    xblk    GetImage()    const;
    xblk    GetCode()     const;
    HMODULE Module()      const;
    bool    IsPE()        const;
  private:
    HMODULE _hMod;
  };