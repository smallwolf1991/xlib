#include "hook.h"

#include <stdlib.h>
#include <string>
#include <vector>

#include "xlib_nt.h"
#include "xblk.h"
#include "xline.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! 全局HOOK错误码
static HookErrCode g_last_hook_err;

//! 设置HOOK错误码
static void SetLastHookErr(const HookErrCode new_errcode);

//! 最大实际修改字节。x86下6 byte，x64下12 byte
static const size_t gk_hook_max_byte2modify =
#ifdef _WIN64
sizeof(AddrDisp)+
#endif
1 + 1 + sizeof(void*);

//! 最大hooksize。真实指令最大长度15 byte
static const size_t gk_hook_max_byte2cover = 15 * 2;

//! 钩子结点结构
class HookNode
  {
  public:
    void*         mem;            //!< hook内存位置
    size_t        byte2cover;     //!< hook长度
    void*         routine;        //!< 回调地址
    void*         ip;             //!< eip/rip
    void*         lpshellcode;    //!< 指向shellcode
    unsigned char oldcode[gk_hook_max_byte2modify];//!< 覆盖前的数据(用于卸载时还原判定)
    unsigned char newcode[gk_hook_max_byte2modify];//!< 覆盖后的数据(用于卸载时覆盖判定)
    line          shellcode;      //!< 钩子代码
  };

//! 钩子链表
class HookList : public vector<HookNode*>
  {
  public:
    ~HookList();
  };

//!< 钩子链表
#ifndef FOR_RING0
//! RING3提供，ring3使用static类以实现自动初始化及自动卸载，故，不采用new HookList形式，
static HookList   g_hooklist;
static HookList*  g_hook_list = &g_hooklist;
#else
static HookList*  g_hook_list = nullptr;
#endif

//! 从链表中删除指定结点，无论链表中是否存在指定结点，都会删除。异常返回false，否则都true
static bool DeleteNode(HookNode* node);

//! 钩子覆盖判定，检查指定范围是否在链表中
static bool MemCanCover(void* mem, const size_t byte2cover);

//! 向链表中追加一个结点
static bool AddNode(HookNode* node);

#ifndef FOR_RING0
//! 存放旧UEF
static LPTOP_LEVEL_EXCEPTION_FILTER g_oldUEFHandling = LPTOP_LEVEL_EXCEPTION_FILTER(-1);

//! 顶层异常处理回调函数
static LONG WINAPI  HookUEFHandling(struct _EXCEPTION_POINTERS * ExceptionInfo);
#endif

//! 设置异常处理回调
static bool SetHookUEF(HookNode* node);

//! 清除异常处理回调
static bool ClearHookUEF();

//! 判定Hook长度是否符合要求
static bool Check_hooksize(const size_t hooksize);

//! 判定Hook地址是否可读可写
static bool Check_hookmem(void* hookmem);

//! 判定Routine是否有效
static bool Check_Routine(void* routine);

//! 初始化结点（普通版本）
static HookNode* MakeNode(void* hookmem, const size_t hooksize, void* routine);

//! 初始化结点（CallTable_Offset版本）
static HookNode* MakeNode(void* hookmem, void* routine, const bool calltable_offset);

//! 调整shellcode，主要是AntiDEP与设置指针
static bool FixShellCode(HookNode* node, void* p_shellcode);

//! 做普通hook shellcode
static bool MakeShellCode_Normal(HookNode* node, const bool docodeend);

//! 做CallTable_Offset hook shellcode
static bool MakeShellCode_CtOff(HookNode* node, const bool docallend, const intptr_t expandargc);

//! 做普通jmpcode
static bool FixJmpCode_Normal(HookNode* node);

//! 做CallTable_Offset jmpcode
static bool FixJmpCode_CtOff(HookNode* node, const bool calltable_offset);

//! 正式修改并加入链表
static bool HookIn(HookNode* node);



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#ifndef _WIN64
/*!
  要求前置shellcode如下

  \code
                                      --CoverCode--
  $ ==>    >  68 XXXXXXXX            push    EIP
  $+5      >  68 XXXXXXXX            push    Routinue
  $+A      >  FF15 XXXXXXXX          call    dword ptr [gk_lp_HookShellCode_Normal]
                                      --CoverCode--
  $+10     >  FF25 XXXXXXXX          jmp     [EIP]
  $+16
  \endcode
*/
static __declspec(naked) void HookShellCode_Normal()
  {
  __asm
    {
    push    dword ptr [esp + 4 * 2]       //参数EIP       ->ret routine
    pushfd
    pushad
    add     dword ptr [esp + 4 * 3], 4 * 5//修正esp       ->edi esi ebp  ->fd eip ret routine eip

    push    esp
    call    dword ptr [esp + 4 * 12]      //调用Routine   ->esp ad fd eip ret

    mov     eax, dword ptr [esp + 4 * 9]  //获取参数EIP   ->ad fd
    cmp     eax, dword ptr [esp + 4 * 12] //检测是否修改  ->ad fd eip ret routine
    mov     dword ptr [esp + 4 * 12], eax //修改EIP       ->ad fd eip ret routine

    popad

    jz      HookShellCode_Normal_Next
    popfd
    lea     esp, dword ptr [esp + 4 * 3]  //修改则跳过    ->eip ret routine
    retn

  HookShellCode_Normal_Next :
    popfd
    lea     esp, dword ptr [esp + 4 * 1]  //仅跳过EIP，返回执行可能存在的代码
    retn    4 * 2

    add     byte ptr [eax], al            //做0结尾
    }
  }

