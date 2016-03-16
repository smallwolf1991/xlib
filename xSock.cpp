#include "xSock.h"

#ifndef FOR_RING0

xUDP& operator<<(xUDP& s,xUDP_func pfn)
  {
  return pfn(s);
  }

xTCP& operator<<(xTCP& s,xTCP_func pfn)
  {
  return pfn(s);
  }

xUDP& xsend(xUDP& s)
  {
  return s.send();
  }

xTCP& xsend(xTCP& s)
  {
  return s.send();
  }

xUDP& xxsend(xUDP& s)
  {
  return s.sendmkhead();
  }

xTCP& xxsend(xTCP& s)
  {
  return s.sendmkhead();
  }
#endif    //#ifndef FOR_RING0