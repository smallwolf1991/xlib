#include "ws_s.h"
#include "xlib_nt.h"

using std::string;
using std::wstring;

size_t ws2s(char* s,const size_t max_s,const wchar_t* ws,const size_t ws_len)
  {
  if(s == nullptr || ws == nullptr || (intptr_t)max_s < 1)
    return 0;

  const size_t wlen = (((intptr_t)ws_len >= 0) ? ws_len : wcslen(ws)) * sizeof(wchar_t);

  size_t wused = 0;
  size_t sused = 0;

  while((wused < wlen) && (sused < max_s))
    {
    size_t wnow = wlen - wused;
    if(wnow > 0xFFFE) wnow = 0xFFFE;

    UNICODE_STRING us;
    us.Length = (USHORT)wnow;
    us.MaximumLength = (USHORT)wnow;
    us.Buffer = (PWCH)&ws[wused / sizeof(wchar_t)];

    size_t snow = max_s - sused;
    if(snow > 0xFFFF) snow = 0xFFFF;

    ANSI_STRING as =
      {
      (USHORT)0,
      (USHORT)snow,
      s
      };

    if(!NT_SUCCESS(RtlUnicodeStringToAnsiString(&as, &us, false)))
      {
      break;
      }

    sused += as.Length;
    wused += wnow;
    }
  return sused;
  }

string ws2s(const std::wstring& ws)
  {
  string s;
  if(ws.empty())  return s;

  const size_t wlen = ws.size() * sizeof(wchar_t);

  size_t wused = 0;

  while(wused < wlen)
    {
    size_t wnow = wlen - wused;
    if(wnow > 0xFFFE) wnow = 0xFFFE;

    UNICODE_STRING us;
    us.Length = (USHORT)wnow;
    us.MaximumLength = (USHORT)wnow;
    us.Buffer = (PWCH)(ws.c_str() + wused / sizeof(wchar_t));

    ANSI_STRING as;
    as.Length = 0;
    as.MaximumLength = 0;
    as.Buffer = nullptr;
    bool rets = NT_SUCCESS(RtlUnicodeStringToAnsiString(&as, &us, true));
    if(rets && as.Length != 0)
      {
      s.append(as.Buffer, as.Length);
      }
    RtlFreeAnsiString(&as);
    if(!rets)
      {
      break;
      }
    wused += wnow;
    }
  return s;
  }

size_t s2ws(wchar_t* ws,const size_t max_ws,const char* s,const size_t s_len)
  {
  if(ws == nullptr || s == nullptr || (intptr_t)max_ws < 1)
    return false;

  const size_t slen = ((intptr_t)s_len >= 0) ? s_len : strlen(s);

  size_t sused = 0;
  size_t wused = 0;

  while((sused < slen) && (wused < max_ws))
    {
    size_t snow = slen - sused;
    if(snow > 0x7FFE) snow = 0x7FFE;

    ANSI_STRING as;
    as.Length = (USHORT)snow;
    as.MaximumLength = (USHORT)snow;
    as.Buffer = (PCH)&s[sused];

    size_t wnow = (max_ws - wused) * sizeof(wchar_t);

    if(wnow > 0xFFFE) wnow = 0xFFFE;

    UNICODE_STRING us =
      {
      (USHORT)0,
      (USHORT)wnow,
      ws
      };

    if(!NT_SUCCESS(RtlAnsiStringToUnicodeString(&us, &as, false)))
      {
      break;
      }

    wused += us.Length / sizeof(wchar_t);
    sused += snow;
    }
  return wused;
  }

wstring s2ws(const std::string& s)
  {
  wstring  ws;
  if(s.empty()) return ws;

  const size_t slen = s.size();

  size_t sused = 0;

  while(sused < slen)
    {
    size_t snow = slen - sused;
    if(snow > 0x7FFE) snow = 0x7FFE;

    ANSI_STRING as;
    as.Length = (USHORT)snow;
    as.MaximumLength = (USHORT)snow;
    as.Buffer = (PCH)(s.c_str() + sused);

    UNICODE_STRING us;
    us.Length = 0;
    us.MaximumLength = 0;
    us.Buffer = nullptr;
    const bool rets = NT_SUCCESS(RtlAnsiStringToUnicodeString(&us, &as, true));
    if(rets && us.Length != 0)
      {
      ws.append(us.Buffer, us.Length / sizeof(wchar_t));
      }
    RtlFreeUnicodeString(&us);
    if(!rets)
      {
      break;
      }
    sused += snow;
    }
  return ws;
  }