/*!
  要求前置shellcode如下

  \code
  $ ==>    >  68 XXXXXXXX            push    CoverCall
  $+5      >  68 XXXXXXXX            push    Routinue
  $+A      >  68 XXXXXXXX            push    HookArgc
  $+F      >  6A 00|01               push    docallend
  $+11     >  FF25 XXXXXXXX          jmp     dword ptr [gk_lp_HookShellCode_CtOff]
  $+17
  \endcode
*/
static __declspec(naked) void HookShellCode_CtOff()
  {
  __asm
    {
    pushfd

    xchg    eax, dword ptr [esp]          //eax为fd，原始值入栈
    xchg    eax, dword ptr [esp + 4 * 1]  //eax为docallend，fd向下移
    test    eax, eax                      //判定docallend

    pop     eax                           //还原eax

    jnz     HookShellCode_CtOff_Transit

    popfd

    push    0x01230814                    //做特征
    push    esp
    push    0x42104210
    mov     dword ptr [esp + 4 * 1], esp  //修正特征指向

    pushfd
    pushad                                //这里不用修正esp，修正也没意义

    mov     esi, esp

    mov     ecx, dword ptr [esp + 4 * 12] //获取argc       ->ad fd # esp #
    mov     edx, ecx
    shl     ecx, 2

    mov     ebx, dword ptr [esp + 4 * 14] //取得CoverCall  ->ad fd # esp # argc routine

    sub     esp, ecx                      //扩展局部栈
    mov     edi, esp

    xor     ecx, ecx
    mov     cl, 9                         //复制寄存器保护
    rep movsd

    mov     cl, 7                         //跳过           -># esp # argc ro cov ret
    rep lodsd

    mov     ecx, edx                      //参数复制
    rep movsd

    push    ebx
    pop     ebx                           //存放临时CoverCall

    popad
    popfd

    call    dword ptr [esp - 4 * 10]      //call CoverCall ->fd ad CoverCall

    push    edi
    push    esi
    push    ecx
    pushfd                                //环境保护

    mov     edi, esp                      //edi指向状态保护

    jmp     HookShellCode_CtOff_Chk
  HookShellCode_CtOff_Transit:
    jmp     HookShellCode_CtOff_Base

  HookShellCode_CtOff_Chk:
    add     esp, 4
    cmp     dword ptr [esp], 0x42104210
    jnz     HookShellCode_CtOff_Chk
    cmp     dword ptr [esp + 4], esp
    jnz     HookShellCode_CtOff_Chk
    cmp     dword ptr [esp + 8], 0x01230814
    jnz     HookShellCode_CtOff_Chk

    lea     esi, dword ptr [esp + 4 * 6]  //esi指向ret    -># esp # argc ro cov

    mov     ecx, dword ptr [esp + 4 * 3]  //提取argc      -># esp #
    shl     ecx, 2
    add     ecx, 4 * 11                   //计算位移      ->fd ecx esi edi # esp # argc ro cov ret

    lea     esp, dword ptr [edi + ecx]    //esp指向real_call_ret

    push    dword ptr [esi - 4 * 0]       //ret
    push    dword ptr [esi - 4 * 2]       //routine
    push    dword ptr [esi - 4 * 3]       //argc

    xchg    edi, esp
    popfd                                 //恢复环境
    pop     ecx
    pop     esi
    xchg    edi, esp
    mov     edi, dword ptr [edi]

    pushfd
  HookShellCode_CtOff_Base:
    popfd

    push    dword ptr [esp + 4 * 2]       //参数CoverCall ->argc routine
    pushfd
    pushad
    add     dword ptr [esp + 4 * 3], 4 * 5//修正esp       ->edi esi ebp ->fd eip argc routine eip

    push    esp
    call    dword ptr [esp + 4 * 12]      //调用Routine   ->esp ad fd eip argc

    popad
    popfd

    push    eax
    mov     eax, dword ptr [esp + 4 * 1]  //提取参数CoverCall
    mov     dword ptr [esp + 4 * 4], eax  //重写可能已被修改的CoverCall
    pop     eax

    lea     esp, dword ptr [esp + 4 * 3]  //跳过          ->CoverCall argc routine
    retn

    add     byte ptr [eax], al            //做0结尾
    }
  }
