﻿#ifdef _WIN32

#pragma warning(push)
#pragma warning(disable:4512)     //SIG_STL无法生成赋值运算符
#pragma warning(disable:4100)

#include "signaturematcher.h"

#include "hex_str.h"
#include "ws_s.h"
#include "pe.h"
#include "syssnap.h"
#include <bitset>

using namespace std;

///////////////////////////调试信息输出设置///////////////////////////////
namespace signaturematcher
  {
  class smlog : public xmsg
    {
    public:
      virtual ~smlog()
        {
        if(empty())  return;
        if(func != nullptr)
          {
          append("\r\n");
          func(c_str());
          clear();
          return;
          }
#ifndef FOR_RING0
        OutputDebugStringA(c_str());
#else
        DbgPrint("%s\n", c_str());
#endif
        clear();
        }
    public:
      static xlog::levels log_level;
      static log_out_func func;
    };

  xlog::levels smlog::log_level = xlog::lvl_error;

  log_out_func smlog::func = nullptr;

#define smlog_do(v)   if((v) <= log_level()) smlog()
#define smtrace       smlog_do(xlog::lvl_trace)
#define smerr         smlog_do(xlog::lvl_error)
  }

////////////////////////////特征码字符串类////////////////////////////////
namespace signaturematcher
  {
  //! 特征码字符串类，用于操纵字符串的遍历
  class Sign
    {
    public:
      Sign(SIGNATURE sig)
        :sign(sig), scaner(0)
        {
        ;
        }
      //! 返回当前处理的字符
      char ch() const
        {
        return sign[scaner];
        }
      //! 向后移动一个字符
      void step()
        {
        ++scaner;
        }
    private:
      friend static xmsg& operator<<(xmsg& msg, const Sign& s);
    private:
      SIGNATURE   sign;             //!< 存放特征码字符串指针
      intptr_t    scaner;           //!< 指示当前解析字符起始索引
    };

#pragma warning(disable:4239)
  //! 重载输出操作以输出当前处理信息
  static xmsg& operator<<(xmsg& msg, const Sign& s)
    {
    intptr_t row = 1;
    intptr_t col = 1;
    const intptr_t lp = s.scaner;
    for(intptr_t i = 0; i < lp; ++i)
      {
      switch(s.sign[i])
        {
        //注意只识别\n以兼容Unix
        case '\n':
          col = 0;  ++row;
          break;
        case '\0':
          return msg << "第" << row << "行" << col << "列[越界错误]";
        default:;
        }
      ++col;
      }
    return msg << "第" << row << "行" << col << "列:";
    }
  }

///////////////////////////////词法类型///////////////////////////////////
namespace signaturematcher
  {
  enum LexicalType : unsigned char
    {
    LT_Error,       //!< 错误词法
    LT_End,         //!< end          -> \0
                    //!< ws           -> [ \t\n\r]*           //空白词法不生成类型
                    //!< note         -> #.*\n|\0             //注释词法不生成类型
                    //!< 以下词法不单独成词
                    //!< hex          -> [0-9A-Fa-f]
                    //!< range_value  -> {hex}({ws}{hex}){1,7}//x64下为{hex}({ws}{hex}){0,15}
                    //!< range_delimiter ->  [,-~:\|]
                    //!< range        -> {ws}([\*\+\?]) | (\{{ws}{range_value}?{ws}{range_delimiter}?{ws}{range_value}?{ws}\})
                    //!< hexhex       -> ({hex}{ws}{hex}) | (\:[^\0])
    LT_String,      //!< string       -> [L|l]?{ws}\'[^\0]+\'
    LT_Quote,       //!< quote        -> [L|l]?{ws}\"[^\0]+\"
    LT_Dot,         //!< dot          -> \.{range}?
    LT_MarkRef,     //!< markreg      -> {hexhex}({ws}@{ws}L)?({ws}@{ws}R)?
                    //!< refreg       -> {hexhex}({ws}${ws}[1-7]{ws}L)?({ws}${ws}[1-7]{ws}R)?
    LT_Ucbit,       //!< ucbit        -> {hexhex}({ws}[\-\|\&]{ws}{hexhex})*{range}?
    LT_ConstHex,    //!< consthex     -> {hexhex}{range}?
                    //!< collection   -> \[\^?{hexhex}({ws}[\&\|\&]?{ws}{hexhex})*{ws}\]{range}?
    LT_Record_Addr,
    LT_Record_Offset,
    LT_Record_Qword,
    LT_Record_Dword,
    LT_Record_Word,
    LT_Record_Byte, //!< record       -> \<{ws}\^?{ws}[AFQDWB]{ws}[^ \t\n\r\0>]*{ws}[^ \t\n\r\0]*{ws}\>
    };
  }

///////////////////////////range标识结构//////////////////////////////////
namespace signaturematcher
  {
  typedef intptr_t      RangeType;       //!< 范围类型

  //! 范围指示结构
  struct Range
    {
    RangeType  N;     //!< 最小匹配次数
    RangeType  M;     //!< 最大匹配次数
    //! 简单初始化
    Range(const RangeType& n, const RangeType& m)
      :N(n), M(m)
      {
      ;
      }
    //! 指定固定范围
    Range(const RangeType& n)
      :N(n), M(n)
      {
      ;
      }
    };

  //! 用以标识错误或最大范围指示值
  static const RangeType gk_max_range =
#ifdef _WIN64
    0x7FFFFFFFFFFFFFFF;
#else
    0x7FFFFFFF;
#endif

  //! 用以标识错误的范围
  static const Range  gk_err_range = { -1, -1 };

  //! 用以标识缺省的范围
  static const Range  gk_normal_range = { 1, 1 };

  //! 用以标识空的范围
  static const Range gk_none_range = { 0, 0 };

  //! 比较范围是否相同
  static bool operator==(const Range& ra, const Range& rb)
    {
    return (ra.N == rb.N) && (ra.M == rb.M);
    }

  //! 范围融合
  static Range& operator+=(Range& ra, const Range& rb)
    {
    const RangeType N = ra.N + rb.N;
    ra.N = (N < ra.N) ? gk_max_range : N;

    const RangeType M = ra.M + rb.M;
    ra.M = (M < ra.M) ? gk_max_range : M;

    return ra;
    }
  }

///////////////////////////////词法基类///////////////////////////////////
namespace signaturematcher
  {
  class LexicalBase
    {
    public:
      LexicalBase(LexicalType t = LT_Error,const Range& r = gk_normal_range)
        :type(t), range(r), match_count(-1)
        {
        ;
        }
      //! 空虚析构，必要存在，以使继承类能正确释放资源
      virtual ~LexicalBase()
        {
        ;
        }
      virtual const char* type_name() const
        {
        return "词法end/未命名/错误类型";
        }
      //! 用以指示是否有效词法
      bool valid() const
        {
        return type != LT_Error;
        }
      //! 用以整合同类型，返回表示融合失败，融合成功后注意释放资源。默认不融合
      virtual bool fusion(LexicalBase*)
        {
        return false;
        }
      //! 用以指示是否特征点，默认不是
      virtual bool record() const
        {
        return false;
        }
      //! 指定内存范围和索引，进行匹配，匹配失败返回false，失败且lp > blk.size()时，彻底失败
      bool match(const xblk& blk, RangeType& lp)
        {
        void* pp = (unsigned char*)blk.start() + lp;
        if(match_count >= range.M)
          {
          smtrace << (void*)((unsigned char*)pp - match_count)
            << ' ' << type_name()
            << "^匹配范围达到最大值：" << (void*)range.M << "，回退";
          lp -= match_count;
          match_count = -1;
          return false;
          }

        if(match_count < range.N)
          {
          if((RangeType)blk.size() < (lp + range.N))
            {
            smtrace << pp << ' ' << type_name()
              << "^无法实现最低" << (void*)range.N << "次匹配，失败！";
            lp = gk_max_range;
            return false;
            }
          smtrace << pp << ' ' << type_name()
            << "^正在进行最低" << (void*)range.N << "次匹配";
          const xblk mem(pp, range.N);
          if(!test(mem))
            {
            smtrace << pp << ' ' << type_name()
              << "^最低匹配失败，回退";
            return false;
            }
          match_mem = pp;
          lp += range.N;
          match_count = range.N;
          return true;
          }

        if((RangeType)blk.size() < (lp + 1))
          {
          smtrace << pp << ' ' << type_name() << "^已经无法递进匹配了，失败！";
          lp = gk_max_range;
          return false;
          }
        smtrace << pp << ' ' << type_name()
          << "^正在进行递进匹配";
        const xblk mem(pp, 1);
        if(!test(mem))
          {
          smtrace << (void*)((unsigned char*)pp - match_count)
            << ' ' << type_name() << "^递进匹配失败，回退";
          lp -= match_count;
          match_count = -1;
          return false;
          }
        ++lp;
        ++match_count;
        return true; 
        }
      //! 设计防止内存不可读产生异常
      virtual bool chk(const xblk& blk) const
        {
#ifdef FOR_RING0
        const size_t base_page = 0x1000;
        if(blk.size() == 0) return true;
        size_t s = (size_t)blk.start() / base_page * base_page;
        size_t e = ((size_t)blk.end() - 1) / base_page * base_page;
        do 
          {
          if(!MmIsAddressValid((PVOID)s)) return false;
          s += base_page;
          }while(s <= e);
        return true;
#else
        return FALSE == IsBadReadPtr(blk.start(), blk.size());
#endif
        }
      //! 用以给定内存段细节匹配，默认匹配。需要细节匹配则重载之
      virtual bool test(const xblk& blk) const
        {
        return chk(blk);
        }
      //! 输出原子细节
      virtual void to_atom(line&) const
        {
        return;
        }
      //! 用以输出原子
      void create_atom(line& atom) const
        {
        atom << type << range;
        to_atom(atom);
        }
    public:
      LexicalType   type;             //!< 指示词法类型
      Range         range;            //!< 指示词法匹配内存大小
      RangeType     match_count;      //!< 指示词法在匹配过程中匹配的大小
      void*         match_mem;        //!< 用以记录匹配位置
    };
  }

