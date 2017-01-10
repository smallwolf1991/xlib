﻿/*!
  \file  xlog.h
  \brief xlog.h定义了日志组织与输出相关的类

  \version    4.0.1611.1511
  \note       For All

  \author     triones
  \date       2011-07-22
*/
#ifndef _XLIB_XLOG_H_
#define _XLIB_XLOG_H_

#include "xmsg.h"

/*!
  xlog用于基本的调试信息输出。\n
  一般不直接使用，而是通过宏定义间接使用。\n
  加入两个全局变量用于动态等级、动态类型的输出控制。\n
  \b 注意 类本身没有输出控制，无论动态，静态控制都需要通过宏完成。(节省资源，加快运行)\n
  如果需要扩展功能，如输出到文件等，你可以选择继承之，或仿造xlog实现之。\n
  \b 注意 继承xlog将无条件输出到DebugView。推荐继承xlog而不是仿造之。\n
  宏的具体操作参见之后说明
*/
class xlog : public xmsg
  {
  public:
    /*!
    输出等级，动态、静态、类型控制皆可。大于设置等级的信息将不被输出\n
    默认动态、静态控制等级为lvl_on，即全输出，类型控制亦全输出\n
    - 静态控制，信息无组织，无二进制代码。\b 局部作用域
    - 动态控制，信息有组织，有二进制代码。\b 全局作用域
    - 类型控制，信息有组织，有二进制代码。\b 全局作用域
    */
    enum levels
      {
      lvl_off,    //!< 屏蔽输出。
      lvl_fatal,  //!< 致命错误，程序无法继续执行
      lvl_error,  //!< 反映错误，例如一些API的调用失败
      lvl_warn,   //!< 反映某些需要注意的可能有潜在危险的情况，可能会造成崩溃或逻辑错误之类
      lvl_info,   //!< 表示程序进程的信息
      lvl_debug,  //!< 普通的调试信息，这类信息发布时一般不输出
      lvl_trace,  //!< 最精细的调试信息，多用于定位错误，查看某些变量的值
      lvl_on,     //!< 全输出
      };
  public:
    virtual ~xlog();
    void out();
  public:
    //! 设置动态输出等级
    static levels set_level(levels new_level);
    //! 打开指定类型输出
    static size_t open_type(size_t be_set_type);
    //! 关闭指定类型输出
    static size_t close_type(size_t be_set_type);
    //! 获取动态输出等级
    static levels level();
    //! 获取类型输出控制信息
    static size_t type();
  private:
    static levels dynamic_lvl;
    static size_t dynamic_type;
  };

//! 用于控制即时输出
/*!

  \code
    xlog() << "第一行消息" << "紧接第一行消息";
    xlog() << "第一行消息" << xlogout << "第二行消息";
  \endcode
*/
xmsg& xlogout(xmsg& v);

//! 以下是三种类型的独立输出控制，如需混合控制，请自行书写相关宏。

/*!
  xlog_static_lvl宏用于控制静态编译结果。\n
  \b 注意 宏的作用是局部的，不同CPP可以设置不同的静态控制等级。\n
  由于需要在包含之前定义宏，故无法使用枚举值，请参考枚举值自定数值。\n
  如果需要设置全局静态控制等级，需要在“预处理器选项”中设置“xlog_static_lvl=[1..7]”，如此控制全局静态编译结果。

  \code
    #define xlog_static_lvl 1 //静态控制等级为lvl_fatal，只输出最严重的错误
    #include "xlog.h"
  \endcode
*/
#ifndef xlog_static_lvl
#define xlog_static_lvl xlog::lvl_on
#endif

/*!
  以下宏用于分级静态控制编译结果，根据xlog_static_lvl以决定指定调试信息是否被编译。
  也是最简单最常见的应用。

  \code
    xerr << "xerr";   //当xlog_static_lvl<xlog_base::lvl_error时，此句不被编译
    xfail << "xfail"; //除非xlog_static_lvl==0，否则此句输出。
  \endcode
*/
#ifdef _WIN32
#   pragma warning(disable:4127)  //C4127: 条件表达式是常量
#endif  // _WIN32
#define xlog_do(v) if((v) <= xlog_static_lvl) xlog()

#define xtrace  xlog_do(xlog::lvl_trace)
#define xdbg    xlog_do(xlog::lvl_debug)
#define xinfo   xlog_do(xlog::lvl_info)
#define xwarn   xlog_do(xlog::lvl_warn)
#define xerr    xlog_do(xlog::lvl_error)
#define xfail   xlog_do(xlog::lvl_fatal)

/*!
  以下宏用于动态控制信息输出。应用于需要在运行时动态控制信息的情况。

  \code
    xderr << "xerr";   //默认级别将输出
    xlog::setlevel(xlog::lvl_fatal);
    xderr << "xerr";   //此时信息将不输出
  \endcode
*/

#define xdlog_do(v) if((v) <= xlog::level()) xlog()

#define xdtrace  xdlog_do(xlog::lvl_trace)
#define xddbg    xdlog_do(xlog::lvl_debug)
#define xdinfo   xdlog_do(xlog::lvl_info)
#define xdwarn   xdlog_do(xlog::lvl_warn)
#define xderr    xdlog_do(xlog::lvl_error)
#define xdfail   xdlog_do(xlog::lvl_fatal)

/*!
  以下宏用于类型控制输出。\n
  注意类型控制是动态的，全局的。\n
  \b 另请特别注意，类型控制只支持32种类型，设置从0~31 (x64下支持64种类型) \n
  建议使用者在此基础上另加宏控制，以使代码直观。参见：

  \code
    #define xlog_type_pickup 1
    #define xlog_type_throw 2
    #define xlog_sell     xinfo_(0) //出售物品
    #define xlog_pickup   xinfo_(xlog_type_pickup) //拾取物品
    #define xlog_buy      xinfo_(0) //买进物品
    #define xlog_throw    xinfo_(xlog_type_throw) //丢弃物品

    xlog_sell << "出售物品：";
    xlog_buy << "买进物品"; //注意：xlog_sell与xlog_buy使用了同一序号，虽然宏名不同，但它们在内部被视作同一类型信息。所以，需要特别注意类型序号的冲突

    xlog::close_type(xlog_type_pickup);
    xlog_pickup << "拾取物品";
    xlog_throw << "丢弃物品"; //注意使用了不同序号，故xlog_pickup输出将被关闭。
  \endcode
*/
#define xtlog_do(v, n) if(((size_t)1 << n) & xlog::type()) xlog()

#define xttrace(n)  xtlog_do(xlog::lvl_trace, n)
#define xtdbg(n)    xtlog_do(xlog::lvl_debug, n)
#define xtinfo(n)   xtlog_do(xlog::lvl_info, n)
#define xtwarn(n)   xtlog_do(xlog::lvl_warn, n)
#define xterr(n)    xtlog_do(xlog::lvl_error, n)
#define xtfail(n)   xtlog_do(xlog::lvl_fatal, n)

/*!
  便捷宏，用于便捷插入函数名及行号

  \code
    xerr << xfuninfo << "这里出错";
  \endcode
*/
#define xfuninfo " [" << __FUNCTION__ << "][" << __LINE__ << "]: "
/*!
  便捷宏，用于便捷插入异常产生的函数

  \code
    xerr<<xfunexpt;
  \endcode
*/
#define xfunexpt " [" << __FUNCTION__ << "]: exception"

#endif  // _XLIB_XLOG_H_