#else                 //#ifndef _WIN64
#   ifndef __INTEL_COMPILER
#   pragma message(" -- 不使用Intel C++ Compiler，HookShellCode版本可能较旧")
#pragma code_seg(".text")
__declspec(allocate(".text"))
static const char HookShellCode_Normal[] = {"\
\xFF\x74\x24\x10\x9C\x41\x57\x41\x56\x41\x55\x41\x54\x41\x53\x41\
\x52\x57\x56\x55\x48\x8D\x6C\x24\x50\x48\x8D\x75\x20\x56\x53\x50\
\x41\x51\x41\x50\x52\x51\x48\x89\xE1\x41\x51\x41\x50\x52\x51\x48\
\x8B\x45\x10\x50\x58\xFF\x54\x24\xF8\x59\x5A\x41\x58\x41\x59\x59\
\x5A\x41\x58\x41\x59\x58\x5B\x5D\x5D\x5E\x5F\x41\x5A\x41\x5B\x41\
\x5C\x41\x5D\x41\x5E\x41\x5F\x48\x87\x44\x24\x08\x48\x3B\x44\x24\
\x20\x48\x89\x44\x24\x20\x48\x87\x44\x24\x08\x74\x07\x9D\x48\x8D\
\x64\x24\x18\xC3\x9D\x48\x8D\x64\x24\x08\xC2\x10\x00" };
__declspec(allocate(".text"))
static const char HookShellCode_CtOff[] = {"\
\x9C\x48\x87\x04\x24\x48\x87\x44\x24\x08\x48\x85\xC0\x58\x75\x59\
\x9D\x68\x14\x08\x23\x01\x54\x68\x10\x42\x10\x42\x48\x89\x64\x24\
\x08\x9C\x57\x56\x52\x51\x50\x48\x8B\x44\x24\x58\x48\x8B\x4C\x24\
\x48\x50\x48\x89\xE6\x48\x89\xCA\x48\xC1\xE1\x03\x48\x2B\xE1\x48\
\x89\xE7\x48\x33\xC9\xB1\x07\xF3\x48\xA5\xB1\x07\xF3\x48\xAD\x48\
\x89\xD1\xF3\x48\xA5\x58\x58\x59\x5A\x5E\x5F\x9D\xFF\x54\x24\xC8\
\x57\x56\x51\x9C\x48\x89\xE7\xEB\x02\xEB\x4E\x48\x83\xC4\x08\x48\
\x81\x3C\x24\x10\x42\x10\x42\x75\xF2\x48\x39\x64\x24\x08\x75\xEB\
\x48\x81\x7C\x24\x10\x14\x08\x23\x01\x75\xE0\x48\x8D\x74\x24\x30\
\x48\x8B\x4C\x24\x18\x48\xC1\xE1\x03\x48\xC7\xC1\x58\x00\x00\x00\
\x48\x8D\x24\x0F\xFF\x36\xFF\x76\xF0\xFF\x76\xE8\x48\x87\xFC\x9D\
\x59\x5E\x48\x87\xFC\x48\x8B\x3F\x9C\x9D\xFF\x74\x24\x10\x9C\x41\
\x57\x41\x56\x41\x55\x41\x54\x41\x53\x41\x52\x57\x56\x55\x48\x8D\
\x6C\x24\x50\x48\x8D\x75\x20\x56\x53\x50\x41\x51\x41\x50\x52\x51\
\x48\x89\xE1\x41\x51\x41\x50\x52\x51\x48\x8B\x45\x10\x50\x58\xFF\
\x54\x24\xF8\x59\x5A\x41\x58\x41\x59\x59\x5A\x41\x58\x41\x59\x58\
\x5B\x5D\x5D\x5E\x5F\x41\x5A\x41\x5B\x41\x5C\x41\x5D\x41\x5E\x41\
\x5F\x9D\x50\x48\x8B\x44\x24\x08\x48\x89\x44\x24\x20\x58\x48\x8D\
\x64\x24\x18\xC3\x00"};
#pragma code_seg()
#   else                      //#ifndef __INTEL_COMPILER
/*!
  要求前置shellcode如下

  \code
                                     --CoverCode--
  $ ==>    >  EB 18                  jmp     $+1A
  $+2      >  XXXXXXXXXXXXXXXX       [RIP]
  $+A      >  XXXXXXXXXXXXXXXX       [Routine]
  $+12     >  XXXXXXXXXXXXXXXX       [gk_lp_HookShellCode_Normal]
  $+1A     >  FF35 E2FFFFFF          push    qword ptr [RIP]
  $+20     >  FF35 E4FFFFFF          push    qword ptr [Routine]
  $+26     >  FF15 E6FFFFFF          call    qword ptr [gk_lp_HookShellCode_Normal]
                                     --CoverCode--
  $+2C     >  FF25 D0FFFFFF?         jmp     [eip]
  $+32
  \endcode
*/
static __declspec(naked) void HookShellCode_Normal()
  {
  __asm
    {
    push    qword ptr [rsp + 8 * 2]       //参数RIP       ->ret routine

    pushfq
    push    r15
    push    r14
    push    r13
    push    r12
    push    r11
    push    r10
    push    rdi
    push    rsi
    push    rbp
    lea     rbp, qword ptr [rsp + 8 * 10] //指向参数RIP   ->rbp rsi rdi r10-15 fq
    lea     rsi, qword ptr [rbp + 8 * 4]  //指向ret       ->rip ret routin rip
    push    rsi
    push    rbx
    push    rax
    push    r9
    push    r8
    push    rdx
    push    rcx

    mov     rcx, rsp
    push    r9
    push    r8
    push    rdx
    push    rcx
    mov     rax, qword ptr [rbp + 8 * 2]  //提取Routine   ->rip ret
    push    rax
    pop     rax
    call    qword ptr [rsp - 8 * 1]       //调用Routine
    pop     rcx                           //弹出参数
    pop     rdx
    pop     r8
    pop     r9

    pop     rcx
    pop     rdx
    pop     r8
    pop     r9
    pop     rax
    pop     rbx
    pop     rbp                           //pop rsp
    pop     rbp
    pop     rsi
    pop     rdi
    pop     r10
    pop     r11
    pop     r12
    pop     r13
    pop     r14
    pop     r15

    xchg    rax, qword ptr [rsp + 8 * 1]  //取出参数RIP   ->fq
    cmp     rax, qword ptr [rsp + 8 * 4]  //检测是否修改  ->fq rip ret routine
    mov     qword ptr [rsp + 8 * 4], rax  //修改EIP       ->fq rip ret routine
    xchg    rax, qword ptr [rsp + 8 * 1]  //还原rax

    jz      HookShellCode_Normal_Next
    popfq
    lea     rsp, qword ptr [rsp + 8 * 3]  //修改则跳过    -> rip ret routine
    retn

  HookShellCode_Normal_Next :
    popfq
    lea     rsp, qword ptr [rsp + 8 * 1]  //仅跳过RIP，返回执行可能存在的代码
    retn    8 * 2

    add     byte ptr [rax], al            //做0结尾
    }
  }

