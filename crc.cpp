#include "crc.h"

#include <stdlib.h>

static const unsigned __int16* GetCrc16Table()
  {
  static unsigned __int16 Crc16Table[0x100];
  for(unsigned __int16 i = 0; i < _countof(Crc16Table); ++i)
    {
    unsigned __int16 crc = i;
    for(size_t j = 0; j < 8; ++j)
      {
      crc = (crc >> 1) ^ ((crc & 1) ? 0xA001 : 0);
      }
    Crc16Table[i] = crc;
    }
  return Crc16Table;
  }

unsigned __int16 crc16(const void* buf, const size_t size)
  {
  if(buf == nullptr) return 0;
  static const unsigned __int16* const ct = GetCrc16Table();
  unsigned __int16 ret = 0;
  const unsigned char* _buf = (const unsigned char*)buf;
  for(size_t i = 0; i < size; ++i)
    {
    ret = ct[(ret & 0xFF) ^ _buf[i]] ^ (ret >> 8);
    }
  return ret;
  }

static const unsigned __int32* GetCrc32Table()
  {
  static unsigned __int32 Crc32Table[0x100];
  for(unsigned __int32 i = 0; i < _countof(Crc32Table); ++i)
    {
    unsigned __int32 crc = i;
    for(size_t j = 0; j < 8; ++j)
      {
      crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320 : 0);
      }
    Crc32Table[i] = crc;
    }
  return Crc32Table;
  }

unsigned __int32 crc32(const void* buf, const size_t size)
  {
  if(buf == nullptr) return 0;
  static const unsigned __int32* const ct = GetCrc32Table();
  unsigned __int32 ret = 0xFFFFFFFF;
  const unsigned char* _buf = (const unsigned char*)buf;
  for(size_t i = 0; i < size; ++i)
    {
    ret = ct[ (ret & 0xFF) ^ _buf[i] ] ^ (ret >> 8);
    }
  return ~ret;
  }

static const unsigned __int64* GetCrc64Table()
  {
  static unsigned __int64 Crc64Table[0x100];
  for(unsigned __int64 i = 0; i < _countof(Crc64Table); ++i)
    {
    unsigned __int64 crc = i;
    for(size_t j = 0; j < 8; ++j)
      {
      crc = (crc >> 1) ^ ((crc & 1) ? 0xC96C5795D7870F42 : 0);
      }
    Crc64Table[i] = crc;
    }
  return Crc64Table;
  }

unsigned __int64 crc64(const void* buf,const size_t size)
  {
  if(buf == nullptr) return 0;
  static const unsigned __int64* const ct = GetCrc64Table();
  unsigned __int64 ret = 0xFFFFFFFFFFFFFFFF;
  const unsigned char* _buf = (const unsigned char*)buf;
  for(size_t i = 0; i < size; ++i)
    {
    ret = ct[ (ret & 0xFF) ^ _buf[i] ] ^ (ret >> 8);
    }
  return ~ret;
  }