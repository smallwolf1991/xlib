/*!
  \file  hook.h
  \brief hook.h用于执行指定hook、unhook操作

  - 当在RING0下，工程请关闭【C++异常】及【缓冲区检查】，否则将链接进VC运行时库导致驱动加载失败！
  - 由于链接器的【增量链接】可能会造成在使用Hook时出现一些莫名其妙的状况，请将之关闭！
  - Ring3卸载时钩子自清除，Ring0需要自行调用HookClear()

  \version    10.1.1507.2108
  \note       For All

  \author     triones
  \date       2010-08-27
*/

#pragma once

#include "xlib_base.h"

#if defined(FOR_RING0) && defined(__EXCEPTIONS)
# error "内核模式不支持C++异常，需关闭之"
#endif

//! Hook错误码(错误码不添加预处理指令，会造成显示值与实际值不符)
enum HookErrCode
  {
  HookErr_Success,                  //!< 成功完成
  HookErr_HookMem_Read_Fail,        //!< 尝试读取hookmem失败(一般是地址非法)
  HookErr_ProtectVirtualMemory_Fail,//!< 尝试ZwProtectVirtualMemory出错
  HookErr_MmCreateMdl_Fail,         //!< 尝试MmCreateMdl出错
  HookErr_MmMapLockedPages_Fail,    //!< 尝试MmMapLockedPages出错
  HookErr_HookMem_Write_Fail,       //!< 尝试写入失败(一般是权限不足)
  HookErr_DeleteNode_Fail,          //!< 尝试删除结点失败(一般是资源不足)
  HookErr_MemCannotCover,           //!< 地址无法再次覆盖(已存在不可覆盖的钩子)
  HookErr_MemCoverChk_Fail,         //!< 检测地址覆盖出错(链表操作出错)
  HookErr_AddNode_Fail,             //!< 加入结点失败(一般是资源不足)
  HookErr_SetUEF_Fail,              //!< 设置异常处理回调失败
  HookErr_ClearUEF_Fail,            //!< 清除异常处理回调失败
  HookErr_ClearUEF_Cover,           //!< 清除异常处理回调被覆盖
  HookErr_HookSize_OverFlow,        //!< hooksize超限(限制最大为15*2)
  HookErr_HookSize_Zero,            //!< hooksize为0
  HookErr_Routine_Illegal,          //!< Routin地址非法
  HookErr_MakeNode_Fail,            //!< 生成HookNode失败(一般是资源不足)
  HookErr_MakShellCode_Fail,        //!< 生成shellcode失败(一般是资源不足)
  HookErr_AntiShellCodeDEP_Fail,    //!< 清除shellcode的DEP失败(ZwProtectVirtualMemory出错)
  HookErr_FixShellCode_Fail,        //!< 调整shellcode失败
  HookErr_Ring0_NoUEF,              //!< 内核无顶层异常
  HookErr_AddDisp_Invalid,          //!< x64下，偏移无效
  HookErr_FixJmpCode_Fail,          //!< 设置jmpcode失败(一般是结点错误)
  HookErr_Hook_Fail,                //!< HOOK出错
  HookErr_UnHook_Restore,           //!< 钩子已经被还原(钩子结点会被删除)
  HookErr_UnHook_BeCover,           //!< 钩子位置被覆盖
  HookErr_UnHook,                   //!< 还原钩子失败
  HookErr_Clear,                    //!< 清除钩子链表失败
  HookErr_MovShellCode_Fail,        //!< 转移shellcode失败(一般是无效地址)
  };


/*!
  取得Hook执行错误码
  \return   参考HookErrCode的说明
*/
HookErrCode GetLastHookErr();

/*!
  指定地址以及缓冲区，写入内存
  \param  mem       指定地址
  \param  hookcode  缓冲区
  \param  len       缓冲区大小
  \return           写操作的执行结果，返回false时，请使用GetLastHookErr查询
*/
bool Hookit(LPVOID mem, LPCVOID hookcode, const size_t len);

/*!
  指定地址以及大小，NOP之
  \param  mem       指定地址
  \param  len       缓冲区大小
  \return           写操作的执行结果，返回false时，请使用GetLastHookErr查询
*/
bool Nopit(LPVOID mem, const size_t len);

//!< 地址偏移，x86与x64都是4 byte
typedef unsigned __int32 AddrDisp;

/*!
  偏移计算
  \param  mem   需要偏移处地址
  \param  dest  跳转目标处地址
  \return       偏移值
*/
size_t CalcOffset(const void* mem, const void* dest);

#ifdef _WIN64
//! x64检测是否有效的AddrDisp
bool IsValidAddrDisp(const size_t addrdisp);
#endif