////////////////////词法end、ws、note、hex识别函数////////////////////////
namespace signaturematcher
  {
  //! 识别词法end，成功返回LT_End，否则返回nullptr
  static LexicalBase* match_end(Sign& sig)
    {
    if(sig.ch() != '\0')  return nullptr;
    sig.step();
    smtrace << "识别到词法end";
    return new LexicalBase(LT_End);
    }
  //! 识别词法ws，只是跳过ws，无论成功与否都返回nullptr
  static LexicalBase* match_ws(Sign& sig)
    {
    while(true)
      {
      switch(sig.ch())
        {
        case ' ':     case '\t':   case '\n':    case '\r':
          sig.step(); continue;
        default:;
        }
      break;
      };
    return nullptr;
    }
  //! 识别词法note，只是跳过note，无论成功与否都返回nullptr
  static LexicalBase* match_note(Sign& sig)
    {
    //建立循环是为了处理连继注释的情况
    while(sig.ch() != '\0')
      {
      if(sig.ch() != '#') return nullptr;
      sig.step();
      while(sig.ch() != '\n' && sig.ch() != '\0')
        sig.step();
      smtrace << "跳过词法note";
      match_ws(sig);
      }
    return nullptr;
    }
  //! 匹配hex词法，返回值 < 0表示非此词法
  static char match_hex(Sign& sig)
    {
    switch(sig.ch())
      {
      case '0':     case '1':    case '2':     case '3':
      case '4':     case '5':    case '6':     case '7':
      case '8':     case '9':
      case 'A':     case 'B':    case 'C':     case 'D':
      case 'E':     case 'F':
      case 'a':     case 'b':    case 'c':     case 'd':
      case 'e':     case 'f':
        {
        char hex = sig.ch() & 0x0F;
        if(sig.ch() > '9')  hex += 0x09;
        sig.step();
        return hex;
        }
      default:;
      }
    return -1;
    }
  }

///////////////////////////词法range提取函数//////////////////////////////
namespace signaturematcher
  {
  //! 匹配词法range_value，返回值 < 0表示非此词法
  static RangeType match_range_value(Sign& sig)
    {
    RangeType r = -1;

    char hex = match_hex(sig);
    if(hex < 0) return r;
    r = hex;
    
    for(size_t i = 0; i < (2 * sizeof(RangeType) - 1); ++i)
      {
      match_ws(sig);
      hex = match_hex(sig);
      if(hex < 0) return r;
      r = (r << 4) | hex;
      }

    return r;
    }
  //! 匹配词法range_delimiter
  static bool match_range_delimiter(Sign& sig)
    {
    switch(sig.ch())
      {
      case ',':     case '-':   case '~':   case '|':   case ':':
        sig.step(); return true;
      default:;
      }
    return false;
    }
  //! 匹配词法range，返回值 == gk_err_range时，匹配错误。
  static Range match_range(Sign& sig)
    {
    Range range = gk_normal_range;

    match_ws(sig);

    switch(sig.ch())  //先识别单一字符的范围指示
      {
      case '*':
        smtrace << "  识别到范围*";
        range.N = 0;  range.M = gk_max_range;
        sig.step();
        return range;
      case '+':
        smtrace << "  识别到范围+";
        range.N = 1;  range.M = gk_max_range;
        sig.step();
        return range;
      case '?':
        smtrace << "  识别到范围?";
        range.N = 0;  range.M = 1;
        sig.step();
        return range;
      case '{':
        sig.step();
        break;
      default:
        smtrace << "  无范围指示，返回默认范围{1,1}";
        return gk_normal_range;
      }
    match_ws(sig);

    range.N = match_range_value(sig);
    bool needM = true;
    if(range.N >= 0)                    //存在N值
      {
      smtrace << "  识别到范围N值：" << (void*)range.N;
      match_ws(sig);
      if(!match_range_delimiter(sig))   //存在N值，但没有分隔符{N}
        {
        smtrace << "    范围分隔符不存在，不再提取M值";
        range.M = range.N;
        needM = false;                  //没有分隔符的情况下，不能继续提取M值
        }
                                        //存在N值且有分隔符的情况下，可能是{N, } | {N, M}
      }
    else                                //不存在N值
      {
      smtrace << "  没有识别到范围N值";
      match_ws(sig);
      if(!match_range_delimiter(sig))   //不存在N值且无分隔符，非法{}
        {
        smerr << sig << "范围指示识别失败！";
        return gk_err_range;
        }
      smtrace << "     存在分隔符，N = 0";
      range.N = 0;                      //不存在N值但有分隔符，可能是{, } | {, M}
      }

    if(needM)
      {
      match_ws(sig);
      range.M = match_range_value(sig);
      if(range.M >= 0)                  //存在M值{N, M} | {, M}
        {
        smtrace << "  识别到范围M值：" << (void*)range.M;
        }
      else                              //不存在M值{N, } | { , }
        {
        smtrace << "  没有识别到范围M值，M为最大值";
        range.M = gk_max_range;
        }
      }

    match_ws(sig);
    if(sig.ch() != '}')
      {
      smerr << sig << "范围指示缺少'}'结束/存在非法字符/超越最大允许位数！";
      return gk_err_range;
      }
    sig.step();

    if(range.N == 0 && range.M == 0)
      {
      smerr << sig << "非法的空范围，请检查范围指示！";
      return gk_err_range;
      }

    seqswap(range.N, range.M);
    smtrace << "  确定范围：{" << (void*)range.N << ',' << (void*)range.M << "}";
    return range;
    }
  }

/////////////////////词法string、quote类及识别函数////////////////////////
namespace signaturematcher
  {
  class LexicalString : public LexicalBase
    {
    public:
      LexicalString(const string& s)
        :LexicalBase(LT_String, Range(s.size())), str(s)
        {
        ;
        }
      virtual const char* type_name() const
        {
        return "词法string";
        }
      virtual bool fusion(LexicalBase* lb)
        {
        if(lb->type != LT_String) return false;
        str.append(((LexicalString*)lb)->str);
        range = Range(str.size());    //注意不是+=
        return true;
        }
      virtual bool test(const xblk& blk) const
        {
        if(!chk(blk)) return false;
        smtrace << "    匹配string\r\n"
          << hex2show(str, (size_t)0)
          << hex2show(string((const char*)blk.start(), blk.size()), (size_t)6);
        if(blk.size() != str.size())  return false;
        return memcmp(str.c_str(), blk.start(), str.size()) == 0;
        }
      virtual void to_atom(line& nline) const
        {
        nline << str.size();
        nline.append((const unsigned char*)str.c_str(), str.size());
        }
    public:
      string str;
    };
  class LexicalQuote : public LexicalBase
    {
    public:
      LexicalQuote(const string& s)
        :LexicalBase(LT_Quote, Range(sizeof(void*))), str(s)
        {
        ;
        }
      virtual const char* type_name() const
        {
        return "词法quote";
        }
      //继承funcsion、test
      virtual void to_atom(line& nline) const
        {
        nline << str.size();
        nline.append((const unsigned char*)str.c_str(), str.size());
        }
    public:
      string str;
    };
  //! 识别词法string、quote，成功返回LT_String或LT_Quote，非词法返回nullptr，失败返回LT_Error
  static LexicalBase* match_string_quote(Sign& sig)
    {
    bool unicode = false;
    if(sig.ch() == 'L' || sig.ch() == 'l')
      {
      sig.step();
      unicode = true;
      match_ws(sig);
      smtrace << "识别到Unicode标识";
      }

    const char ch = sig.ch();
    if(ch != '\"' && ch != '\'' || ch == '\0')
      {
      if(!unicode)  return nullptr;
      smerr << sig << "不允许单独的Unicode标识'L'";
      return new LexicalBase(LT_Error);
      }
    sig.step();

    string str;
    while(sig.ch() != '\0')
      {
      if(sig.ch() != ch)
        {
        //简单处理\'或\"的情况
        if(sig.ch() == '\\')
          {
          sig.step();
          if(sig.ch() == ch)
            {
            str.append(1, ch);
            sig.step();
            }
          else
            {
            str.append(1, '\\');
            }
          continue;
          }

        str.append(1, sig.ch());
        sig.step();
        continue;
        }

      if(str.empty())
        {
        smerr << sig << "字符串/引用串不可以为空";
        return new LexicalBase(LT_Error);
        }
      sig.step();

      smtrace << "字符串提取完成：" << str;
      const string s(escape(str));

      smtrace << "字符串转换完成\r\n" << hex2show(s, (size_t)2);

      if(unicode)
        {
        const wstring ws(s2ws(s));
        str.assign((const char*)ws.c_str(), ws.size()* sizeof(wchar_t));
        smtrace << "字符串转换为Unicode\r\n"
          << hex2show(str.c_str(), str.size(), HC_UNICODE, true, 2);
        }
      else
        {
        str = s;
        }

      if(ch == '\"')  return new LexicalQuote(str);
      return new LexicalString(str);
      }

    smerr << sig << "缺少" << ch << "来界定结束";
    return new LexicalBase(LT_Error);
    }
  }

