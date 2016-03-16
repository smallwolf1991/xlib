/*!
  \file  xline.h
  \brief xline.h定义了便捷数据组织的类，常用于封包的组织与解析

  - 数据不足的情况下读取数据默认将抛出runtime_error异常
  - 如不愿意抛出异常，请在包含前#define xline_noexcept。注意，此时将读出未知数据
  - Ring0不使用异常机制

  \version    9.1.1501.2211
  \note       For All

  \author     triones
  \date       2010-03-26
*/

#pragma once

#include "swap.h"
#include <string>

#define cnull  ('\0')
#define snull  (L'\0')

//xline用于便捷的数据组织操作
/*!
  \param  headtype    数据头类型，byte,word,dwork,qword
  \param  otherhead   指示数据头是否包含自身长度
  \param  net         输入输出的数据是否按网络顺序翻转
  \param  zeroend     指示处理流时是否追加处理结尾0
*/
template<
          typename  headtype,
          bool      otherhead,
          bool      net,
          bool      zeroend
        >
class xline : public std::basic_string<unsigned char>
  {
  public:
    typedef xline<headtype,otherhead,net,zeroend> _Myt;
    typedef std::basic_string<unsigned char>      _Mybase;
  public:
    /*========================  数据输入  ========================*/
    //! 模板：数据输入I
    /*!
      格式串输入\n
      注意：输入数据内容，根据标志处理结尾0

      \code
        line << "2121321";
        line << L"12312";
      \endcode
    */
    template<typename T> _Myt& operator<<(T* str)
      {
      if(str == nullptr)  return *this;

      size_type strlen = 0;
      while(str[strlen])  ++strlen;

      strlen += zeroend ? 1 : 0;
      strlen *= sizeof(T);

      _Mybase::append((unsigned char*)str, strlen);

      return *this;
      }
    //! 模板：数据输入II
    /*!
      模板适用于内置类型,结构体等

      \code
        line << dword << word << char;
      \endcode
    */
    template<typename T> _Myt& operator<<(const T& argvs)
      {
      const T v = net ? bswap(argvs) :argvs;

      _Mybase::append((unsigned char*)&v, sizeof(v));

      return *this;
      }
    //! 模板特化：数据输入III。由于void*指向无效大小数据，故特化之
    template<> _Myt& operator<<(void* str)
      {
      return this->operator<<((size_t)str);
      }
    //! 扩展功能，使得line可以执行一个函数
    _Myt& operator<<(_Myt& (*pfn)(_Myt&))
      {
      return pfn(*this);
      }
    //! 扩展功能，使得不借助两个line就能完成头部添加
    _Myt& mkhead()
      {
      const size_type head_size = sizeof(headtype);
      const size_type nlen = _Mybase::size() + (otherhead ? head_size : 0);
      const headtype v = (net ? bswap((headtype)nlen) : (headtype)nlen);

      _Mybase::insert(0, (unsigned char*)&v, head_size);

      return *this;
      }
    //! 模板特化：数据输入IV
    /*!
      先输入数据长度，再输入数据内容\n
      注意区别operator=、+=(const line& nline)操作\n
      前两个操作将直接输入数据内容，不会将数据长度输入在缓冲区\n
      这个操作会将数据长度先输入缓冲，（输入格式由net决定），然后才是源数据\n
      它们与本操作在操作结果：缓冲长度、缓冲内容上都有区别！\n
      注意：当操作自身时，相当于mkhead()

      \code
        line << tmpline;
      \endcode
    */
    template<> _Myt& operator<<(const _Myt& nline)
      {
      if(&nline == this)
        {
        return mkhead();
        }

      const size_type nlen = nline.size() + (otherhead ? sizeof(headtype) : 0);
      const headtype xlen = (headtype)nlen;

      this->operator<<(xlen);
      _Mybase::append(nline.begin(), nline.end());

      return *this;
      }
  public:
    /*========================  数据输出  ========================*/
    //! 数据输出I
    /*!

      \code
        line >> (void*)xx;
      \endcode
    */
    _Myt& operator>>(void*& v)
      {
      return this->operator>>((size_t&)v);
      }
    //! 模板：数据输出II
    /*!
      格式串输出，注意不能特化void* !\n
      输出数据内容，根据标志处理结尾0\n
      允许空指针，这样将丢弃一串指定类型的数据

      \code
        line >> (unsigned char*)lpstr;
      \endcode
      \exception 数据不足时，抛出runtime_error异常
    */
    template<typename T> _Myt& operator>>(T* str)
      {
      T* lpstr = (T*)_Mybase::c_str();

      size_type strlen = 0;
      while(lpstr[strlen])++strlen;

#if !defined(FOR_RING0) && !defined(xline_noexcept)
      if(strlen * sizeof(T) > size())
        {
        throw std::runtime_error("xline >> T* 数据不足");
        }
#endif

      strlen += zeroend ? 1 : 0;
      strlen *= sizeof(T);

      if(str != nullptr)
        memcpy(str, lpstr, strlen);

      _Mybase::erase(0, strlen);

      return *this;
      }
    //! 模板：数据输出III
    /*!
      模板适用于内置类型,结构体等

      \code
        line >> dword >> word >> char;
      \endcode
      \exception 数据不足时，抛出runtime_error异常
    */
    template<typename T> _Myt& operator>>(T& argvs)
      {
      size_type typesize = sizeof(T);

#if !defined(FOR_RING0) && !defined(xline_noexcept)
      if(typesize > size())
        {
        throw std::runtime_error("xline >> T& 数据不足");
        }
#endif

      memcpy(&argvs, _Mybase::c_str(), typesize);

      _Mybase::erase(0, typesize);
      argvs = net ? bswap(argvs) : argvs;

      return *this;
      }
    //! 模板特化：数据输出IV
    /*!
      将重置参数line\n
      执行后line缓冲存放数据内容，line的缓冲长度即为数据长度。\n
      注意区别operator=、+=(const line& nline)操作\n
      前两个操作将直接导致nline数据内容全部复制\n
      本操作复制的数据长度取决于缓冲区当前short数据\n
      它们与本操作在操作结果：缓冲长度、缓冲内容上都有区别！\n
      注意：当操作自身时，相当于丢弃数据头

      \code
        line >> tmpline;
      \endcode
      \exception 数据不足时，抛出runtime_error异常
    */
    template<> _Myt& operator>>(_Myt& nline)
      {
      headtype xlen;
      this->operator>>(xlen);

      if(&nline == this)  return *this;

      size_type nlen = xlen;
      nlen -= (otherhead ? sizeof(headtype) : 0);

#if !defined(FOR_RING0) && !defined(xline_noexcept)
      if(nlen > size())
        {
        throw std::runtime_error("xline >> _Myt& 数据不足");
        }
#endif

      nline.assign(_Mybase::c_str(), nlen);
      _Mybase::erase(0, nlen);

      return *this;
      }
    //! 模板：数据输出V
    /*!
      目的用以跳过某些不需要的数据

      \code
        line >> cnull >> snull >> 0;
      \endcode
      \exception 数据不足时，抛出runtime_error异常
    */
    template<typename T> _Myt& operator>>(T const&)
      {

#if !defined(FOR_RING0) && !defined(xline_noexcept)
      if(sizeof(T) > size())
        {
        throw std::runtime_error("xline >> _Myt& 数据不足");
        }
#endif
      _Mybase::erase(0, sizeof(T));

      return *this;
      }
  };

//! line数据头为us，不包含自身，机器顺序，不处理结尾0
typedef xline<unsigned short,false,false,false>   line;
//! netline数据头为us,不包含自身，网络顺序，处理结尾0
typedef xline<unsigned short,false,true,true>     netline;