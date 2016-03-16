#include "ws_utf8.h"

using namespace std;

static const size_t gk_utf8_max_byte = 6;   //utf8最大占用字节

#pragma warning(push)
#pragma warning(disable:4244) //warning C4244: “=”: 从“const unsigned long”转换到“unsigned char”，可能丢失数据
size_t unicode_byte2utf8_byte(p_utf8               utf8,
                              const unsigned long  unicode)
  {
  unsigned char tu[gk_utf8_max_byte];
  if(utf8 == nullptr) utf8 = (unsigned char*)&tu;
  //0000 0000 0000 0000 0000 0000 0111 1111     7bit
  //0000 0000 0000 0000 0000 0000 0XXX XXXX     7bit
  if(unicode < 0x00000080)
    {
    utf8[0] = ((unicode & 0x0000007F) >> 0) | 0x00;
    return 1;
    }
  //0000 0000 0000 0000 0000 0000 10XX XXXX     6bit
  const unsigned long a = ((unicode & 0x0000003F) >> 0) | 0x80;
  //0000 0000 0000 0000 0000 0111 1111 1111     11bit
  //0000 0000 0000 0000 0011 0XXX XX00 0000     5bit
  if(unicode < 0x00000800)
    {
    utf8[1] = a;
    utf8[0] = ((unicode & 0x000007C0) >> 6) | 0xC0;
    return 2;
    }
  //0000 0000 0000 0000 0010 XXXX XX00 0000     6bit
  const unsigned long b = ((unicode & 0x00000FC0) >> 6) | 0x80;
  //0000 0000 0000 0000 1111 1111 1111 1111     16bit
  //0000 0000 0000 1110 XXXX 0000 0000 0000     4bit
  if(unicode < 0x00010000)
    {
    utf8[2] = a;  utf8[1] = b;
    utf8[0] = ((unicode & 0x0000F000) >> 12) | 0xE0;
    return 3;
    }
  //0000 0000 0000 10XX XXXX 0000 0000 0000     6bit
  const unsigned long c = ((unicode & 0x0003F000) >> 12) | 0x80;
  //0000 0000 0001 1111 1111 1111 1111 1111     21bit
  //0000 0011 110X XX00 0000 0000 0000 0000     3bit
  if(unicode < 0x00200000)
    {
    utf8[3] = a;  utf8[2] = b;  utf8[1] = c;
    utf8[0] = ((unicode & 0x001C0000) >> 18) | 0xF0;
    return 4;
    }
  //0000 0010 XXXX XX00 0000 0000 0000 0000     6bit
  const unsigned long d = ((unicode & 0x00FC0000) >> 18) | 0x80;
  //0000 0011 1111 1111 1111 1111 1111 1111     26bit
  //1111 10XX 0000 0000 0000 0000 0000 0000     2bit
  if(unicode < 0x04000000)
    {
    utf8[4] = a;  utf8[3] = b;  utf8[2] = c;  utf8[1] = d;
    utf8[0] = ((unicode & 0x03000000) >> 24) | 0xF8;
    return 5;
    }
  //00XX XXXX 0000 0000 0000 0000 0000 0000     6bit
  const unsigned long e = ((unicode & 0x3F000000) >> 24) | 0x80;
  //0111 1111 1111 1111 1111 1111 1111 1111     31bit
  //0X00 0000 0000 0000 0000 0000 0000 0000     1bit
  if(unicode < 0x80000000)
    {
    utf8[5] = a;  utf8[4] = b;  utf8[3] = c;  utf8[2] = d;  utf8[1] = e;
    utf8[0] = ((unicode & 0x04000000) >> 30) | 0xFC;
    return 6;
    }
  return 0;
  }
#pragma warning(pop)

