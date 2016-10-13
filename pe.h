/*!
  \file  pe.h
  \brief pe.h定义了解析PE文件的类

  \version    1.4.1610.1314
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
    //! 返回DosHead，即hMod
    const IMAGE_DOS_HEADER* GetDosHead() const;
    //! 当mod非PE时，返回nullptr
    const IMAGE_NT_HEADERS* GetPeHead() const;
    //! 当mod非PE时，返回nullptr
    void*   EntryPoint()  const;
    //! 当mod非PE时，返回空范围
    xblk    GetImage()    const;
    //! 当mod非PE时，返回空范围
    xblk    GetCode()     const;
    HMODULE Module()      const;
    bool    IsPE()        const;
  private:
    HMODULE _hMod;
  };