////////////////////////词法dot类及识别函数///////////////////////////////
namespace signaturematcher
  {
  class LexicalDot : public LexicalBase
    {
    public:
      LexicalDot(const Range& range)
        :LexicalBase(LT_Dot, range)
        {
        ;
        }
      virtual const char* type_name() const
        {
        return "词法dot";
        }
      virtual bool fusion(LexicalBase* lb)
        {
        if(lb->type != LT_Dot)  return false;
        range += ((LexicalDot*)lb)->range;
        return true;
        }
    };
  //! 识别词法dot，成功返回LT_Dot，非词法返回nullptr，失败返回LT_Error
  static LexicalBase* match_dot(Sign& sig)
    {
    if(sig.ch() != '.') return nullptr;
    sig.step();
    smtrace << "识别到词法dot";
    Range range = match_range(sig);
    if(range == gk_err_range) return new LexicalBase(LT_Error);
    return new LexicalDot(range);
    }
  }

//////////////////////////////ucbit类/////////////////////////////////////
namespace signaturematcher
  {
  class ucbit : public bitset<0x100>
    {
    public:
      //! 添加模糊匹配组
      ucbit& set_obfus(const unsigned char TA, const unsigned char TB)
        {
        for(size_t i = 0; i < size(); ++i)
          {
          if(!((TA^i) & (TB^i)))
            set(i);
          }
        return *this;
        }
      //! 添加连续组
      ucbit& set_queue(unsigned char TA, unsigned char TB)
        {
        seqswap(TA, TB);
        for(size_t i = TA; i <= TB; ++i)
          set(i);
        return *this;
        }
    };
  }

/////////////////词法ucbit、consthex、markreg、refreg类///////////////////
namespace signaturematcher
  {
  class LexicalUcbit : public LexicalBase
    {
    public:
      LexicalUcbit(const ucbit& u, const Range& range = gk_normal_range)
        :LexicalBase(LT_Ucbit, range), uc(u)
        {

        }
      virtual const char* type_name() const
        {
        return "词法ucbit";
        }
      virtual bool fusion(LexicalBase* lb)
        {
        if(lb->type != LT_Ucbit)  return false;
        if(uc != ((LexicalUcbit*)lb)->uc) return false;
        range += ((LexicalUcbit*)lb)->range;
        return true;
        }
      virtual bool test(const xblk& blk) const
        {
        if(!chk(blk)) return false;
        const unsigned char* lp = (const unsigned char*)blk.start();
        smtrace << "    匹配ucbit\r\n" << hex2show(blk.start(), blk.size(), HC_ASCII, true, 6);
        for(size_t i = 0; i < blk.size(); ++i)
          {
          if(!uc.test(lp[i])) return false;
          }
        return true;
        }
      virtual void to_atom(line& nline) const
        {
        nline << uc;
        }
    public:
      ucbit uc;
    };
  class LexicalConstHex : public LexicalBase
    {
    public:
      LexicalConstHex(const unsigned char h, const Range& range = gk_normal_range)
        :LexicalBase(LT_ConstHex, range), hex(h)
        {

        }
      virtual const char* type_name() const
        {
        return "词法consthex";
        }
      virtual bool fusion(LexicalBase* lb)
        {
        if(lb->type != LT_ConstHex) return false;
        if(hex != ((LexicalConstHex*)lb)->hex) return false;
        range += ((LexicalConstHex*)lb)->range;
        return true;
        }
      virtual bool test(const xblk& blk) const
        {
        if(!chk(blk)) return false;
        const char* lp = (const char*)blk.start();
        smtrace << "    匹配consthex : " << hex << "\r\n"
          << hex2show(blk.start(), blk.size(), HC_ASCII, true, 6);
        for(size_t i = 0; i < blk.size(); ++i)
          {
          if(lp[i] != hex) return false;
          }
        return true;
        }
      virtual void to_atom(line& nline) const
        {
        nline << hex;
        }
    public:
      unsigned char hex;
    };
  struct MarkRef
    {
    bool mark_right : 1;
    bool mark_left : 1;
    char ref_right : 4;
    char ref_left : 4;
    };
  static const MarkRef gk_none_mark_ref = { false, false, 0, 0 };
  static const MarkRef gk_err_mark_ref = { true, true, 1, 1 };
  static bool operator==(const MarkRef& mra, const MarkRef& mrb)
    {
    return (mra.ref_left == mrb.ref_left) &&
      (mra.mark_left == mrb.mark_left) &&
      (mra.ref_right == mrb.ref_right) &&
      (mra.mark_right == mrb.mark_right);
    }
  static bool operator!=(const MarkRef& mra, const MarkRef& mrb)
    {
    return !(mra == mrb);
    }
  class LexicalMarkRef : public LexicalBase
    {
    public:
      LexicalMarkRef(const ucbit& u, const MarkRef& mr)
        :LexicalBase(LT_MarkRef, gk_normal_range), uc(u), mark_ref(mr)
        {
        ;
        }
      virtual const char* type_name() const
        {
        return "词法markref";
        }
      virtual bool test(const xblk& blk) const
        {
        if(!chk(blk)) return false;
        const unsigned char* lp = (const unsigned char*)blk.start();
        smtrace << "    匹配markref\r\n" << hex2show(blk.start(), blk.size(), HC_ASCII, true, 6);
        for(size_t i = 0; i < blk.size(); ++i)
          {
          if(!uc.test(lp[i])) return false;
          }
        return true;
        }
      virtual void to_atom(line& nline) const
        {
        nline << uc << mark_ref;
        }
    public:
      ucbit     uc;
      MarkRef   mark_ref;
    };
  }