//! 前置声明，不开放定义
class HookNode;

//! 标准回调函数使用的CPU结构
#pragma warning(push)
#pragma warning(disable:4510)  //C4510: 未能生成默认构造函数
#pragma warning(disable:4512)  //C4512: 未能生成赋值运算符
#pragma warning(disable:4610)  //C4610: 永远不能实例化
struct CPU_ST
  {
#ifndef _WIN64
  DWORD         regEdi;
  DWORD         regEsi;
  DWORD         regEbp;
  const DWORD   regEsp;    //!< 注意esp不能修改，改了也没意义
  DWORD         regEbx;
  DWORD         regEdx;
  DWORD         regEcx;
  DWORD         regEax;
  CPU_FLAGS     regflag;
  DWORD         regEip;
#else
  DWORD64       regRcx;
  DWORD64       regRdx;
  DWORD64       regR8;
  DWORD64       regR9;
  DWORD64       regRax;
  DWORD64       regRbx;
  const DWORD64 regRsp;
  DWORD64       regRbp;
  DWORD64       regRsi;
  DWORD64       regRdi;
  DWORD64       regR10;
  DWORD64       regR11;
  DWORD64       regR12;
  DWORD64       regR13;
  DWORD64       regR14;
  DWORD64       regR15;
  CPU_FLAGS     regflag;
  DWORD64       regRip;
#endif
  };
#pragma warning(pop)

//! 回调函数格式
#ifndef _WIN64
typedef void(__stdcall *HookRoutine)(CPU_ST* lpcpu);
#else
typedef void(*HookRoutine)(CPU_ST* lpcpu);
#endif

//! 普通Hook函数
/*!
  指定内存位置与回调函数，执行Hook操作。

  - x86情况下：\n
    - 当hooksize == 1~4：采用UEF，有强制钩子覆盖判定，不可下在已有钩子范围内；
    - 5：采用jmp XXX，有强制钩子覆盖判定，不可下在已有钩子范围内；
    - 6以上：采用jmp dword ptr [XXX]

  - x64情况下：\n
    - 当hooksize == 1~4：采用UEF，有强制钩子覆盖判定，不可下在已有钩子范围内；
    - 5：采用jmp XXX，偏移超出采用UEF，有强制钩子覆盖判定，不可下在已有钩子范围内；
    - 6-13：采用jmp qword ptr [XXX]，偏移超出采用UEF。采用UEF时，有覆盖判定
    - 14以上：采用jmp qword ptr [XXX]

  \param  hookmem     指定hook内存的位置
  \param  hooksize    hook长度(1~0xFF)
  \param  routine     指定的回调函数(声明请参考HookRoutine)
  \param  docodeend   true：覆盖代码后于回调执行，false：代码先行
  \param  p_shellcode 指定中转shellcode的存放位置\n
                      x86下可忽略，为兼容而设计。但设置也不影响使用\n
                      x64下用于避免偏移超出而自动采用UEF。
  \return             钩子结点指针，用于UnHook。\n
                      当执行失败时，返回nullptr。此时应调用GetLastHookErr得到失败原因

  \code
    #include "hook.h"
    void __stdcall Routine(CPU_ST* lpcpu)
      {
      return;
      }
    //在0x401000下5 byte JMP XXX钩子，代码先行
    HookNode* node = Hook((void*)0x401000,5,Routine,false);
    //在0x401000下6 byte JMP [XXX]钩子，代码后行
    HookNode* node = Hook((void*)0x401000,6,Routine,true);
    //在0x401000下1 byte UEF钩子，代码后行
    HookNode* node = Hook((void*)0x401000,1,Routine,true);
    if(node == nullptr)
      {
      cout << "下钩子出错，错误码：" << GetLastHookErr();
      }
  \endcode

  \note
  - 在hook位置复用时，注意根据需要特别注意先后行的选择\n
  - Routine由于设计及安全上的考虑，Esp值的改变将被忽略。
    其它改变都是允许的，但需要保证程序的继续运行，否则进程崩溃。\n
  - 在Routine内部期望修改EIP以达到流程控制效果时，谨慎选择前后行，
    推荐使用"代码先行"，否则在EIP被修改的情况下覆盖代码将不被执行。\n
  - 采用异常处理实现存在一定风险。在于：当顶层异常处理被其它处理覆盖时，
    钩子可能失效，并且这时恢复钩子，将破坏当前处理。(当然，概率较小)\n
  - 罕见使用：当希望使用UEF却又需要覆盖hooksize>=5时，
    请在回调函数中设置Eip一样可以实现效果，而不能使hooksize>=5实现UEF。
  - 由于没有添加偏移修正，Hook范围内不能存在偏移地址，否则旧代码执行出错。
  - Hook保证所有寄存器前后一致，除非回调内部特意修改。
*/
HookNode* Hook(void*              hookmem,
               const size_t       hooksize,
               HookRoutine        routine,
               const bool         docodeend,
               void*              p_shellcode = nullptr);

 /*!
  指定跳转表或call偏移位置，执行Hook操作。

  \param  hookmem     跳转表位置或call偏移位置
  \param  routine     指定的回调函数(声明请参考HookRoutine)
  \param  calltable_offset    指明是跳转表或call偏移
  \param  docallend   指明覆盖函数后于回调执行，或是先于回调执行
  \param  p_shellcode 指定中转shellcode的存放位置\n
                      x86下可忽略，为兼容而设计。但设置也不影响使用\n
                      x64下用于避免偏移超出而无法HOOK。
  \param  expandargc  覆盖函数可能存在参数过多的现象，以调整栈平衡
  \return             钩子结点指针，同样可以用UnHook卸载钩子。\n
                      当执行失败时，返回nullptr。此时应调用GetLastHookErr得到失败原因

  \code
    #include "hook.h"
    int __stdcall Test(int a,int b)
      {
      return a+b;
      }
    typedef int (__stdcall *func)(int a,int b);
    func oldfunc = Test;
    void __stdcall Routine(CPU_ST* lpcpu)
      {
      ...
      }
    HookNode* node = Hook((void*)&oldfunc,Routine,true,false);
    if(node == nullptr)
      {
      cout << "下钩子出错，错误码：" << GetLastHookErr();
      }
  \endcode

  \note
  - 覆盖跳转表或call偏移位置时，无视覆盖函数的调用格式。
  - 注意，docallend==true时，EIP为覆盖函数地址，Routine可选择改变之，但此时请特别注意原函数的调用格式，改变EIP可跳过原函数的执行。docallend==false时，EIP为返回地址，Routine也可选择改变之
  - docallend==true时，Hook保证寄存器前后一致。docallend==false时，Hook保证除esp外的寄存器在原函数调用前一致，局部环境一致。Routine调用寄存器前后一致。出Hook时寄存器一致，局部环境一致。（不因Hook而造成任何寄存器变动及栈飘移）
  - 使用MoveHookCallTableShellCode()伪造返回地址。
  - 未特殊说明的情况，参考Hook函数声明。
 */
 HookNode* Hook(void*             hookmem,
                HookRoutine       routine,
                const bool        calltable_offset,
                const bool        docallend,
                void*             p_shellcode = nullptr,
                const intptr_t    expandargc = 0);

