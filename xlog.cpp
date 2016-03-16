#include "xlog.h"

xlog::levels xlog::dynamic_lvl = xlog::lvl_on;

size_t xlog::dynamic_type = (size_t)(0-1);

xlog::~xlog()
  {
  out();
  }

void xlog::out()
  {
  if(empty())  return;
#ifndef  FOR_RING0
  OutputDebugStringA(c_str());
#else
  DbgPrint("%s\n",c_str());
#endif
  clear();
  }

xlog::levels xlog::set_level(xlog::levels new_level)
  {
  levels lvl = dynamic_lvl;
  dynamic_lvl = new_level;
  return lvl;
  }

size_t xlog::open_type(size_t be_set_type)
  {
  size_t tmp_type = dynamic_type;
  dynamic_type |= ((size_t)(1)<<be_set_type);
  return tmp_type;
  }

size_t xlog::close_type(size_t be_set_type)
  {
  size_t tmp_type = dynamic_type;
  dynamic_type &= ~((size_t)(1)<<be_set_type);
  return tmp_type;
  }

xlog::levels xlog::level()
  {
  return dynamic_lvl;
  }

size_t xlog::type()
  {
  return dynamic_type;
  }

xmsg& xlogout(xmsg& v)
  {
  (*(xlog*)&v).out();
  return v;
  }