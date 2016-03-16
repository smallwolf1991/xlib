/*!
  \file  hex_str.h
  \brief hex_str.h定义了hex与str的转换操作

  \version    7.3.1408.1310
  \note       For All

  \author     triones
  \date       2010-03-03
*/

#pragma once

#include "xmsg.h"

typedef std::basic_string<unsigned char> hexstring;

//! 十六进制值的结构体
/*!
  主要体现BYTE与十六进制字符间的关系

  \code
    HEX_VALUE_STRUCT hexs = {1,4};  //hexs为字符'A'
  \endcode
*/
#pragma pack (push,1)
struct HEX_VALUE_STRUCT
  {
  unsigned char low:    4;
  unsigned char high:   4;
  };
#pragma pack (pop)

//! 十六进制ASCII表现形式的结构体
struct HEX_ASCII_STRUCT
  {
  char high;
  char low;
  };

//! 一个byte所需的ASCII长度
const size_t gk_str2byte_len  = sizeof(HEX_ASCII_STRUCT);
//! 一个word所需的ASCII长度
const size_t gk_str2word_len  = gk_str2byte_len * sizeof(unsigned __int16);
//! 一个dword所需的ASCII长度
const size_t gk_str2dword_len = gk_str2byte_len * sizeof(unsigned __int32);
//! 一个qword所需的ASCII长度
const size_t gk_str2qword_len = gk_str2byte_len * sizeof(unsigned __int64);

//! 指定HEX串转换为ASCII格式(版本一)
/*!
  可选大小写，默认小写\n
  注意dest不能与hexs同位置，这样将导致转换错误\n
  最后追加结尾0\n
  \param  dest      目标缓冲区
  \param  hexs      源HEX串
  \param  destlen   目标缓冲区长度。 < 0 表示缓冲区足够\n
                    注意destlen >= (hexslen * 2 + 1)
  \param  hexslen   源HEX串长度。< 0 视hexs为C格式串。
  \param  isup      指定转换后的ASCII大小写，默认小写
  \return           返回转换后的长度，包括结尾0。\n
                    返回0表示转换错误。

  \code
    cout << "result len:" << hex2str(dest,"\xAB\xCD\xEF");
      //将输出result len:7，并且dest == "abcdef"
  \endcode
*/
size_t  hex2str(char*           dest,
                const void*     hexs,
                const size_t    destlen = -1,
                const size_t    hexslen = -1,
                const bool      isup    = false);

//! 指定HEX串转换为ASCII格式(版本二)
/*!
  可选大小写，默认小写。最后追加结尾0\n
  \param  hexs      源HEX串
  \param  isup      指定转换后的ASCII大小写，默认小写
  \return           返回转换后的ascii串对象

  \code
    string asc = hex2str(hexstring((unsigned char*)"\xAB\xCD\xEF"));
    if(asc.empty())
      {
      cout << "转换失败！";
      }
    else
      {
      cout << "hex2str:" << asc; //将输出"abcdef"
      }
  \endcode
  */
std::string hex2str(const hexstring& hexs, const bool isup = false);
std::string hex2str(const std::string& hexs, const bool isup = false);

//! 指定十六进制ASCII串转换为HEX值
/*!
  \param  strs      源ASCII串
  \param  lpreadlen 成功读取ASCII串时，返回读取的字符个数。\n
                    lpreadlen可以为nullptr，此时不返回结果。\n
                    转换失败，*lpreadlen == 0。
  \param  wantlen   需要处理的 \b 有效数据 大小(0<=wantlen<=8)。\n
                    当<=0或>8时，一律视==8。x64下大小为16
  \param  errexit   指定转换未结束前遭遇非HEX字符，是否视作失败处理。\n
                    默认不作失败处理。当此标志为true时，忽略errbreak。
  \param  errbreak  指定转换未结束前遭遇非HEX字符，是否中止处理。默认跳过
  \return           成功转换则为十六进制ASCII串对应的HEX值。\n
                    判定转换失败，应该通过lpreadlen的返回结果判定。

  \code
    unsigned int readlen = 0;
    const unsigned int hexvalue = str2hex("12345678",readlen);
    //返回hexvalue == 0x12345678; readlen == 8;
    const unsigned int hexvalue = str2hex("1234 56|78",readlen);
    //返回hexvalue == 0x12345678; readlen == 10;
    const unsigned int hexvalue = str2hex("1234 56|78",readlen,8,true);
    //返回hexvalue == 0; readlen == 0;
    const unsigned int hexvalue = str2hex("1234 56|78",readlen,8,false,ture);
    //返回hexvalue == 0x1234; readlen == 4;
  \endcode
*/
size_t str2hex(const std::string&  strs,
               size_t*             lpreadlen = nullptr,
               size_t              wantlen   = 0,
               const bool          errexit   = false,
               const bool          errbreak  = false);

//! 指定十六进制ASCII串转换为HEX串
/*!
  转换ASCII的十六进制字符应成对的，如最后字符不成对，将忽略最后不成对的字符
  \param  strs      源ASCII串
  \param  lpreadlen 成功读取ASCII串时，返回读取的字符个数。\n
                    lpreadlen可以为nullptr，此时不返回结果。\n
                    转换失败，*lpreadlen == 0。
  \param  errexit   指定转换未结束前遭遇非HEX字符，是否视作失败处理。\n
                    默认不作失败处理。当此标志为true时，忽略errbreak。
  \param  errbreak  指定转换未结束前遭遇非HEX字符，是否中止处理。默认跳过
  \return           成功转换则为十六进制ASCII串对应的HEX值。\n
                    判定转换失败，应该通过lpreadlen的返回结果判定。
*/
hexstring str2hexs(const std::string&  strs,
                   size_t*             lpreadlen = nullptr,
                   const bool          errexit = false,
                   const bool          errbreak = false);

//! 指定ASCII串，分析转义字符
/*!
  可转义的字符有：
  \code
    \0、\a、\b、\f、\n、\r、\t、\v、\\、\'、\"、\?
  \endcode
  另可转义\x########。
  \x识别十六进制数据时，根据读取长度，自动匹配类型(2:byte,4:short,8:int)
  \param    strs    源ASCII串(转换字串最大长度为0xFFFF)
  \return           返回转换后的ascii串对象
*/
std::string escape(const std::string& strs);

enum Hex2showCode
  {
  HC_ASCII    = 0,
  HC_UNICODE  = 1,
  HC_UTF8     = 2,
  };
//! 指定hex串，格式化显示
/*!
  \param  data      hex串
  \param  size      hex串长度
  \param  code      指明内容编码
  \param  isup      hex格式大小写控制
  \param  prews     前缀空格数
  \return           格式化后的内容

  \code

    char aa[] = "123456789aasdfsdhcf";
    cout << hex2show(aa,sizeof(aa));
    //0012FF54┃31 32 33 34|35 36 37 38|39 61 61 73|64 66 73 64┃123456789aasdfsd
    //0012FF64┃68 63 66 00|           |           |           ┃hcf.
  \endcode
*/
xmsg hex2show(const void*           data,
              intptr_t              size,
              const size_t          prews = 0,
              const Hex2showCode    code = HC_ASCII,
              const bool            isup = true);

//! 模版　指定hex串，格式化显示
template<typename T>
xmsg hex2show(const std::basic_string<T>& s,
              const size_t                prews = 0,
              const Hex2showCode          code = HC_ASCII,
              const bool                  isup = true)
  {
  return hex2show(s.c_str(), s.size() * sizeof(T), prews, code, isup);
  }