//////////词法hexhex、ucbit、consthex、markreg、refreg识别函数////////////
namespace signaturematcher
  {
  //! 识别hexhex，成功返回string包含一个字符，非词法为空，错误则包含不只一个字符
  static string match_hexhex(Sign& sig)
    {
    const char hhex = match_hex(sig);

    if(hhex < 0)
      {
      if(sig.ch() != ':')   return string();
      sig.step();
      char hex = sig.ch();
      if(hex == '\0')
        {
        smerr << sig << "\":\"后提前结束";
        return string("err");
        }
      sig.step();
      return string(1, hex);
      }

    match_ws(sig);

    const char lhex = match_hex(sig);

    if(lhex < 0)
      {
      smerr << sig << "hexhex配对失败";
      return string("err");
      }

    return string(1, hhex << 4 | lhex);
    }
  //! 识别markreg和refreg的后缀词法，返回gk_err_mark_ref、gk_none_mark_ref或结果
  static MarkRef match_markref(Sign& sig)
    {
    match_ws(sig);
    MarkRef mr = gk_none_mark_ref;
    while(sig.ch() != '\0')
      {
      switch(sig.ch())
        {
        case '@':
          {
          sig.step();
          match_ws(sig);
          switch(sig.ch())
            {
            case 'L':       case 'l':
              if(mr.mark_left == true)
                {
                smerr << sig << "重复标识左寄存器";
                return gk_err_mark_ref;
                }
              if(mr.ref_left != 0)
                {
                smerr << sig << "已引用左寄存器，不能继续标识，请指明其一";
                return gk_err_mark_ref;
                }
              mr.mark_left = true;
              smtrace << "  识别到mark_left";
              break;
            case 'R':       case 'r':
              if(mr.mark_right == true)
                {
                smerr << sig << "重复标识右寄存器";
                return gk_err_mark_ref;
                }
              if(mr.ref_right != 0)
                {
                smerr << sig << "已引用右寄存器，不能继续标识，请指明其一";
                return gk_err_mark_ref;
                }
              mr.mark_right = true;
              smtrace << "  识别到mark_right";
              break;
            default:
              smerr << sig << "未知的标记符：" << sig.ch() << "，请指定[LR]";
              return gk_err_mark_ref;
            }
          sig.step();
          }
          break;
        case '$':
          {
          sig.step();
          match_ws(sig);
          char hex = match_hex(sig);
          if(hex <= 0)
            {
            hex = 1;
            }
          match_ws(sig);
          switch(sig.ch())
            {
            case 'L':   case 'l':
              if(mr.mark_left == true)
                {
                smerr << sig << "已标识左寄存器，不能引用，请指明其一";
                return gk_err_mark_ref;
                }
              if(mr.ref_left != 0)
                {
                smerr << sig << "已引用左寄存器";
                return gk_err_mark_ref;
                }
              mr.ref_left = hex;
              smtrace << "  识别到ref_left:" << (unsigned char)hex;
              break;
            case 'R':   case 'r':
              if(mr.mark_right == true)
                {
                smerr << sig << "已标识右寄存器，不能引用，请指明其一";
                return gk_err_mark_ref;
                }
              if(mr.ref_right != 0)
                {
                smerr << sig << "已引用右寄存器";
                return gk_err_mark_ref;
                }
              mr.ref_right = hex;
              smtrace << "  识别到ref_right:" << (unsigned char)hex;
              break;
            default:
              smerr << sig << "未知的引用符或索引：" << sig.ch() << "，请指定[1-F][LR]";
              return gk_err_mark_ref;
            }
          sig.step();
          }
          break;
        default:  return mr;
        }
      }
    return mr;
    }
  /*!
    优化识别结果
    当非模糊匹配时，如果范围固定，转化为LT_String，否则转化为LT_ConstHex
    当模糊匹配全部时，转化为LT_Dot，否则为LT_Ucbit
  */
  static LexicalBase* ucbit_optimization(const ucbit& uc, const Range& range)
    {
    if(uc.count() == 1)
      {
      size_t i = 0;
      for(; i < uc.size(); ++i)
        {
        if(uc.test(i))  break;
        }
      if(range.N == range.M)
        {
        smtrace << "非模糊匹配且范围固定，转换为string";
        return new LexicalString(string(range.N, (char)i));
        }
      smtrace << "非模糊匹配，但范围不定，转换为consthex";
      return new LexicalConstHex((char)i, range);
      }
    if(uc.count() == uc.size())
      {
      smtrace << "模糊匹配全部，优化为dot词法";
      return new LexicalDot(range);
      }
    smtrace << "无法优化，返回词法ucbit";
    return new LexicalUcbit(uc, range);
    }
  //! 识别ucbit、consthex、markreg、refreg词法，返回结果参考ucbit_optimization
  static LexicalBase* match_hexhexs(Sign& sig)
    {
    string ls(match_hexhex(sig));

    if(ls.empty())  return nullptr;
    if(ls.size() != 1) return new LexicalBase(LT_Error);

    unsigned char lhex = *(ls.c_str());
    smtrace << "识别到hexhex左值：" << (void*)lhex;

    const MarkRef mr = match_markref(sig);
    if(mr == gk_err_mark_ref) return new LexicalBase(LT_Error);

    ucbit uc;

    if(mr != gk_none_mark_ref)
      {
      smtrace << "  识别到markref： mark_left" << mr.mark_left
        << " mark_right" << mr.mark_right
        << " ref_left:" << (unsigned char)mr.ref_left
        << " ref_right:" << (unsigned char)mr.ref_right;
      unsigned char ahex = lhex;
      unsigned char bhex = lhex;
      if((mr.mark_right == true) || (mr.ref_right != 0))
        {
        ahex &= 0xF8;
        bhex |= 0x07;
        }
      if((mr.mark_left == true) || (mr.ref_left != 0))
        {
        ahex &= 0xC7;
        bhex |= 0x38;
        }
      smtrace << "  处理mrakref： " << (void*)ahex << " & " << (void*)bhex;
      uc.set_obfus(ahex, bhex);
      return new LexicalMarkRef(uc, mr);
      }

    //注意不直接使用while，以避免遇到结尾0，而前值未处理的情况
    do
      {
      match_ws(sig);
      char oper = sig.ch();
      switch(oper)
        {
        case '&':     case '|':    case '-':
          sig.step(); break;
        default:      oper = 0;
        }
      //无连接符则退出
      if(oper == 0)
        {
        uc.set(lhex);
        break;
        }

      match_ws(sig);
      ls = match_hexhex(sig);
      if(ls.empty())
        {
        //处理&L&R的情况
        if(oper == '&')
          {
          switch(sig.ch())
            {
            case 'L':     case 'l':
              uc.set_obfus(lhex & 0xC7, lhex | 0x38);
              sig.step();
              break;
            case 'R':     case 'r':
              uc.set_obfus(lhex & 0xF8, lhex | 0x07);
              sig.step();
              break;
            default:
              smerr << sig << "操作符'&'后需要一个完整hexhex或[LR]标记";
              return new LexicalBase(LT_Error);
            }
          continue;
          }
        else
          {
          smerr << sig << "操作符'" << oper << "'后需要一个完整hexhex";
          return new LexicalBase(LT_Error);
          }
        }
      if(ls.size() != 1) return new LexicalBase(LT_Error);

      const unsigned char rhex = *(ls.c_str());
      smtrace << "  识别到hexhex右值" << oper << (void*)rhex;

      switch(oper)
        {
        case '&':   uc.set_obfus(lhex, rhex);     break;
        case '|':   uc.set(lhex);  uc.set(rhex);  break;
        case '-':   uc.set_queue(lhex, rhex);     break;
        default:;
        }

      lhex = rhex;
      } while(sig.ch() != '\0');

    const Range range = match_range(sig);
    if(range == gk_err_range)  return new LexicalBase(LT_Error);

    return ucbit_optimization(uc, range);
    }
  }

////////////////////////词法collection识别函数////////////////////////////
namespace signaturematcher
  {
  //! 识别collection词法，返回结果参考ucbit_optimization
  static LexicalBase* match_collection(Sign& sig)
    {
    if(sig.ch() != '[') return nullptr;
    sig.step();
    match_ws(sig);
    smtrace << "识别到词法collection";

    //识别可能存在的翻转请求
    const bool reversal = (sig.ch() == '^');
    if(reversal)         
      {
      sig.step();
      match_ws(sig);
      smtrace << "  识别到集合翻转指示^";
      }

    unsigned char lhex;
    ucbit uc;
    while(sig.ch() != ']' || sig.ch() != '\0')
      {
      match_ws(sig);
      string ls(match_hexhex(sig));

      if(ls.empty())  break;
      if(ls.size() != 1) return new LexicalBase(LT_Error);
      lhex = *(ls.c_str());
      smtrace << "  识别到hexhex左值：" << (void*)lhex;

      char oper = sig.ch();
      switch(oper)
        {
        case '&':     case '|':    case '-':
          sig.step(); break;
        default:      oper = 0;
        }
      if(oper == 0)
        {
        uc.set(lhex);
        continue;
        }

      match_ws(sig);
      ls = match_hexhex(sig);
      if(ls.empty())
        {
        smerr << sig << "操作符'" << oper << "'后需要一个完整hexhex";
        return new LexicalBase(LT_Error);
        }
      if(ls.size() != 1) return new LexicalBase(LT_Error);

      const unsigned char rhex = *(ls.c_str());
      smtrace << "  识别到hexhex右值" << oper << (void*)rhex;

      switch(oper)
        {
        case '&':   uc.set_obfus(lhex, rhex);     break;
        case '|':   uc.set(lhex);  uc.set(rhex);  break;
        case '-':   uc.set_queue(lhex, rhex);     break;
        default:;
        }
      lhex = rhex;
      }

    match_ws(sig);
    if(sig.ch() != ']')
      {
      smerr << sig << "集合需要]符以标识结束";
      }
    sig.step();

    if(reversal)  uc.flip();

    if(uc.none())
      {
      smerr << sig << "集合不允许空匹配，请检查";
      return new LexicalBase(LT_Error);
      }

    smtrace << "词法collection成功读取";

    const Range range = match_range(sig);
    if(range == gk_err_range)  return new LexicalBase(LT_Error);

    return ucbit_optimization(uc, range);
    }
  }

