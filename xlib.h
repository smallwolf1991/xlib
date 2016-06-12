/*!
  \file  xlib.h
  \brief 用于预编译包含，也是不指定功能的整体包含。

  \section more 额外说明

  - \b 注意 xlib_base需要在几个基本文件之前，否则编译重定义
  - \b 注意 注释处'-'代表只有头文件，'+'代表有对应CPP
  - \b 注意 文件名全部小写表示Ring0与Ring3通用\n
            有大写字母表示目前只适用于Ring3

  \code
    #include <xlib.h>           //使xlib库所有功能可用
    //如果不需要包含xlib库所有功能，也可根据需要包含相应的头文件，如：
    #include <ws_s.h>           //只使用xlib库的ws_s功能
  \endcode

  \author     triones
  \date       2014-01-06
*/

#pragma once

#include "xlib_base.h"          //+20140106   locked        base/base
//          xlib_link.h
//          xlib_struct.h
//          xlib_struct_ring0.h
//          xlib_struct_ring3.h
#include "xlib_def.h"           //-20140215   locked        base/def
#include "xlib_link.h"          //-20140106   locked        base/link
#include "xlib_struct.h"        //-20131206   open          base/struct
//          xlib_def.h
#include "xlib_struct_ring0.h"  //-20131206   open          base/struct
//          xlib_def.h
#include "xlib_struct_ring3.h"  //-20131206   open          base/struct
//          xlib_def.h

//以下文件都默认包含xlib_base.h
#include "xlib_nt.h"            //+20140107   open          base/nt

#include "swap.h"               //-20140107   locked        memory/swap
#include "xrand.h"              //+20140725   locked        algorithm/xrand
#include "crc.h"                //+20140725   locked        algorithm/crc
#include "xmutex.h"             //+20130301   locked        system_object/mutex
#include "xevent.h"             //+20130301   locked        system_object/event
#include "Critical.h"           //+20121112   close         system_object/critical
#include "singleton.h"          //-20121112   locked        container/singleton
#include "ws_s.h"               //+20160314   locked        string/ws&s
//              xlib_nt.h
#include "ws_utf8.h"            //+20140507   locked        string/ws&utf8
#include "xblk.h"               //+20131210   locked        memory/xblk
//              swap.h
#include "xmsg.h"               //+20140409   locked        string/xmsg
//          ws_utf8.h
//              ws_s.h
#include "xlog.h"               //+20131210   locked        string/xlog
//          xmsg.h
#include "hex_str.h"            //+20150509   locked        string/hex&str
//          xmsg.h
//              ws_s.h
//              ws_utf8.h
#include "xline.h"              //-20150122   locked        container/xline
//          swap.h
#include "md5.h"                //+20140121   locked        algorithm/md5
//              swap.h
//              hex_str.h
#include "tean.h"               //+20160311   locked        algorithm/tean
//          xline.h
//              xrand.h
#include "aes.h"                //+20131210   locked        algorithm/aes
//          xline.h
#include "syssnap.h"            //+20140219   to add        system_object/syssnap
//          xline.h
//              xlib_nt.h
#include "pe.h"                 //+20140508   to add        file/pe
//          xblk.h
//              ws_s.h
//              sysinfo.h
#include "hook.h"               //+20160612   check         memory/hook
//              xlib_nt.h
//              xblk.h
//              xline.h
//              xmsg.h
//              hex_str.h
//              xlog.h
//              syssnap.h
#include "caller.h"             //+20150721   locked        memory/caller
//              hook.h
//              xevent.h
#include "xWSA.h"               //+20121113   locked        sock/wsa
//          xmsg.h
//              swap.h
#include "xSock.h"              //+20121113   close         sock/xsock
//          xWSA.h
//          xline.h
//          xlog.h
#include "xNetDl.h"             //+20150123   check         sock/netdl
//              xWSA.h
//              xlog.h
#include "xServer.h"            //+20121114   close         sock/xserver
//          xSock.h
//          ws_s.h
//          xline.h
#include "signaturematcher.h"   //+20160406   check         algorithm/signaturematcher
//          xline.h
//          xblk.h
//              xlog.h
//              hex_str.h
//              ws_s.h
#include "des.h"                //+20150428   check         algorithm/des