/*!
  要求前置shellcode如下

  \code
  $ ==>    >  EB 28                  jmp     $+2A
  $+2      >  XXXXXXXXXXXXXXXX       [CoverCall]
  $+A      >  XXXXXXXXXXXXXXXX       [Routine]
  $+12     >  XXXXXXXXXXXXXXXX       [HookArgc]
  $+1A     >  XXXXXXXXXXXXXXXX       [docallend]
  $+22     >  XXXXXXXXXXXXXXXX       [gk_lp_HookShellCode_CtOff]
  $+2A     >  FF35 D2FFFFFF          push    qword ptr [CoverCall]
  $+30     >  FF35 D4FFFFFF          push    qword ptr [Routine]
  $+36     >  FF35 D6FFFFFF          push    qword ptr [HookArgc]
  $+3C     >  FF35 D8FFFFFF          push    qword ptr [docallend]
  $+42     >  FF35 DAFFFFFF          push    qword ptr [gk_lp_HookShellCode_CtOff]
  $+48     >  48 873C 24             xchg    rdi, qword ptr [rsp]
  $+4C     >  48 8B3F                mov     rdi, qword ptr [rdi]
  $+4F     >  48 873C 24             xchg    rdi, qword ptr [rsp]
  $+53     >  C3                     ret
  $+54
  \endcode
*/
static __declspec(naked) void HookShellCode_CtOff()
  {
  __asm
    {
    pushfq

    xchg    rax, qword ptr [rsp]          //rax为fq，原始值入栈
    xchg    rax, qword ptr [rsp + 8 * 1]  //rax为docallend，fq向下移
    test    rax, rax
    pop     rax

    jnz     HookShellCode_CtOff_Transit

    popfq
    push    0x01230814                    //push imm只允许32位，但入栈是64位
    push    rsp                           //注意后面会修正这个值，但不即时修正
    push    0x42104210                    //以上三个特征值为最后堆栈平衡设置
    mov     qword ptr [rsp + 8 * 1], rsp  //修正入栈rsp，以便最后栈平衡计算，rsp指向特征

    pushfq
    push    rdi
    push    rsi
    push    rdx
    push    rcx
    push    rax

    mov     rax, qword ptr [rsp + 8 * 11] //取得CoverCall ->rax rcx rdx rsi rdi fq # rsp # argc ro
    mov     rcx, qword ptr [rsp + 8 * 9]  //获取argc      ->rax rcx rdx rsi rdi fq # rsp #

    push    rax                           //push covercall

    mov     rsi, rsp
    mov     rdx, rcx
    shl     rcx, 3
    sub     rsp, rcx                      //扩展局部栈
    mov     rdi, rsp

    xor     rcx, rcx
    mov     cl, 7                         //复制寄存器保护->covercall rax rcx rdx rsi rdi fq
    rep movsq

    mov     cl,7                          //跳过          -># rsp # argc ro cov ret
    rep lodsq

    mov     rcx, rdx                      //参数复制
    rep movsq

    pop     rax
    pop     rax
    pop     rcx
    pop     rdx
    pop     rsi
    pop     rdi
    popfq
    call    qword ptr [rsp - 8 * 7]      //call CoverCall ->covercall rax rcx rdx rsi rdi fq

    push    rdi
    push    rsi
    push    rcx
    pushfq
    mov     rdi, rsp

    jmp     HookShellCode_CtOff_Chk

  HookShellCode_CtOff_Transit:
    jmp     HookShellCode_CtOff_Base

  HookShellCode_CtOff_Chk:
    add     rsp, 8
    cmp     qword ptr [rsp], 0x42104210
    jnz     HookShellCode_CtOff_Chk
    cmp     qword ptr [rsp + 8], rsp
    jnz     HookShellCode_CtOff_Chk
    cmp     qword ptr [rsp + 8 * 2], 0x01230814
    jnz     HookShellCode_CtOff_Chk

    lea     rsi, qword ptr [rsp + 8 * 6]  //rsi指向ret    -># rsp # argc ro cov

    mov     rcx, qword ptr [rsp + 8 * 3]  //提取argc      -># rsp #
    shl     rcx, 3
    mov     rcx, 8 * 11                   //计算位移      ->rcx rsi rdi fd # rsp # argc ro cov ret
    lea     rsp, dword ptr [rdi + rcx]    //rsp指向real_call_ret

    push    qword ptr [rsi - 8 * 0]       //ret
    push    qword ptr [rsi - 8 * 2]       //routine
    push    qword ptr [rsi - 8 * 3]       //argc

    xchg    rdi, rsp
    popfq                                 //恢复环境
    pop     rcx
    pop     rsi
    xchg    rdi, rsp
    mov     rdi, qword ptr [rdi]

    pushfq

  HookShellCode_CtOff_Base:
    popfq

    push    qword ptr [rsp + 8 * 2]       //参数CoverCall ->argc routine
    pushfq
    push    r15
    push    r14
    push    r13
    push    r12
    push    r11
    push    r10
    push    rdi
    push    rsi
    push    rbp
    lea     rbp, qword ptr [rsp + 8 * 10] //指向参数CoverCall ->rbp rsi rdi r10-15 fq
    lea     rsi, qword ptr [rbp + 8 * 4]  //指向ret       ->covercall argc routin covercall
    push    rsi                           //push rsp
    push    rbx
    push    rax
    push    r9
    push    r8
    push    rdx
    push    rcx

    mov     rcx, rsp
    push    rcx
    mov     rax, qword ptr [rbp + 8 * 2]  //提取Routine   ->covercall argc
    push    rax
    pop     rax
    call    qword ptr [rsp - 8 * 1]       //调用Routine
    pop     rcx                           //弹出参数

    pop     rcx
    pop     rdx
    pop     r8
    pop     r9
    pop     rax
    pop     rbx
    pop     rbp                           //pop rsp
    pop     rbp
    pop     rsi
    pop     rdi
    pop     r10
    pop     r11
    pop     r12
    pop     r13
    pop     r14
    pop     r15
    popfq

    push    rax
    mov     rax, qword ptr [rsp + 8 * 1]  //提取参数CoverCall
    mov     qword ptr [rsp + 8 * 4], rax  //重写可能已被修改的CoverCall
    pop     rax

    lea     rsp, qword ptr [rsp + 8 * 3]  //跳过          ->Covercall argc routine
    retn

    add     byte ptr [rax], al             //做0结尾
    }
  }
#   endif                     //#ifndef __INTEL_COMPILER
#endif                //#ifndef _WIN64

//! 指针，用于shellcode
static const void* const gk_lp_HookShellCode_Normal = (void*)HookShellCode_Normal;
static const void* gk_lp_HookShellCode_CtOff = (void*)HookShellCode_CtOff;


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SetLastHookErr(const HookErrCode new_errcode)
  {
  g_last_hook_err = new_errcode;
  }