////////////////////////词法record类及识别函数////////////////////////////
namespace signaturematcher
  {
  class LexicalRecord : public LexicalBase
    {
    public:
      LexicalRecord(const LexicalType type, const Range& r, const string& n, const string& o,const bool offset)
        :LexicalBase(type, r), name(n), note(o), isoff(offset)
        {

        }
      virtual bool record() const
        {
        return true;
        }
      virtual REPORT_VALUE pick_value(void* start) const = 0;
      virtual void to_atom(line& nline) const
        {
        nline << isoff;
        nline << name.size();
        nline.append((const unsigned char*)name.c_str(), name.size());
        nline << note.size();
        nline.append((const unsigned char*)note.c_str(), note.size());
        }
    public:
      string name;
      string note;
      bool   isoff;
    };
#pragma warning(push)
#pragma warning(disable:4244)  //C4244: 可能丢失数据
  class LexicalRecordAddr : public LexicalRecord
    {
    public:
      LexicalRecordAddr(const string& n, const string& o, const bool offset)
        :LexicalRecord(LT_Record_Addr, gk_none_range, n, o, offset)
        {

        }
      virtual const char* type_name() const
        {
        return "词法record_addr";
        }
      virtual REPORT_VALUE pick_value(void* start) const
        {
        REPORT_VALUE rv;
        rv.t = 'p';
        rv.q = 0;
        rv.p = match_mem;
        if(isoff) rv.p = (void*)((size_t)rv.p - (size_t)start);
        return rv;
        }
    };
  class LexicalRecordOffset : public LexicalRecord
    {
    public:
      LexicalRecordOffset(const string& n, const string& o, const bool offset)
        :LexicalRecord(LT_Record_Offset, Range(sizeof(unsigned __int32)), n, o, offset)
        {

        }
      virtual const char* type_name() const
        {
        return "词法record_offset";
        }
      virtual REPORT_VALUE pick_value(void* start) const
        {
        REPORT_VALUE rv;
        const __int32 off = *(__int32*)match_mem;
        rv.t = 'p';
        rv.q = 0;
        rv.p = (void*)((unsigned char*)match_mem + off + sizeof(off));
        if(isoff) rv.p = (void*)((size_t)rv.p - (size_t)start);
        return rv;
        }
    };
  class LexicalRecordQword : public LexicalRecord
    {
    public:
      LexicalRecordQword(const string& n, const string& o, const bool offset)
        :LexicalRecord(LT_Record_Qword, Range(sizeof(unsigned __int64)), n, o, offset)
        {

        }
      virtual const char* type_name() const
        {
        return "词法record_qword";
        }
      virtual REPORT_VALUE pick_value(void* start) const
        {
        REPORT_VALUE rv;
        rv.t = 'q';
        rv.q = *(unsigned __int64*)match_mem;
        if(isoff) rv.q = rv.q - (unsigned __int64)start;
        return rv;
        }
    };
  class LexicalRecordDword : public LexicalRecord
    {
    public:
      LexicalRecordDword(const string& n, const string& o, const bool offset)
        :LexicalRecord(LT_Record_Dword, Range(sizeof(unsigned __int32)), n, o, offset)
        {

        }
      virtual const char* type_name() const
        {
        return "词法record_dword";
        }
      virtual REPORT_VALUE pick_value(void* start) const
        {
        REPORT_VALUE rv;
        rv.t = 'd';
        rv.q = 0;
        rv.d = *(unsigned __int32*)match_mem;
        if(isoff) rv.d = rv.d - (unsigned __int32)start;
        return rv;
        }
    };
  class LexicalRecordWord : public LexicalRecord
    {
    public:
      LexicalRecordWord(const string& n, const string& o, const bool offset)
        :LexicalRecord(LT_Record_Word, Range(sizeof(unsigned __int16)), n, o, offset)
        {

        }
      virtual const char* type_name() const
        {
        return "词法record_word";
        }
      virtual REPORT_VALUE pick_value(void* start) const
        {
        REPORT_VALUE rv;
        rv.t = 'w';
        rv.q = 0;
        rv.w = *(unsigned __int16*)match_mem;
        if(isoff) rv.w = rv.w - (unsigned __int32)start;
        return rv;
        }
    };
  class LexicalRecordByte : public LexicalRecord
    {
    public:
      LexicalRecordByte(const string& n, const string& o, const bool offset)
        :LexicalRecord(LT_Record_Byte, Range(sizeof(unsigned __int8)), n, o, offset)
        {

        }
      virtual const char* type_name() const
        {
        return "词法record_byte";
        }
      virtual REPORT_VALUE pick_value(void* start) const
        {
        REPORT_VALUE rv;
        rv.t = 'b';
        rv.q = 0;
        rv.b = *(unsigned __int8*)match_mem;
        if(isoff) rv.b = rv.b - (unsigned __int8)start;
        return rv;
        }
    };
#pragma warning(pop)
  //! 识别record词法
  static LexicalBase* match_record(Sign& sig)
    {
    if(sig.ch() != '<') return nullptr;
    sig.step();
    match_ws(sig);
    bool offset = false;
    if(sig.ch() == '^')
      {
      offset = true;
      sig.step();
      match_ws(sig);
      }
    LexicalRecord* lr = nullptr;
    const string nullstr;
    switch(sig.ch())
      {
      case 'A':       case 'a':
        lr = new LexicalRecordAddr(nullstr, nullstr, offset);  break;
      case 'F':       case 'f':
        lr = new LexicalRecordOffset(nullstr, nullstr, offset);  break;
      case 'Q':       case 'q':
        lr = new LexicalRecordQword(nullstr, nullstr, offset);  break;
      case 'D':       case 'd':
        lr = new LexicalRecordDword(nullstr, nullstr, offset);  break;
      case 'W':       case 'w':
        lr = new LexicalRecordWord(nullstr, nullstr, offset);  break;
      case 'B':       case 'b':
        lr = new LexicalRecordByte(nullstr, nullstr, offset);  break;
      default:
        smerr << sig << "特征点请指定[AFQDWB]";
        return new LexicalBase(LT_Error);
      }
    sig.step();

    match_ws(sig);

    string s[2];
    for(size_t i = 0; i < 2;)
      {
      const char ch = sig.ch();
      if(ch == '\0')
        {
        smerr << sig << "词法record需要用'>'来标识结束";
        return new LexicalBase(LT_Error);
        }
      if(ch == '>')
        {
        sig.step();
        break;
        }
      switch(ch)
        {
        case '\r':        break;
        case '\n':
          if(i == 0)
            {
            ++i;
            }
          else
            {
            s[i].append(1, ' ');
            }
          break;
        case ' ':     case '\t':
          if(i == 0)
            {
            ++i;
            break;
            }
          //故意没有break
        default:
          s[i].append(1, ch);
        }
      sig.step();
      }
    
    //消除注释前后空白，由于前面消除了"\r\n"的输入，所以只需要处理" \t"
    const char trimch[] = " \t "; //后面再加空格为了处理先制表再空格的情况
    for(auto ch : trimch)
      {
      if(s[1].empty())  break;
      string::size_type pos = s[1].find_last_not_of(ch);
      if(pos != string::npos)
        {
        s[1].erase(pos + 1);
        pos = s[1].find_first_not_of(ch);
        if(pos != string::npos) s[1].erase(0, pos);
        }
      }

    smtrace << "识别到词法record，" << "偏移" << offset << "："
      << lr->type << ':' << s[0] << " //" << s[1];
    lr->name = s[0];
    lr->note = s[1];
    return lr;
    }
  }

//////////////////////////////词法识别////////////////////////////////////
namespace signaturematcher
  {
  //! 词法识别函数格式
  typedef LexicalBase* (*LexicalMatcher)(Sign& sig);
  //! 词法识别函数组
  static const LexicalMatcher gk_lexical_matcher[] = {
    &match_ws,
    &match_note,    //注意，note词法应放在其它词法之前
    &match_dot,
    &match_hexhexs,
    &match_record,
    &match_collection,
    &match_string_quote,
    &match_end,     //注意，end词法应放在最后
    };

  vector<LexicalBase*> create_lexer(SIGNATURE signature)
    {
    Sign sig(signature);
    vector<LexicalBase*> vec;
    size_t matcher_lp = 0;
    smtrace << "-----------------------词法分析开始-----------------------";
    for(; matcher_lp < _countof(gk_lexical_matcher); ++matcher_lp)
      {
      LexicalBase* lb = gk_lexical_matcher[matcher_lp](sig);

      //非此词法转下条
      if(lb == nullptr) continue;

      smtrace << "词法生成：" << lb->type << '-' << lb->type_name();

      //非法词法，停止
      if(!lb->valid())
        {
        delete lb;
        break;
        }


      if(lb->type == LT_End)
        {
        delete lb;

        smtrace << "-----------------------词法分析结束-----------------------";

        if(vec.empty()) return vec;

        //判定至少要存在一个record，否则添加
        bool need_add = true;

        for(auto lb : vec)
          {
          if(lb->record())
            {
            need_add = false;
            break;
            }
          }
        if(need_add)
          {
          smtrace << "由于没有特征点，头部自动添加";
          const string nullstr;
          vec.insert(vec.begin(), new LexicalRecordAddr(nullstr, nullstr, false));
          }

        return vec;
        }

      //对引用索引进行校验
      if(lb->type == LT_MarkRef)
        {
        //先计算以前的标记个数
        size_t mark_count = 0;
        vec.push_back(lb);
        for(auto olb : vec)
          {
          if(olb->type != LT_MarkRef) continue;
          LexicalMarkRef* lmr = (LexicalMarkRef*)olb;
          if(lmr->mark_ref.mark_left)   ++mark_count;
          if(lmr->mark_ref.mark_right)  ++mark_count;
          }
        vec.pop_back();
        smtrace << "可能存在索引，之前标记个数：" << mark_count;
        //判定索引位是否超过标记个数
        LexicalMarkRef* lmr = (LexicalMarkRef*)lb;
        if((size_t)(lmr->mark_ref.ref_left) > mark_count ||
           (size_t)(lmr->mark_ref.ref_right) > mark_count)
          {
          smerr << sig << "索引位置无法确定，请检查索引";
          break;
          }
        smtrace << "索引检查通过";
        }

      //词法非空时，考虑同类型融合
      if(!vec.empty())
        {
        LexicalBase* olb = *(vec.rbegin());
        //尝试融合，整合成功则释放资源不加入vec
        if(olb->fusion(lb))
          {
          smtrace << "   成功融合\r\n\r\n";
          delete lb;
          lb = nullptr;
          }
        }

      if(lb != nullptr)
        {
        vec.push_back(lb);
        smtrace << "    加入类型：" << lb->type << ' '
          << lb->type_name() << "\r\n\r\n";
        }

      //词法成功识别，则从头开始
      matcher_lp = 0;
      --matcher_lp;
      }

    if(matcher_lp >= _countof(gk_lexical_matcher))
      smerr << sig << "无法识别的词法";

    smtrace << "-----------------------词法分析失败-----------------------";
    //词法识别失败，释放资源
    for(auto lb : vec)
      {
      delete lb;
      }
    vec.clear();
    return vec;
    }
  }

