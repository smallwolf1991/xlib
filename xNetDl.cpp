#include "xNetDl.h"

#ifndef FOR_RING0

#include <tchar.h>
#include <WinInet.h>
#pragma comment (lib,"wininet.lib")

#include "xWSA.h"
#include "xlog.h"

using namespace std;

static string err(const bool except, const xmsg& msg)
  {
  if(except)
    {
    throw runtime_error(msg);
    }
  xerr << msg;
  return string();
  }

static string err(const bool except, const char* msg)
  {
  return err(except, xmsg() << msg);
  }

string DownloadUrl(LPCTSTR url, const bool except)
  {
  try
    {
    static xWSA wsa;
    }
  catch(const runtime_error& e)
    {
    return err(except, e.what());
    }

  HINTERNET hNETIO = InternetOpen(NULL,INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,0);
  if(!hNETIO)
    {
    return err(except, "InternetOpen fail!");
    }

  HINTERNET	hIO = InternetOpenUrl(hNETIO,
    url,NULL,0,
    INTERNET_FLAG_RELOAD |	INTERNET_FLAG_NO_CACHE_WRITE	|	INTERNET_FLAG_KEEP_CONNECTION	|	INTERNET_FLAG_PRAGMA_NOCACHE,
    0);
  if(!hIO)
    {
    CloseHandle(hNETIO);
    return err(except, "InternetOpenUrl失败!");
    }

  TCHAR strfilelen[0x10];			//注意这个长度
  int filelen_len = sizeof(strfilelen);
  if(!HttpQueryInfo(hIO,HTTP_QUERY_CONTENT_LENGTH,&strfilelen,(LPDWORD)&filelen_len,0))
    {
    _tcscpy_s(strfilelen, TEXT("0"));
//     CloseHandle(hIO);
//     CloseHandle(hNETIO);
//     return err(except, "HttpQueryInfo失败!");
    }

  string data;
  const int len = _tstoi(strfilelen);
  if(len != 0)  data.reserve(len);

  int remain = 0;
  while(InternetQueryDataAvailable(hIO,(LPDWORD)&remain,0,0))
    {
    if(remain==0)
      {
      CloseHandle(hIO);
      CloseHandle(hNETIO);
      if(len != 0 && len != (int)data.size())
        {
        return err(except,
                   xmsg() << "DownloadUrl数据长度不一致:" << data.size() << " - " << len);
        }
      return data;
      }

    int readed;
    if(!InternetReadFile(hIO,(LPVOID)(data.end()._Ptr),remain,(LPDWORD)&readed))
      {
      CloseHandle(hIO);
      CloseHandle(hNETIO);
      return err(except,
                 xmsg() << "InternetReadFile失败!已读:" << data.size());
      }

    data.append(data.end()._Ptr,readed);
    }

  CloseHandle(hIO);
  CloseHandle(hNETIO);

  return err(except,
             xmsg() << "InternetQueryDataAvailable失败!已读:" << data.size());
  }

#endif  //#ifndef FOR_RING0