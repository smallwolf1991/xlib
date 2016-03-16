#include "xblk.h"

#include "swap.h"

xblk::xblk()
:_start(nullptr),_end(nullptr),_size(0)
  {

  }

xblk::xblk(void* const starts,void* const ends)
:_start(starts),_end(ends)
  {
  seqswap(_start,_end);
  _size = (size_t)_end - (size_t)_start;
  }

xblk::xblk(void* const starts,const intptr_t sizes)
:_start(starts),_size(sizes)
  {
  _end = (void*)((intptr_t)starts + sizes);
  if(seqswap(_start,_end))
    {
    _size = -(sizes);
    }
  }

void* xblk::start() const
  {
  return _start;
  }

void* xblk::end() const
  {
  return _end;
  }

size_t xblk::size() const
  {
  return _size;
  }

xblk::PosDcrpt xblk::checkin(void* const starts,void* const ends) const
  {
  const unsigned char* const s = (unsigned char*)start();
  const unsigned char* const e = (unsigned char*)end();
  const unsigned char* ss = (unsigned char*)starts;
  const unsigned char* ee = (unsigned char*)ends;
  seqswap(ss,ee);
  if(ss < s)
    {
    if(ee < s)  return PD_NoIn;
    if(ee > e)  return PD_SubIn;
    return PD_TailIn;
    }
  if(ss > e)  return PD_NoIn;
  if(ee > e)  return PD_HeadIn;
  return PD_WholeIn;
  }

xblk::PosDcrpt xblk::checkin(void* const starts,const intptr_t sizes) const
  {
  return checkin(starts,(void*)((intptr_t)starts + sizes));
  }

xblk::PosDcrpt xblk::checkin(const xblk &b) const
  {
  return checkin(b.start(),b.end());
  }

bool xblk::operator==(const xblk& b) const
  {
  if(start() != b.start()) return false;
  if(end() != b.end())  return false;
  return true;
  }

bool xblk::operator!=(const xblk& b) const
  {
  return !this->operator==(b);
  }