////////////////////////原子生成、原子识别////////////////////////////////
namespace signaturematcher
  {
  line create_atom(SIGNATURE signature)
    {
    vector<LexicalBase*> vec(create_lexer(signature));
    if(vec.empty()) return line();
    line nline;
    for(auto lb : vec)
      {
      lb->create_atom(nline);
      delete lb;
      }
    nline << LT_End << gk_err_range;
    return nline;
    }
  //! 指定原子，生成词法组
  static vector<LexicalBase*> LexicalAtom(line& atom)
    {
    vector<LexicalBase*> vec;
    bool end = false;
    while(!atom.empty() && !end)
      {
      LexicalType type;
      atom >> type;
      Range range(gk_err_range);
      atom >> range;
      switch(type)
        {
        case LT_End:
          smtrace << "识别到词法end";
          end = true;
          break;
        case LT_String:
          {
          size_t size;
          atom >> size;
          const string str((const char*)atom.c_str(), size);
          atom.erase(0, size);
          smtrace << "识别到词法string\r\n" << hex2show(str, (size_t)2);
          vec.push_back(new LexicalString(str));
          }
          break;
        case LT_Quote:
          {
          size_t size;
          atom >> size;
          const string str((const char*)atom.c_str(), size);
          atom.erase(0, size);
          smtrace << "识别到词法quote\r\n" << hex2show(str, (size_t)2);
          vec.push_back(new LexicalQuote(str));
          }
          break;
        case LT_Dot:
          smtrace << "识别到词法dot：{" << (void*)range.N << ',' << (void*)range.M;
          vec.push_back(new LexicalDot(range));
          break;
        case LT_MarkRef:
          {
          ucbit uc;
          MarkRef mr;
          atom >> uc >> mr;
          smtrace << "识别到词法markref： mark_left" << mr.mark_left
            << " mark_right" << mr.mark_right
            << " ref_left:" << (void*)mr.ref_left
            << " ref_right:" << (void*)mr.mark_right;
          vec.push_back(new LexicalMarkRef(uc, mr));
          }
          break;
        case LT_Ucbit:
          {
          ucbit uc;
          atom >> uc;
          smtrace << "识别到词法ucbit：{" << (void*)range.N << ',' << (void*)range.M << '}';
          vec.push_back(new LexicalUcbit(uc, range));
          }
          break;
        case LT_ConstHex:
          {
          unsigned char hex;
          atom >> hex;
          smtrace << "识别到词法consthex"<< (void*)hex
            << "{" << (void*)range.N << ',' << (void*)range.M << '}';
          vec.push_back(new LexicalConstHex(hex, range));
          }
          break;
        case LT_Record_Addr:
        case LT_Record_Offset:
        case LT_Record_Qword:
        case LT_Record_Dword:
        case LT_Record_Word:
        case LT_Record_Byte:
          {
          bool offset;
          atom >> offset;
          size_t size;
          atom >> size;
          const string name((const char*)atom.c_str(), size);
          atom.erase(0, size);
          atom >> size;
          const string note((const char*)atom.c_str(), size);
          atom.erase(0, size);
          LexicalBase* lb = nullptr;
          switch(type)
            {
            case LT_Record_Addr:
              lb = new LexicalRecordAddr(name, note, offset);
              break;
            case LT_Record_Offset:
              lb = new LexicalRecordOffset(name, note, offset);
              break;
            case LT_Record_Qword:
              lb = new LexicalRecordQword(name, note, offset);
              break;
            case LT_Record_Dword:
              lb = new LexicalRecordDword(name, note, offset);
              break;
            case LT_Record_Word:
              lb = new LexicalRecordWord(name, note, offset);
              break;
            case LT_Record_Byte:
              lb = new LexicalRecordByte(name, note, offset);
              break;
            default:;
            }
          smtrace << "识别到"
            << (offset ? " ^" : " ")
            << lb->type_name() << "  " << name << "  //" << note;
          vec.push_back(lb);
          }
          break;
        default:
          {
          smerr << "未知类型：" << type << "，无法继续从原子生成词法";
          for(auto lb : vec)
            {
            delete lb;
            }
          vec.clear();
          return vec;
          }
        }
      }
    return vec;
    }
  }

//////////////////////////特征码匹配后继校验//////////////////////////////
namespace signaturematcher
  {
  //! 校验引用
  static bool match_check_ref(vector<LexicalBase*>::const_iterator itt,
                              intptr_t refs,
                              const unsigned char ch_ref)
    {
    void* srclp = (*itt)->match_mem;
    for(; refs > 0; --itt)
      {
      LexicalBase* lb_ref = *itt;
      if(lb_ref->type != LT_MarkRef)  continue;
      LexicalMarkRef* lmr_ref = (LexicalMarkRef*)lb_ref;

      if(lmr_ref->mark_ref.mark_right)
        {
        --refs;
        if(refs == 0)
          {
          const unsigned char ch = *(unsigned char*)lmr_ref->match_mem;
          smtrace << "  找到了ref，是@R " << lmr_ref->match_mem << ':' << ch;
          if(ch_ref != (ch & 0x07))
            {
            smerr << "原值" << srclp << "：" << ch_ref
              << " @R引用值" << lb_ref->match_mem << "："
              << (unsigned char)(ch & 0x07)
              << "无法匹配";
            return false;
            }
          break;
          }
        smtrace << "  跳过一个@R";
        }

      if(lmr_ref->mark_ref.mark_left)
        {
        --refs;
        if(refs == 0)
          {
          const unsigned char ch = *(unsigned char*)lmr_ref->match_mem;
          smtrace << "  找到了ref，是@L:" << lmr_ref->match_mem << ':' << ch;
          if(ch_ref != ((ch & 0x38) >> 3))
            {
            smerr << "原值" << srclp << "：" << (void*)ch_ref
              << " @L引用值" << lb_ref->match_mem << "："
              << (unsigned char)((ch & 0x38) >> 3)
              << "无法匹配";
            return false;
            }
          break;
          }
        smtrace << "  跳过一个@L";
        }
      }
    smtrace << "匹配，通过";
    return true;
    }
  //! 总校验
  static bool match_check(const vector<LexicalBase*>& vec)
    {
    XLIB_TRY
      {
      smtrace << "-----------正在校验-----------";
      //开始引用校验
      for(auto lb : vec)
        {
        if(lb->type != LT_Quote)  continue;
        //提取引用串地址
        void* quote = *(void**)lb->match_mem;
        LexicalQuote* lq = (LexicalQuote*)lb;
        smtrace << "校验引用串："
          << hex2show(lq->str, (size_t)2)
          << hex2show(string((const char*)quote, lq->str.size()), (size_t)2);
        if(memcmp(quote, lq->str.c_str(), lq->str.size()) != 0)
          {
          smtrace << "引用校验" << lb->match_mem << "：" << quote << "无法通过";
          return false;
          }
        }

      //开始refreg校验
      for(vector<LexicalBase*>::const_iterator it = vec.begin(); it != vec.end(); ++it)
        {
        LexicalBase* lb = *it;
        if(lb->type != LT_MarkRef)  continue;

        LexicalMarkRef* lmr = (LexicalMarkRef*)lb;
        vector<LexicalBase*>::const_iterator itt = it;

        const unsigned char ch_ref = *(unsigned char*)lmr->match_mem;
        //检查是否存在左引用
        if(lmr->mark_ref.ref_left != 0)
          {
          const intptr_t refs = lmr->mark_ref.ref_left;
          smtrace << "发现$" << refs << "L  :" << lmr->match_mem << ':' << ch_ref;

          if(!match_check_ref(itt, refs, ((ch_ref & 0x38) >> 3))) return false;
          }
        //检查是否存在右引用
        if(lmr->mark_ref.ref_right != 0)
          {
          const intptr_t refs = lmr->mark_ref.ref_right;
          smtrace << "发现$" << refs << "R  :" << lmr->match_mem << ':' << ch_ref;

          if(!match_check_ref(itt, refs, ch_ref & 0x07)) return false;
          }
        }
      smtrace << "-----------校验完成-----------";
      return true;
      }
    XLIB_CATCH
      {
      smtrace << "-----------校验异常-----------";
      }
    return false;
    }
  }