HookErrCode GetLastHookErr()
  {
  return g_last_hook_err;
  }

bool Hookit(LPVOID mem, LPCVOID hookcode, const size_t len)
  {
  XLIB_TRY
    {
    unsigned char tmp;
    memcpy(&tmp, mem, 1);
    }
  XLIB_CATCH
    {
    SetLastHookErr(HookErr_HookMem_Read_Fail);
    return false;
    }

#ifndef FOR_RING0
  LPVOID Mem = mem;
  ULONG_PTR Len = (ULONG_PTR)len;
  ULONG_PTR oap;

  if(STATUS_SUCCESS != ZwProtectVirtualMemory(
    GetCurrentProcess(), &Mem, &Len, PAGE_EXECUTE_READWRITE, &oap))
    {
    SetLastHookErr(HookErr_ProtectVirtualMemory_Fail);
    return false;
    }

  XLIB_TRY
    {
    RtlCopyMemory(mem, hookcode, len);
    }
  XLIB_CATCH
    {
    SetLastHookErr(HookErr_HookMem_Write_Fail);
    return false;
    }

  ZwProtectVirtualMemory(GetCurrentProcess(), &Mem, &Len, oap, &oap);
#else
  PMDL pMDL = MmCreateMdl(nullptr, mem, len);
  if(pMDL == nullptr)
    {
    SetLastHookErr(HookErr_MmCreateMdl_Fail);
    return  false;
    }

  MmBuildMdlForNonPagedPool(pMDL);
  pMDL->MdlFlags = pMDL->MdlFlags | MDL_MAPPED_TO_SYSTEM_VA | MDL_WRITE_OPERATION;

  PVOID Mem = MmMapLockedPages(pMDL, KernelMode);
  if(Mem == nullptr)
    {
    SetLastHookErr(HookErr_MmMapLockedPages_Fail);
    return false;
    }

  XLIB_TRY
    {
    RtlCopyMemory(Mem, hookcode, len);
    }
  XLIB_CATCH
    {
    SetLastHookErr(HookErr_HookMem_Write_Fail);
    return false;
    }

  MmUnmapLockedPages(Mem, pMDL);
  IoFreeMdl(pMDL);
#endif
  SetLastHookErr(HookErr_Success);
  return true;
  }

bool Nopit(LPVOID mem, const size_t len)
  {
  const string str(len, '\x90');
  return Hookit(mem, str.c_str(), str.size());
  }

size_t CalcOffset(const void* mem, const void* dest)
  {
  return (size_t)dest - (size_t)mem - sizeof(AddrDisp);
  }

#ifdef _WIN64
bool IsValidAddrDisp(const size_t addrdisp)
  {
  if((intptr_t)addrdisp <= (intptr_t)(0x00000000FFFFFFFF) &&
     (intptr_t)addrdisp >= (intptr_t)(0xFFFFFFFF80000000))
    {
    return true;
    }
  return false;
  }
#endif

HookList::~HookList()
  {
  XLIB_TRY
    {
    if(!ClearHookUEF())  return;

    while(!empty())
      {
      UnHook(*begin(), false);
      }
    clear();
    }
  XLIB_CATCH
    {
    ;
    }
  }

bool DeleteNode(HookNode* node)
  {
  XLIB_TRY
    {
    if(node != nullptr)
      {
      delete node;

      if(g_hook_list != nullptr)
        {
        for(auto it = g_hook_list->begin(); it != g_hook_list->end(); ++it)
          {
          HookNode* now = *it;
          if(node == now)
            {
            g_hook_list->erase(it);
            break;
            }
          }
        }
      }
    SetLastHookErr(HookErr_Success);
    return true;
    }
  XLIB_CATCH
    {
    ;
    }
  SetLastHookErr(HookErr_DeleteNode_Fail);
  return false;
  }

bool MemCanCover(void* mem, const size_t byte2cover)
  {
  XLIB_TRY
    {
    if(g_hook_list != nullptr)
      {
      const xblk blkA(mem, byte2cover);
      for(const auto now : (*g_hook_list))
        {
        const xblk blkB(now->mem, now->byte2cover);
        if(blkB.checkin(blkA) != xblk::PD_NoIn)
          {
          SetLastHookErr(HookErr_MemCannotCover);
          return false;
          }
        }
      }
    SetLastHookErr(HookErr_Success);
    return true;
    }
  XLIB_CATCH
    {
    ;
    }
  SetLastHookErr(HookErr_MemCoverChk_Fail);
  return false;
  }

bool AddNode(HookNode* node)
  {
  XLIB_TRY
    {
    if(g_hook_list == nullptr)
      {
      g_hook_list = new HookList;
      }
    g_hook_list->push_back(node);
    SetLastHookErr(HookErr_Success);
    return true;
    }
  XLIB_CATCH
    {
    ;
    }
  SetLastHookErr(HookErr_AddNode_Fail);
  return false;
  }

#ifndef FOR_RING0
LONG WINAPI  HookUEFHandling(struct _EXCEPTION_POINTERS * ExceptionInfo)
  {
  if(ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_BREAKPOINT)
    {
    if(g_hook_list != nullptr)
      {
      for(const auto now : (*g_hook_list))
        {
#ifndef _WIN64
        if((DWORD)(now->mem) == ExceptionInfo->ContextRecord->Eip)
          {
          ExceptionInfo->ContextRecord->Eip = (DWORD)(now->lpshellcode);
          return  EXCEPTION_CONTINUE_EXECUTION;
          }
#else
        if((DWORD64)(now->mem) == ExceptionInfo->ContextRecord->Rip)
          {
          ExceptionInfo->ContextRecord->Rip = (DWORD64)(now->lpshellcode);
          return  EXCEPTION_CONTINUE_EXECUTION;
          }
#endif
        }
      }
    }
  if(g_oldUEFHandling == nullptr)  return  EXCEPTION_CONTINUE_SEARCH;
  return  g_oldUEFHandling(ExceptionInfo);
  }