//! UnHook函数
/*!
  指定HookNode结点指针，执行UnHook操作。\n
  注意：UnHook函数直接返回Hook错误码，不影响GetLastHookErr\n
  因为卸载钩子会有一些异常情况，强制卸载极可能会造成崩溃，\n
  所以可以指定错误停止

  \param  node      指定HookNode结点指针
  \param  errbreak  指定在卸载钩子遭遇错误时，是否停止卸载
  \return           当执行失败时，调用GetLastHookErr得到失败原因
    - \b HookErr_Success　      操作成功完成
    - \b HookErr_UnHook_Restore 钩子位置已经被还原(注意这只是一个Warning不影响卸载的安全性)
    - \b HookErr_UnHook_BeCover 钩子位置被覆盖(强制卸载有风险，可停止卸载)
    - \b HookErr_UnHook_Write   还原钩子时，写入失败(一般是权限不足)
    - \b HookErr_Del_NoExist    钩子不存在于链表(注意这只是Warning，卸载已完成)
    - \b HookErr_Unkown         未知异常(可能是个非法的HookNode)

  \code
    #include "hook.h"
    HookNode* node = Hook((void*)0x401000,5,Routin,false);
    if(node == nullptr) return;
    cout << "卸载钩子：" << UnHook(node,false);
  \endcode

*/
bool UnHook(HookNode* node,const bool errbreak);

//! 还原全部钩子 
/*!
  注意HookClear执行的是强制卸载操作，会尝试卸载链表中的每个钩子\n
  注意：HookClear函数也直接返回Hook错误码，不影响GetLastHookErr\n
  一般应用于动态库卸载的清理流程中。

  \return   当执行失败时，调用GetLastHookErr得到失败原因。返回的是最后一个卸载错误码
*/
bool HookClear();

//! 用于伪造call先行时的返回地址。
/*!
  x86情况下，mem至少提供B6空间。x64情况下，mem至少提供126空间
  \param  mem       指定转移地址。当为nullptr时，还原
  \return           当执行失败时，调用GetLastHookErr得到失败原因。
*/
bool MoveHookCallTableShellCode(void* mem);