//////////////////////////特征点数据收集相关//////////////////////////////
namespace signaturematcher
  {
  bool report_add(REPORT& report, string name, const REPORT_NODE& node)
    {
    if(!name.empty())
      {
      REPORT::const_iterator it = report.find(name);
      if(it != report.end())
        {
        const REPORT_NODE& crn = (*it).second;
        if(crn.values.t != node.values.t)
          {
          smerr << "特征点\"" << name << "\"类型不符，原值:"
            << crn.values.t << "，现值:" << node.values.t;
          return false;
          }
        switch(crn.values.t)
          {
          case 'd':
            if(crn.values.d != node.values.d)
              {
              smerr << "特征点\"" << name << "\"值不符，原值:"
                << crn.values.d << "，现值:" << node.values.d;
              return false;
              }
            break;
          case 'w':
            if(crn.values.w != node.values.w)
              {
              smerr << "特征点\"" << name << "\"值不符，原值:"
                << crn.values.w << "，现值:" << node.values.w;
              return false;
              }
            break;
          case 'b':
            if(crn.values.b != node.values.b)
              {
              smerr << "特征点\"" << name << "\"值不符，原值:"
                << crn.values.b << "，现值:" << node.values.b;
              return false;
              }
            break;
          case 'p':
            if(crn.values.p != node.values.p)
              {
              smerr << "特征点\"" << name << "\"值不符，原值:"
                << crn.values.p << "，现值:" << node.values.p;
              return false;
              }
            break;
          default:
            if(crn.values.q != node.values.q)
              {
              smerr << "特征点\"" << name << "\"值不符，原值:"
                << crn.values.q << "，现值:" << node.values.q;
              return false;
              }
            break;
          }
        smtrace << "特征点\"" << name << "\"重复，略过";
        return true;
        }
      }
    else
      {
      xmsg tmpname;
      tmpname << "noname" << report.size();
      name = tmpname;
      }
    report[name] = node;
    smtrace << "新添特征点：" << node.values.q << ':' << name << "  //" << node.note;
    return true;
    }
  //! 提取特征点数据及相关信息
  static bool fix_report(void* start, const vector<LexicalBase*>& vec, REPORT& report)
    {
    smtrace << "-----------正在提取-----------";
    for(auto lb : vec)
      {
      if(!lb->record()) continue;
      LexicalRecord* lr = (LexicalRecord*)lb;
      
      const REPORT_NODE rn = { lr->pick_value(start), lr->note };

      if(!report_add(report, lr->name, rn)) return false;;
      }
    smtrace << "-----------提取完成-----------";
    return true;
    }
  }

////////////////////////////特征码匹配内核////////////////////////////////
namespace signaturematcher
  {
  //! 词法匹配内核
  static bool lexical_match(const xblk& blk,
                            const vector<LexicalBase*>& vec,
                            intptr_t lp)
    {
    XLIB_TRY
      {
      for(vector<LexicalBase*>::const_iterator it = vec.begin(); it != vec.end();)
        {
        LexicalBase* lb = *it;

        if(lb->match(blk, lp))                  //匹配成功，继续
          {
          ++it;
          continue;
          }

        if(lp > (intptr_t)blk.size())  break;  //匹配彻底失败，跳出

        if(it == vec.begin())  break;           //无法回退，跳出

        --it;                                   //正常回退
        }
      if(lp > (intptr_t)blk.size())
        {
        smtrace << "-------------------------------匹配失败-------------------------------";
        return false;
        }
      return true;
      }
    XLIB_CATCH
      {
      smerr << "-------------------------------匹配异常-------------------------------";
      }
    return false;
    }
  //! 校验与提取
  static bool fix_match(void* start, const vector<LexicalBase*>& vec, REPORT& report)
    {
    if(!match_check(vec))
      {
      smtrace << "-----------校验失败-----------";
      return false;
      }

    XLIB_TRY
      {
      if(!fix_report(start, vec, report))
        {
        smtrace << "-----------提取失败-----------";
        return false;
        }
      }
    XLIB_CATCH
      {
      smerr << "-----------提取异常-----------";
      return false;
      }
    return true;
    }

  REPORT match(const xblk& blk, const vector<LexicalBase*>& vec)
    {
    if(vec.empty()) return REPORT();

    smtrace << "-------------------------------开始匹配------------------------------- "
      << blk.start();
    smtrace << "词法个数：" << vec.size();
    intptr_t lp = 0;
    REPORT report;
    for(; lp < (intptr_t)blk.size(); ++lp)
      {
      if(!lexical_match(blk, vec, lp))  return REPORT();

      const unsigned char* mem = (const unsigned char*)blk.start() + lp;

      //检验匹配成功与否的标准是判定最后一个结点是否匹配完成
      if((*(vec.rbegin()))->match_count < 0)
        {
        smtrace << "-------------------------------匹配递进------------------------------- "
          << (void*)(mem + 1);
        continue;
        }

      if(!fix_match(blk.start(), vec, report))
        {
        for(auto lb : vec)
          {
          lb->match_count = -1;
          }
        continue;
        }

      smtrace << "-------------------------------匹配成功------------------------------- "
        << (void*)mem;
      break;
      }

    return report;
    }
  }

////////////////////////////特征码匹配重载////////////////////////////////
namespace signaturematcher
  {

  REPORT match(void* start, void* end, SIGNATURE signature)
    {
    vector<LexicalBase*> vec(create_lexer(signature));
    if(vec.empty())   return REPORT();
    const xblk blk(start, end);

    REPORT report(match(blk, vec));

    for(auto lb : vec)
      {
      delete lb;
      }

    return report;
    }

  REPORT match(void* start, void* end, line& atom)
    {
    vector<LexicalBase*> vec(LexicalAtom(atom));
    if(vec.empty())   return REPORT();
    const xblk blk(start, end);

    REPORT report(match(blk, vec));

    for(auto lb : vec)
      {
      delete lb;
      }

    return report;
    }

  REPORT match(const std::vector<xblk>& blks, SIGNATURE signature)
    {
    REPORT report;
    vector<LexicalBase*> vec(create_lexer(signature));
    if(vec.empty())   return report;
    for(auto blk : blks)
      {
      report = match(blk, vec);
      if(!report.empty())  break;
      }

    for(auto lb : vec)
      {
      delete lb;
      }
    return report;
    }

  REPORT match(const std::vector<xblk>& blks, line& atom)
    {
    REPORT report;
    vector<LexicalBase*> vec(LexicalAtom(atom));
    if(vec.empty())   return report;
    for(auto blk : blks)
      {
      report = match(blk, vec);
      if(!report.empty())  break;
      }

    for(auto lb : vec)
      {
      delete lb;
      }
    return report;
    }

  REPORT_VALUE find(void* start, void* end, SIGNATURE signature)
    {
    REPORT report(match(start, end, signature));
    if(report.empty())
      {
      REPORT_VALUE rv;
      rv.t = 'n';
      rv.q = 0;
      return rv;
      }
    return (*report.begin()).second.values;
    }
  }

///////////////////////////调试信息输出设置///////////////////////////////
namespace signaturematcher
  {
  xlog::levels& log_level()
    {
    return smlog::log_level;
    }

  void set_log_out(log_out_func func)
    {
    smlog::func = func;
    }
  }

