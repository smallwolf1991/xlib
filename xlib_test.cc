#include "xlib_test.h"

#ifdef _XLIB_TEST_

#include <map>

using std::map;

//! 做成函数即保证初始化，也保证内部访问
static map<string, xlib_test_routine>& routines()
  {
  static map<string, xlib_test_routine> xlib_test_routines;
  return xlib_test_routines;
  }

bool Add_XLIB_TEST_ROUTINE(const char* const name, xlib_test_routine func)
  {
  routines()[name] = func;
  return name != nullptr;
  }

#include "xlib.h"

#ifdef _WIN32
#include <tchar.h>
int _tmain(int , _TCHAR* )
#else   // _WIN32
int main()
#endif  // _WIN32
  {
  try
    {
    for(const auto& r : routines())
      {
      r.second();
      }
    cout << endl << "done." << endl;
    }
  catch(...)
    {
    cout << endl << endl << "exception !!!" << endl;
    }
  return 0;
  }

#endif  // _XLIB_TEST_