#include "crc.h"

using std::string;

static const uint16* GetCrc16Table()
  {
  static uint16 CrcTable[0x100];
  for(uint16 i = 0; i < _countof(CrcTable); ++i)
    {
    uint16 crc = i;
    for(size_t j = 0; j < 8; ++j)
      {
      crc = (crc >> 1) ^ ((crc & 1) ? 0xA001 : 0);
      }
    CrcTable[i] = crc;
    }
  return CrcTable;
  }

uint16 crc16(const void* data, const size_t size)
  {
  static const uint16* const ct = GetCrc16Table();
  uint16 ret = 0;
  const unsigned char* buf = (const unsigned char*)data;
  for(size_t i = 0; i < size; ++i)
    {
    ret = ct[(ret & 0xFF) ^ buf[i]] ^ (ret >> 8);
    }
  return ret;
  }

static const uint32* GetCrc32Table()
  {
  static uint32 CrcTable[0x100];
  for(uint32 i = 0; i < _countof(CrcTable); ++i)
    {
    uint32 crc = i;
    for(size_t j = 0; j < 8; ++j)
      {
      crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320 : 0);
      }
    CrcTable[i] = crc;
    }
  return CrcTable;
  }

uint32 crc32(const void* data, const size_t size)
  {
  static const uint32* const ct = GetCrc32Table();
  uint32 ret = 0xFFFFFFFF;
  const unsigned char* buf = (const unsigned char*)data;
  for(size_t i = 0; i < size; ++i)
    {
    ret = ct[ (ret & 0xFF) ^ buf[i] ] ^ (ret >> 8);
    }
  return ~ret;
  }

static const uint64* GetCrc64Table()
  {
  static uint64 CrcTable[0x100];
  for(uint64 i = 0; i < _countof(CrcTable); ++i)
    {
    uint64 crc = i;
    for(size_t j = 0; j < 8; ++j)
      {
      crc = (crc >> 1) ^ ((crc & 1) ? 0xC96C5795D7870F42 : 0);
      }
    CrcTable[i] = crc;
    }
  return CrcTable;
  }

uint64 crc64(const void* data, const size_t size)
  {
  static const uint64* const ct = GetCrc64Table();
  uint64 ret = 0xFFFFFFFFFFFFFFFF;
  const unsigned char* buf = (const unsigned char*)data;
  for(size_t i = 0; i < size; ++i)
    {
    ret = ct[ (ret & 0xFF) ^ buf[i] ] ^ (ret >> 8);
    }
  return ~ret;
  }

static const uint16* GetCrcCcittTable()
  {
  static uint16 CrcTable[0x100];
  for(uint16 i = 0; i < _countof(CrcTable); ++i)
    {
    uint16 crc = i;
    for(size_t j = 0; j < 8; ++j)
      {
      crc = (crc >> 1) ^ ((crc & 1) ? 0x8408 : 0);
      }
    CrcTable[i] = crc;
    }
  return CrcTable;
  }

uint16 crcccitt(const void* data, const size_t size)
  {
  static const uint16* const ct = GetCrcCcittTable();
  uint16 ret = 0xFFFF;
  const unsigned char* buf = (const unsigned char*)data;
  for(size_t i = 0; i < size; ++i)
    {
    ret = ct[(ret & 0xFF) ^ buf[i]] ^ (ret >> 8);
    }
  return ret;
  }

#ifdef _XLIB_TEST_

ADD_XLIB_TEST(CRC)
  {
  SHOW_TEST_INIT;

  bool done = false;

  const string data("1234567890");

  SHOW_TEST_HEAD("crc16");
  done = (50554 == crc16(data));
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("crc32");
  done = (639479525 == crc32(data));
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("crc64");
  done = (0xB1CB31BBB4A2B2BE == crc64(data));
  SHOW_TEST_RESULT(done);

  SHOW_TEST_HEAD("crcccitt");
  done = (0xB4EC == crcccitt(data));
  SHOW_TEST_RESULT(done);
  }

#endif  // _XLIB_TEST_