////////////////////////////特征码匹配脚本////////////////////////////////
namespace signaturematcher
  {
  //! 给定脚本，提取特征码串
  static line pick_sign(line& script)
    {
    if(script.empty())
      {
      smtrace << "给定脚本为空";
      return line();
      }

    line sign;

    //消除起始空白
    line::iterator it = script.begin();
    for(; it != script.end(); ++it)
      {
      const unsigned char ch = *it;
      if(ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
        {
        sign.push_back(*it);
        continue;
        }
      break;
      }
    script.erase(script.begin(), it);

    if(script.empty())
      {
      smtrace << "给定脚本无有效数据";
      return line();
      }

    //提取起始，可以有起始标记"/\r?\n"，如果没有起始标记，当前即为起始
    it = script.begin();
    if(*it == '/')
      {
      if(*(it + 1) == '\n')
        {
        script >> '/';
        script >> '\n';
        }
      else
        {
        if(*(it + 1) == '\r' && *(it + 2) == '\n')
          {
          script >> '/';
          script >> '\r';
          script >> '\n';
          }
        }
      //如果存在起始标记，清除之前保存的空白
      sign.clear();
      }

    if(script.empty())
      {
      smerr << "给定脚本只有起始标记，无有效数据";
      return line();
      }

    //查找结束标记，提取特征码
    it = script.begin();
    for(; it != script.end(); ++it)
      {
      sign.push_back(*it);

      if(*it == '/')
        {
        // '/'后继脚本结束，完成提取
        if((it + 1) == script.end())
          {
          sign.pop_back();
          script.clear();
          //smtrace << "脚本提取到特征码:\r\n" << sign;
          return sign;
          }
        // '/'后继'\n'情况，结束标记，完成提取
        if(*(it + 1) == '\n')
          {
          sign.pop_back();
          script.erase(script.begin(), it + 2);
          //smtrace << "脚本提取到一个特征码:\r\n" << sign;
          return sign;
          }
        if(*(it + 1) != '\r')  break;
        if((it + 2) == script.end())  break;
        // '/'后继"\r\n"情况，通过，定为起始
        if(*(it + 2) != '\n') break;
        sign.pop_back();
        script.erase(script.begin(), it + 3);
        //smtrace << "脚本提取到一个特征码:\r\n" << sign;
        return sign;
        }
      }

    //没有结束标记，全数作为特征码
    for(; it != script.end(); ++it)
      {
      sign.push_back(*it);
      }
    script.clear();
    //smtrace << "脚本提取到特征码:\r\n" << sign;
    return sign;
    }

  //! 特征码中间原子文件头标记
#ifdef _WIN64
  static const char gk_atom_sign[] = { "SAx64" };
#else
  static const char gk_atom_sign[] = { "SAx86" };
#endif

  //! 分离以使ring0下SEH正常
  static bool analy_atom_script(line& script, analy_script_routine asr, void* lparam)
    {

    script.erase(0, sizeof(gk_atom_sign)-1);

    smtrace << "给定的脚本是中间原子脚本";

    while(!script.empty())
      {
      ScriptNode sn;
      script >> sn.type;
      //中间原子的类型只有两种
      size_t size = 0;
      script >> size;

      if(sn.type == ST_Blk)
        {
        sn.sig.assign((const char*)script.c_str(), size);
        script.erase(0, size);
        smtrace << "中间原子脚本提取得到范围指示：" << sn.sig;
        if(!asr(sn, lparam)) return false;
        continue;
        }

      if(sn.type != ST_Atom)
        {
        smerr << "中间原子未知类型数据:" << sn.type;
        return false;
        }

      sn.atom.assign(script.c_str(), size);
      script.erase(0, size);
//      smtrace << "中间原子脚本提取得到ATOM：\r\n"
 //       << hex2show(sn.atom);
      if(!asr(sn, lparam)) return false;
      }

    return true;
    }

  static bool analy_script_script(line& script, analy_script_routine asr, void* lparam)
    {
    smtrace << "不是中间原子脚本，作为普通脚本处理";
    while(!script.empty())
      {
      line sign(pick_sign(script));
      if(sign.empty())
        {
        return script.empty();
        }

      bool isblk = false;

      for(auto ch : sign)
        {
        if(ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') continue;
        if(ch == '@' || ch == '$')
          {
          isblk = true;
          }
        break;
        }

      if(isblk) smtrace << "提取到的特征码是范围指示";

      ScriptNode sn;

      sn.type = isblk ? ST_Blk : ST_Sign;
      sn.sig.assign((const char*)sign.c_str(), sign.size());
      if(!asr(sn, lparam))  return false;
      }

    return true;
    }

  bool analy_script(line& script, analy_script_routine asr, void* lparam)
    {
    XLIB_TRY
      {
      if(script.empty())
        {
        smerr << "给定的脚本为空";
        return false;
        }

      //先判定是否中间原子
      const size_t atom_sign_len = sizeof(gk_atom_sign)-1;
      if(script.size() >= atom_sign_len)
        {
        if(memcmp(gk_atom_sign, script.c_str(), sizeof(gk_atom_sign)-1) == 0)
          {
          return analy_atom_script(script, asr, lparam);
          }
        }

      //作脚本处理
      return analy_script_script(script, asr, lparam);
      }
    XLIB_CATCH
      {
      ;
      }
    smerr << "脚本解析未知异常";
    return false;
    }

  static bool create_atom_routine(ScriptNode sn, void* lparam)
    {
    line& atoms = *(line*)lparam;

    if(sn.type == ST_Blk)
      {
      atoms << sn.type;
      atoms << sn.sig.size();
      atoms.append((const unsigned char*)sn.sig.c_str(), sn.sig.size());
      return true;
      }
    if(sn.type == ST_Sign)
      {
      sn.atom = create_atom(sn.sig.c_str());
      if(sn.atom.empty())
        {
        atoms.clear();
        return false;
        }
      atoms << ST_Atom;
      atoms << sn.atom.size();
      atoms.append(sn.atom);
      return true;
      }
    if(sn.type != ST_Atom)
      {
      smerr << "脚本建立中间原子未知类型:" << sn.type;
      }
    atoms.clear();
    return false;
    }

  line create_atom_by_script(line& script)
    {
    line atoms;
    atoms.append((const unsigned char*)gk_atom_sign, sizeof(gk_atom_sign) - 1);
    if(!analy_script(script, create_atom_routine, &atoms))
      {
      atoms.clear();
      }
    return atoms;
    }

  vector<xblk> analy_blk(string& blkstr)
    {
    vector<xblk> vec;
    //消除前缀空白
    for(string::iterator it = blkstr.begin(); it != blkstr.end(); ++it)
      {
      const char ch = *it;
      if(ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') continue;
      blkstr.erase(blkstr.begin(), it);
      break;
      }

    xblk tmpblk(nullptr, nullptr);

    while(!blkstr.empty())
      {
      const char flag = *blkstr.begin();
      if(flag != '@' && flag != '$')
        {
        smerr << "范围解析未知指示：" << flag;
        vec.clear();
        return vec;
        }

      blkstr.erase(blkstr.begin(), blkstr.begin() + 1);

      //提取内容
      string::iterator it = blkstr.begin();
      for(; it != blkstr.end(); ++it)
        {
        const char ch = *it;
        if(ch == '@' || ch == '$')
          {
          break;
          }
        }

      string blk(blkstr.begin(), it);
      blkstr.erase(blkstr.begin(), it);

      //消除前缀空白
      for(string::iterator it = blk.begin(); it != blk.end(); ++it)
        {
        const char ch = *it;
        if(ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') continue;
        blk.erase(blk.begin(), it);
        break;
        }

      //消除后缀空白
      for(string::reverse_iterator it = blk.rbegin(); it != blk.rend();/* ++it*/)
        {
        const char ch = *it;
        if(ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
          {
          blk.pop_back();
          it = blk.rbegin();
          continue;
          }
        break;
        }

      smtrace << "提取到范围指示：" << blk;

      if(tmpblk.start() == nullptr)
        {
        //尝试识别为模块
        smtrace << "尝试识别为模块...";
#ifndef FOR_RING0
        HMODULE hmod = GetModuleHandleA(blk.c_str());
        if(hmod != nullptr)
          {
          pe thispe(hmod);
          if(thispe.IsPE())
            {
            auto i = thispe.GetImage();
            auto c = thispe.GetCode();
            tmpblk = (flag == '@') ? i : xblk(i.start(), c.end());
            }
          }
#else // FOR_RING0
        SysDriverSnap sds;
        for(const SYSTEM_MODULE& st : sds)
          {
          const char* name = (const char*)(st.Name + st.NameOffset);
          if(_stricmp(blk.c_str(), name) == 0)
            {
            if(flag == '@')
              {
              tmpblk = xblk(st.ImageBaseAddress, st.ImageSize);
              }
            else
              {
              pe thispe((HMODULE)st.ImageBaseAddress);
              if(thispe.IsPE())
                {
                auto i = thispe.GetImage();
                auto c = thispe.GetCode();
                tmpblk = xblk(i.start(), c.end());
                }
              }
            }
          }
#endif  // FOR_RING0
        }
      if(tmpblk.size() != 0)
        {
        vec.push_back(tmpblk);
        smtrace << "提取得到范围：{ " << tmpblk.start() << " , "
          << tmpblk.end() << '}';
        tmpblk = xblk(nullptr, nullptr);
        continue;
        }

      //如果识别模块失败，识别为范围
      void* p = (void*)str2hex(blk);
      if(p == nullptr)
        {
        smerr << "无法识别的范围指示：" << blk;
        vec.clear();
        return vec;
        }

      if(tmpblk.start() == nullptr)
        {
        smtrace << "提取到范围起始:" << p;
        tmpblk = xblk(p, p);
        }

      tmpblk = xblk(tmpblk.start(), p);
      vec.push_back(tmpblk);
      smtrace << "提取到范围结束，此时：{" << tmpblk.start() << " , "
        << tmpblk.end() << '}';
      tmpblk = xblk(nullptr, nullptr);
      }

    if(tmpblk.start() != nullptr && tmpblk.size() == 0)
      {
      smerr << "范围指示没有正确配对";
      vec.clear();
      }

    return vec;
    }
  }

#pragma warning(pop)

#ifdef _XLIB_TEST_

#include "pe.h"
static const char* const signaturematchertest = "4210421042104210421042104210";

ADD_XLIB_TEST(SIGNATUREMATCHER)
  {
  SHOW_TEST_INIT;

  bool done = false;

  SHOW_TEST_HEAD("signaturematcher");
  pe my;
  const auto img(my.GetImage());
  auto rets = signaturematcher::find(img.start(), img.end(), "<A>'4210421042104210421042104210'");
  done = (signaturematchertest == rets.p);
  SHOW_TEST_RESULT(done);
  }

#endif  // _XLIB_TEST_

#endif  // _WIN32