#include "xlog.h"

#if defined(_XLIB_TEST_) || !defined(_WIN32)

#include <iostream>
using std::clog;
using std::endl;

#endif  // _XLIB_TEST _WIN32

xlog::levels xlog::dynamic_lvl = xlog::lvl_on;

size_t xlog::dynamic_type = (size_t)(0 - 1);

xlog::~xlog()
  {
  out();
  }

void xlog::out()
  {
  if(empty())  return;
#if !defined(_XLIB_TEST_) && defined(_WIN32)
#   ifndef  FOR_RING0
  OutputDebugStringA(c_str());
#   else    // FOR_RING0
  DbgPrint("%s\n",c_str());
#   endif   // FOR_RING0
#else   // _XLIB_TEST _WIN32
  clog << c_str() << endl;
#endif  // _XLIB_TEST _WIN32
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
  dynamic_type |= ((size_t)(1) << be_set_type);
  return tmp_type;
  }

size_t xlog::close_type(size_t be_set_type)
  {
  size_t tmp_type = dynamic_type;
  dynamic_type &= ~((size_t)(1) << be_set_type);
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

#ifdef _XLIB_TEST_

ADD_XLIB_TEST(XLOG)
  {
  SHOW_TEST_INIT;

  xerr << "xlog output";
  }

#endif  // _XLIB_TEST_