#endif  //#ifndef FOR_RING0

bool SetHookUEF(HookNode* node)
  {
#ifdef FOR_RING0
  UNREFERENCED_PARAMETER(node);
  SetLastHookErr(HookErr_Ring0_NoUEF);
  return false;
#else
  XLIB_TRY
    {
    if(!MemCanCover(node->mem, node->byte2cover)) return false;
    //注意先替换U.E.F！
    LPTOP_LEVEL_EXCEPTION_FILTER  olds = SetUnhandledExceptionFilter(HookUEFHandling);
    //考虑到U.E.F可能被反复下，只保存最后一次非CrackUEFHandling的原U.E.F
    if(olds != &HookUEFHandling) g_oldUEFHandling = olds;
    node->newcode[0] = 0xCC;
    SetLastHookErr(HookErr_Success);
    return true;
    }
  XLIB_CATCH
    {
    ;
    }
  SetLastHookErr(HookErr_SetUEF_Fail);
  return false;
#endif  //#ifndef FOR_RING0
  }

bool ClearHookUEF()
  {
#ifdef FOR_RING0
  SetLastHookErr(HookErr_Success);
  return true;
#else
  XLIB_TRY
    {
    if(g_oldUEFHandling != (LPTOP_LEVEL_EXCEPTION_FILTER)(-1))  //如果有改变顶层异常
      {
      LPTOP_LEVEL_EXCEPTION_FILTER  olds =
        SetUnhandledExceptionFilter(g_oldUEFHandling);  //尝试还原UEF
      if(olds != &HookUEFHandling)
        {
        SetLastHookErr(HookErr_ClearUEF_Cover);
        return false;
        }
      }
    SetLastHookErr(HookErr_Success);
    return true;
    }
  XLIB_CATCH
    {
    ;
    }
  SetLastHookErr(HookErr_ClearUEF_Fail);
  return false;
#endif
  }

bool Check_hooksize(const size_t hooksize)
  {
  if(hooksize > gk_hook_max_byte2cover)
    {
    SetLastHookErr(HookErr_HookSize_OverFlow);
    return false;
    }
  if(hooksize == 0)
    {
    SetLastHookErr(HookErr_HookSize_Zero);
    return false;
    }
  SetLastHookErr(HookErr_Success);
  return true;
  }

bool Check_hookmem(void* hookmem)
  {
  unsigned char tmp[gk_hook_max_byte2modify];
  XLIB_TRY
    {
    memcpy(tmp, hookmem, sizeof(tmp));
    }
  XLIB_CATCH
    {
    SetLastHookErr(HookErr_HookMem_Read_Fail);
    return false;
    }
  return Hookit(hookmem, (LPCVOID)tmp, sizeof(tmp));
  }

bool Check_Routine(void* routine)
  {
  XLIB_TRY
    {
    unsigned char tmp[2];
    memcpy(tmp, routine, sizeof(tmp));
    SetLastHookErr(HookErr_Success);
    return true;
    }
  XLIB_CATCH
    {
    ;
    }
  SetLastHookErr(HookErr_Routine_Illegal);
  return false;
  }

HookNode* MakeNode(void* hookmem, const size_t hooksize, void* routine)
  {
  XLIB_TRY
    {
    HookNode* node = nullptr;

    if(!Check_hooksize(hooksize) ||
       !Check_hookmem(hookmem) ||
       !Check_Routine(routine))
      {
      return nullptr;
      }

    node = new HookNode;
    node->mem = hookmem;
    node->byte2cover = hooksize;
    node->routine = routine;
    node->ip = ((unsigned char*)hookmem + hooksize);
    node->lpshellcode = nullptr;

    memcpy(node->oldcode, node->mem, sizeof(node->oldcode));
    memcpy(node->newcode, node->mem, sizeof(node->newcode));

    SetLastHookErr(HookErr_Success);
    return node;
    }
  XLIB_CATCH
    {
    ;
    }
  SetLastHookErr(HookErr_MakeNode_Fail);
  return nullptr;
  }

HookNode* MakeNode(void* hookmem, void* routine, const bool calltable_offset)
  {
  XLIB_TRY
    {
    HookNode* node = nullptr;

    if(!Check_hooksize(sizeof(void*)) ||
       !Check_hookmem(hookmem) ||
       !Check_Routine(routine))
      {
      return nullptr;
      }

    node = new HookNode;
    node->mem = hookmem;
    node->byte2cover = (calltable_offset) ? sizeof(void*): sizeof(AddrDisp);
    node->routine = routine;
    node->ip = (calltable_offset)
      ? (*(void**)(hookmem))
      : (void*)(*(AddrDisp*)(hookmem) + (size_t)hookmem + sizeof(AddrDisp));
    node->lpshellcode = nullptr;

    memcpy(node->oldcode, node->mem, sizeof(node->oldcode));
    memcpy(node->newcode, node->mem, sizeof(node->newcode));

    SetLastHookErr(HookErr_Success);
    return node;
    }
  XLIB_CATCH
    {
    ;
    }
  SetLastHookErr(HookErr_MakeNode_Fail);
  return nullptr;
  }

bool FixShellCode(HookNode* node, void* p_shellcode)
  {
  XLIB_TRY
    {
    //前缀的添加完全是为了x64。x86下不使用但不影响
    line& scs = node->shellcode;

    unsigned char head[1 + 1 + sizeof(void*)] =
      { (unsigned char)'\xEB', (unsigned char)sizeof(void*), 0 };
    scs.insert(0, head, sizeof(head));

    if(p_shellcode != nullptr)
      {
      if(node->lpshellcode == nullptr)
        {
        node->lpshellcode = (void*)(scs.c_str());
        }

      if(!Hookit(p_shellcode, node->lpshellcode, scs.size()))
        return false;

      node->lpshellcode = p_shellcode;
      }
    else
      {
      node->lpshellcode = (void*)(scs.c_str());
      }
#ifndef FOR_RING0
    LPVOID Mem = (LPVOID)node->lpshellcode;
    ULONG_PTR Len = (ULONG_PTR)scs.size();
    ULONG_PTR oap;
    if(STATUS_SUCCESS != ZwProtectVirtualMemory(
      GetCurrentProcess(), &Mem, &Len, PAGE_EXECUTE_READWRITE, &oap))
      {
      SetLastHookErr(HookErr_AntiShellCodeDEP_Fail);
      return false;
      }
#endif
    *(void**)((unsigned char*)node->lpshellcode + 2) = node->lpshellcode;

    SetLastHookErr(HookErr_Success);
    return true;
    }
  XLIB_CATCH
    {
    ;
    }
  SetLastHookErr(HookErr_FixShellCode_Fail);
  return false;
  }