size_t utf8_byte2unicode_byte(unsigned long*   unicode,
                              const p_utf8     utf8)
  {
  if(utf8 == nullptr)  return 0;
  unsigned long tu;
  if(unicode == nullptr) unicode = &tu;

  const unsigned char utf8_flag[gk_utf8_max_byte] = {0x7F,0xC0,0xE0,0xF0,0xF8,0xFC};

  if(utf8[0] <= utf8_flag[0])
    {
    *unicode = utf8[0];
    return 1;
    }

  if(utf8[0] < utf8_flag[1])   //首字节非法
    {
    return 0;
    }

  size_t lp = 0;

  for(size_t i = 2; i < sizeof(utf8_flag); ++i)
    {
    //判定首字节处于哪个区域
    if((utf8[lp] < utf8_flag[i]))
      {
      unsigned long u = utf8[lp] ^ utf8_flag[i - 1];
      ++lp;
      for(size_t j = 1; j < i; ++j)
        {
        if(utf8[lp] >= utf8_flag[1])  return 0; //后继字节非法，跳过
        u <<= 6;
        u |= (utf8[lp] & 0x3F);
        ++lp;
        }
      *unicode = u;
      return lp;
      }
    }
  return 0;
  }

size_t ws2utf8(p_utf8           utf8,
               const size_t     max_utf8,
               const wchar_t*   ws,
               const size_t     ws_len)
  {
  if(utf8 == nullptr || ws == nullptr || (intptr_t)max_utf8 < 1)
    return 0;

  size_t wlen = ws_len;
  if((intptr_t)ws_len < 0)
    {
    for(wlen = 0; ws[wlen] != L'\0'; ++wlen);
    }

  size_t lp = 0;
  unsigned char tu[gk_utf8_max_byte];

  //转换写入缓冲而不是直接写入，考虑有溢出情况
  for(size_t i = 0; i < wlen ; ++i)
    {
    const size_t k = unicode_byte2utf8_byte(tu, ws[i]);
    if(k == 0)  return 0;
    if(lp + k >= max_utf8)  return 0;
    memcpy(&utf8[lp], tu, k);
    lp += k;
    }

  //自行追加结束符，以应对ws指定长度且没有null结尾的情况
  const size_t k = unicode_byte2utf8_byte(tu, L'\0');
  if(lp + k >= max_utf8)  return 0;
  memcpy(&utf8[lp], tu, k);
  lp += k;
  return lp;
  }

xutf8 ws2utf8(const wstring& ws)
  {
  xutf8 utf8;
  unsigned char tu[gk_utf8_max_byte];

  for(auto ch : ws)
    {
    const size_t k = unicode_byte2utf8_byte(tu, ch);
    if(k == 0)
      {
      utf8.clear();
      break;
      }
    utf8.append(tu, k);
    }

  return utf8;
  }

size_t utf82ws(wchar_t*       ws,
               const size_t   max_ws,
               const p_utf8   utf8,
               const size_t   utf8_len)
  {
  if(ws == nullptr || utf8 == nullptr || (intptr_t)max_ws < 1)
    return 0;

  size_t ulen = utf8_len;
  if((intptr_t)utf8_len < 0)
    {
    for(ulen = 0; utf8[ulen] != '\0'; ++ulen);
    }

  size_t lp = 0;
  unsigned long ch;
  for(size_t i = 0; i < ulen ;)
    {
    const size_t k = utf8_byte2unicode_byte(&ch,&utf8[i]);
    if(k == 0)  return 0;

    i += k;
    if(i > ulen)  break;

    ws[lp] = (wchar_t)ch;
    if(lp >= max_ws)  return 0;
    ++lp;
    }

  if(lp >= max_ws)  return 0;
  ws[lp] = L'\0';
  ++lp;
  return lp;
  }

wstring utf82ws(const xutf8& utf8)
  {
  wstring ws;

  const p_utf8 pu = (const p_utf8)utf8.c_str();
  unsigned long ch;

  for(size_t i = 0; i < utf8.size();)
    {
    const size_t k = utf8_byte2unicode_byte(&ch,&pu[i]);
    if(k == 0)
      {
      ws.clear();
      return ws;
      }

    i += k;
    if(i > utf8.size())  break;

    ws.push_back((wchar_t)ch);
    }

  return ws;
  }