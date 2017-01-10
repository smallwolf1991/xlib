#include "xline.h"

#ifdef _XLIB_TEST_

#include "ws_s.h"

ADD_XLIB_TEST(XLINE)
  {
  SHOW_TEST_INIT;

  bool done = false;
  line tline;
  netline nline;
  line aa;
  netline bb;

  SHOW_TEST_HEAD("<< void*");
  tline.clear();
  nline.clear();
  tline << (void*)0x12345678;
  nline << (void*)0x12345678;
  done = (nline.size() == sizeof(void*)) &&
         (tline.size() == sizeof(void*)) &&
#if defined(_WIN64) || defined(__amd64)
         (*(void**)nline.c_str() == (void*)0x7856341200000000) &&
#else
         (*(void**)nline.c_str() == (void*)0x78563412) &&
#endif
         (*(void***)tline.c_str() == (void*)0x12345678);
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("<< T*");
  nline.clear();
  nline << "123456";
  done = (0 == memcmp(nline.c_str(), "123456", 6));
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("<< xline");
  tline.clear();
  nline.clear();
  aa << (int)0x12345678;
  bb << (int)0x12345678;
  tline << aa;
  nline << bb;
  done = (nline.size() == sizeof(int) + sizeof(unsigned short)) &&
    (tline.size() == sizeof(int) + sizeof(unsigned short)) &&
    (*(unsigned short*)nline.c_str() == 0x0400) &&
    (*(unsigned short*)tline.c_str() == 0x0004);
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("<< basic_string");
  tline.clear();
  nline.clear();
  tline << string("12345678");
  nline << string("12345678");
  done = (nline.size() == 8) && (tline.size() == 8);
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("<< T&");
  tline.clear();
  nline.clear();
  tline << (int)0x12345678;
  nline << (int)0x12345678;
  done = (nline.size() == sizeof(int)) &&
         (tline.size() == sizeof(int)) &&
         (*(int*)nline.c_str() == 0x78563412) &&
         (*(int*)tline.c_str() == 0x12345678);
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("mkhead");
  tline.clear();
  nline.clear();
  tline << (int)0x12345678;
  nline << (int)0x12345678;
  tline.mkhead();
  nline.mkhead();
  done = (nline.size() == sizeof(int) + sizeof(unsigned short)) &&
         (tline.size() == sizeof(int) + sizeof(unsigned short)) &&
         (*(unsigned short*)nline.c_str() == 0x0400) &&
         (*(unsigned short*)tline.c_str() == 0x0004);
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD(">> void*");
  tline.clear();
  nline.clear();
  tline << (void*)0x12345678 << "12345678" << '\0' << (int)12345678 << aa << '\0';
  nline << (void*)0x12345678 << "12345678" << (int)12345678 << bb << '\0';
  void* va;
  void* vb;
  tline >> va;
  nline >> vb;
  done = (va == (void*)0x12345678) && (vb == (void*)0x12345678);
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD(">> T*");
  char abuf[0x10];
  char bbuf[0x10];
  tline >> (char*)abuf >> cnull;
  nline >> (char*)bbuf;
  done = (0 == memcmp(abuf, "12345678", 8)) && (0 == memcmp(bbuf, "12345678", 9));
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD(">> xline");
  aa.clear();
  bb.clear();
  aa << (int)0x12345678;
  bb << (int)0x12345678;
  tline.clear();
  nline.clear();
  tline << aa;
  nline << bb;
  aa.clear();
  bb.clear();
  tline >> aa;
  nline >> bb;
  done = (aa.size() == sizeof(int)) && (bb.size() == sizeof(int)) &&
    nline.empty() && tline.empty();
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD(">> basic_string&");
  string bs;
  nline.clear();
  nline << (int)0x12345678;
  nline >> bs;
  done == (bs.size() == sizeof(int)) && nline.empty();
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD(">> T&");
  tline.clear();
  nline.clear();
  tline << (int)0x12345678;
  nline << (int)0x12345678;
  int ai;
  int bi;
  tline >> ai;
  nline >> bi;
  done = (0x12345678 == ai) && (0x12345678 == bi);
  SHOW_TEST_RESULT(done);
  
  SHOW_TEST_HEAD(">> T const&");
  tline.clear();
  nline.clear();
  tline << cnull;
  nline << cnull;
  tline >> cnull;
  nline >> cnull;
  done = tline.empty() && nline.empty();
  SHOW_TEST_RESULT(done);
  }

#endif  // _XLIB_TEST_