bool MakeShellCode_Normal(HookNode* node, const bool docodeend)
  {
  XLIB_TRY
    {
    line& scs = node->shellcode;

    if(!docodeend)    //代码前行需要先写原始代码
      {
      scs.append((unsigned char*)node->mem, node->byte2cover);
      }
#ifndef _WIN64
    scs << '\x68' << (AddrDisp)node->ip
      << '\x68' << (AddrDisp)node->routine
      << "\xFF\x15" << (AddrDisp)&gk_lp_HookShellCode_Normal;

    if(docodeend)
      {
      scs.append((unsigned char*)node->mem, node->byte2cover);  //代码后行后写原始代码
      }

    scs << "\xFF\x25" << (AddrDisp)&(node->ip);
#else     //#ifndef _WIN64
    scs << "\xEB\x18"
      << node->ip << node->routine << (void*)gk_lp_HookShellCode_Normal
      << "\xFF\x35\xE2\xFF\xFF\xFF\xFF\x35\xE4\xFF\xFF\xFF\xFF\x15\xE6\xFF\xFF\xFF";

    if(docodeend)
      {
      scs.append((unsigned char*)node->mem, node->byte2cover);  //代码后行后写原始代码
      scs << "\xFF\x25" << (AddrDisp)(0xFFFFFFD0 - node->byte2cover);
      }
    else
      {
      scs << "\xFF\x25" << (AddrDisp)0xFFFFFFD0;
      }

#endif     //#ifndef _WIN64
    SetLastHookErr(HookErr_Success);
    return true;
    }
  XLIB_CATCH
    {
    ;
    }
  SetLastHookErr(HookErr_MakShellCode_Fail);
  return false;
  }

bool MakeShellCode_CtOff(HookNode* node, const bool docallend, const intptr_t expandargc)
  {
  XLIB_TRY
    {
    line& scs = node->shellcode;

    static const intptr_t gk_hook_default_argc = 0x8;     //默认参数8个
    intptr_t hookargc = gk_hook_default_argc + expandargc;
    if(hookargc <= 0)  hookargc = gk_hook_default_argc;   //检测不让堆栈错误

#ifndef _WIN64
    scs << '\x68' << node->ip
      << '\x68' << node->routine
      << '\x68' << hookargc
      << '\x6A' << (char)docallend
      << "\xFF\x25" << (AddrDisp)&gk_lp_HookShellCode_CtOff;
#else     //#ifndef _WIN64
    scs << "\xEB\x28"
      << node->ip << node->routine << hookargc
      << (void*)docallend << (void*)&gk_lp_HookShellCode_CtOff
      << "\xFF\x35\xD2\xFF\xFF\xFF\xFF\x35\xD4\xFF\xFF\xFF\xFF\x35\xD6\xFF\xFF\xFF"
      << "\xFF\x35\xD8\xFF\xFF\xFF\xFF\x35\xDA\xFF\xFF\xFF"
      << "\x48\x87\x3C\x24\x48\x8B\x3F\x48\x87\x3C\x24\xC3";
#endif    //#ifndef _WIN64
    SetLastHookErr(HookErr_Success);
    return true;
    }
  XLIB_CATCH
    {
    ;
    }
  SetLastHookErr(HookErr_MakShellCode_Fail);
  return false;
  }

bool FixJmpCode_Normal(HookNode* node)
  {
  XLIB_TRY
    {
    switch(node->byte2cover)
      {
      case 1:  case 2: case 3: case 4:
        {
        if(!SetHookUEF(node)) return false;
        break;
        }
      case 5:
        {
        if(!MemCanCover(node->mem, node->byte2cover)) return false;
        node->newcode[0] = 0xE9;
        const size_t addrdisp = CalcOffset((unsigned char*)node->mem + 1, node->lpshellcode);
#ifdef _WIN64
        if(!IsValidAddrDisp(addrdisp))
          {
          if(!SetHookUEF(node)) return false;
          break;
          }
#endif    //#ifdef _WIN64
        *(AddrDisp*)(&node->newcode[1]) = (AddrDisp)addrdisp;
        break;
        }
#ifndef _WIN64
      default:
        {
        node->newcode[0] = 0xFF;
        node->newcode[1] = 0x25;
        *(AddrDisp*)(&node->newcode[2]) = (AddrDisp)&node->lpshellcode;
        break;
        }
#else     //#ifndef _WIN64
      case 6: case 7: case 8: case 9: case 10: case 11: case 12: case 13:
        {
        node->newcode[0] = 0xFF;
        node->newcode[1] = 0x25;
        const size_t addrdisp = CalcOffset((unsigned char*)node->mem + 2,
                                           (unsigned char*)node->lpshellcode + 2);
        if(!IsValidAddrDisp(addrdisp))
          {
          if(!SetHookUEF(node)) return false;
          break;
          }
        *(AddrDisp*)(&node->newcode[2]) = (AddrDisp)addrdisp;
        break;
        }
      default:
        {
        node->newcode[0] = 0xFF;
        node->newcode[1] = 0x25;
        *(AddrDisp*)(&node->newcode[2]) = 0;
        *(unsigned char**)(&node->newcode[2 + sizeof(AddrDisp)]) = (unsigned char*)node->lpshellcode;
        break;
        }
#endif    //#ifdef _WIN64
      }
    SetLastHookErr(HookErr_Success);
    return true;
    }
  XLIB_CATCH
    {
    ;
    }
  SetLastHookErr(HookErr_FixJmpCode_Fail);
  return false;
  }

