#include "xrand.h"

#include <stdlib.h>

#pragma warning(push)
#pragma warning(disable:4244) //warning C4244: “初始化”: 从“ULONG64”转换到“const unsigned int”，可能丢失数据

TULONG xrand(const TULONG mod)
  {
  //! 由于重要性不是很高，随机数的生成可以忽视多线程读写问题
  static TULONG gk_xrand_seed = 0;
  const TULONG r = __rdtsc();
#if _WIN64
  const int l = r % 64;
  gk_xrand_seed += _rotl64(r,l);
#else
  const int l = r % 32;
  gk_xrand_seed += _lrotl(r,l);
#endif
  return (mod) ? (gk_xrand_seed % mod) : (gk_xrand_seed);
  }

#pragma warning(pop)