bool FixJmpCode_CtOff(HookNode* node, const bool calltable_offset)
  {
  XLIB_TRY
    {
    if(calltable_offset)
      {
      *(void**)&node->newcode = node->lpshellcode;
      }
    else
      {
      size_t addrdisp = CalcOffset(node->mem, node->lpshellcode);
#ifdef _WIN64
      if(!IsValidAddrDisp(addrdisp))
        {
        SetLastHookErr(HookErr_AddDisp_Invalid);
        return false;
        }
#endif    //#ifndef _WIN64
      *(AddrDisp*)&node->newcode = (AddrDisp)addrdisp;
      }
    SetLastHookErr(HookErr_Success);
    return true;
    }
  XLIB_CATCH
    {
    ;
    }
  SetLastHookErr(HookErr_FixJmpCode_Fail);
  return false;
  }

bool HookIn(HookNode* node)
  {
  XLIB_TRY
    {
    const size_t thisbyte2cover =
    (node->byte2cover > sizeof(node->newcode))
    ? sizeof(node->newcode) : node->byte2cover;

    if(!Hookit(node->mem, node->newcode, thisbyte2cover)) return false;

    if(!AddNode(node))  return false;

    SetLastHookErr(HookErr_Success);
    return true;
    }
  XLIB_CATCH
    {
    ;
    }
  SetLastHookErr(HookErr_Hook_Fail);
  return false;
  }

HookNode* Hook(void*              hookmem,
               const size_t       hooksize,
               HookRoutine        routine,
               const bool         docodeend,
               void*              p_shellcode)
  {
  //////////////////////////////////////////////////////////////////////////第一步：MakeNode
  HookNode* node = MakeNode(hookmem,hooksize,routine);
  if(node == nullptr) return nullptr;
  //////////////////////////////////////////////////////////////////////////第二步：MakeShellCode
  if(!MakeShellCode_Normal(node, docodeend))
    {
    delete node;
    return nullptr;
    }
  if(!FixShellCode(node, p_shellcode))
    {
    delete node;
    return nullptr;
    }
  //////////////////////////////////////////////////////////////////////////第三步：FixJmpCode
  if(!FixJmpCode_Normal(node))
    {
    delete node;
    return nullptr;
    }
  //////////////////////////////////////////////////////////////////////////第四步：Hookin
  if(!HookIn(node))
    {
    delete node;
    return nullptr;
    }
  return node;
  }

HookNode* Hook(void*              hookmem,
               HookRoutine        routine,
               const bool         calltable_offset,
               const bool         docallend,
               void*              p_shellcode,
               const intptr_t     expandargc)
  {
  //////////////////////////////////////////////////////////////////////////第一步：MakeNode
  HookNode* node = MakeNode(hookmem,routine,calltable_offset);
  if(node == nullptr) return nullptr;
  //////////////////////////////////////////////////////////////////////////第二步：MakeShellCode
  if(!MakeShellCode_CtOff(node, docallend, expandargc))
    {
    delete node;
    return nullptr;
    }
  if(!FixShellCode(node, p_shellcode))
    {
    delete node;
    return nullptr;
    }
  //////////////////////////////////////////////////////////////////////////第三步：FixJmpCode
  if(!FixJmpCode_CtOff(node,calltable_offset))
    {
    delete node;
    return nullptr;
    }
  //////////////////////////////////////////////////////////////////////////第四步：Hookin
  if(!HookIn(node))
    {
    delete node;
    return nullptr;
    }
  return node;
  }

bool UnHook(HookNode* node,const bool errbreak)
  {
  XLIB_TRY
    {
    const size_t thisbyte2cover =
      (node->byte2cover > sizeof(node->newcode))
      ? sizeof(node->newcode) : node->byte2cover;

    if(memcmp(node->mem,node->oldcode,thisbyte2cover) == 0)
      {
      DeleteNode(node);
      SetLastHookErr(HookErr_UnHook_Restore);
      return false;
      }
    else
      {
      if(memcmp(node->mem,node->newcode,thisbyte2cover) == 0)
        {
        SetLastHookErr(HookErr_UnHook_BeCover);
        if(errbreak)  return false;
        }

      if(!Hookit(node->mem, node->oldcode, thisbyte2cover))
        {
        if(errbreak)  return false;
        }
      }
    return DeleteNode(node);
    }
  XLIB_CATCH
    {
    ;
    }
  SetLastHookErr(HookErr_UnHook);
  return false;
  }

bool  HookClear()
  {
  XLIB_TRY
    {
    if(!ClearHookUEF())  return false;

    if(g_hook_list == nullptr)  return true;

    while(!g_hook_list->empty())
      {
      UnHook(*(g_hook_list->begin()), false);
      }
#ifdef FOR_RING0
    delete g_hook_list;
    g_hook_list = nullptr;
#endif
    return GetLastHookErr() == HookErr_Success;
    }
  XLIB_CATCH
    {
    ;
    }
  SetLastHookErr(HookErr_Clear);
  return false;
  }

bool MoveHookCallTableShellCode(void* mem)
  {
  XLIB_TRY
    {
    if(mem == nullptr)
      {
      gk_lp_HookShellCode_CtOff = (void*)HookShellCode_CtOff;
      SetLastHookErr(HookErr_Success);
      return true;
      }

    const unsigned char* lp = (unsigned char*)HookShellCode_CtOff;
    size_t len = 0;
    while(lp[len++]);

    Hookit(mem, lp, len);
    gk_lp_HookShellCode_CtOff = (void*)mem;

    }
  XLIB_CATCH
    {
    ;
    }
  SetLastHookErr(HookErr_MovShellCode_Fail);
  return false;
  }