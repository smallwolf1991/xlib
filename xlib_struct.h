/*!
  \file  xlib_struct.h
  \brief xlib_struct.h定义了一些未文档化的结构，供Ring0、Ring3通用

  - 每个结构前列举了简要说明、加入时间及相关参考

  \author   triones
  \date     2011-4-8
*/
#pragma once

#include "xlib_def.h"

//系统信息结构枚举  20100820 1635  20120526
//http://undocumented.ntinternals.net/UserMode/Undocumented%20Functions/System%20Information/SYSTEM_INFORMATION_CLASS.html
//http://msdn.microsoft.com/en-us/library/windows/desktop/ms724509(v=vs.85).aspx
typedef enum _SYSTEM_INFORMATION_CLASS
  {
  SystemBasicInformation,         //右边为旧枚举
  SystemProcessorInformation,
  SystemPerformanceInformation,
  SystemTimeOfDayInformation,
  SystemPathInformation,          //SystemNotImplemented1,
  SystemProcessInformation,       //SystemProcessesAndThreadsInformation,
  SystemCallCountInformation,     //SystemCallCounts,
  SystemDeviceInformation,        //SystemConfigurationInformation,
  SystemProcessorPerformanceInformation,//SystemProcessorTimes,
  SystemFlagsInformation,         //SystemGlobalFlag,
  SystemCallTimeInformation,      //SystemNotImplemented2,
  SystemModuleInformation,
  SystemLocksInformation,         //SystemLockInformation,
  SystemStackTraceInformation,    //SystemNotImplemented3,
  SystemPagedPoolInformation,     //SystemNotImplemented4,
  SystemNonPagedPoolInformation,  //SystemNotImplemented5,
  SystemHandleInformation,
  SystemObjectInformation,
  SystemPageFileInformation,
  SystemVdmInstemulInformation,   //SystemInstructionEmulationCounts,
  SystemVdmBopInformation,        //SystemInvalidInfoClass1,
  SystemFileCacheInformation,     //SystemCacheInformation,
  SystemPoolTagInformation,
  SystemInterruptInformation,     //SystemProcessorStatistics,
  SystemDpcBehaviorInformation,   //SystemDpcInformation,
  SystemFullMemoryInformation,    //SystemNotImplemented6,
  SystemLoadGdiDriverInformation, //SystemLoadImage,
  SystemUnloadGdiDriverInformation,//SystemUnloadImage,
  SystemTimeAdjustmentInformation,//SystemTimeAdjustment,
  SystemSummaryMemoryInformation, //SystemNotImplemented7,
  SystemNextEventIdInformation,   //SystemNotImplemented8,
  SystemEventIdsInformation,      //SystemNotImplemented9,
  SystemCrashDumpInformation,
  SystemExceptionInformation,
  SystemCrashDumpStateInformation,
  SystemKernelDebuggerInformation,
  SystemContextSwitchInformation,
  SystemRegistryQuotaInformation,
  SystemExtendServiceTableInformation,//SystemLoadAndCallImage,
  SystemPrioritySeperation,
  SystemPlugPlayBusInformation,   //SystemNotImplemented10,
  SystemDockInformation,          //SystemNotImplemented11,
  SystemPowersInformation,        //SystemInvalidInfoClass2,
                                  //原SystemPowerInformation，但与winnt.h重定义冲突
  SystemProcessorSpeedInformation,//SystemInvalidInfoClass3,
  SystemCurrentTimeZoneInformation,//SystemTimeZoneInformation,
  SystemLookasideInformation,
                                  //以下为旧枚举，新枚举无
  SystemSetTimeSlipEvent,
  SystemCreateSession,
  SystemDeleteSession,
  SystemInvalidInfoClass4,
  SystemRangeStartInformation,
  SystemVerifierInformation,
  SystemAddVerifier,
  SystemSessionProcessesInformation
  } SYSTEM_INFORMATION_CLASS, *PSYSTEM_INFORMATION_CLASS;

//系统文件句柄类型枚举  20120303 1729
//http://hi.baidu.com/_achillis/blog/item/b175e2d254745ad8a8ec9a69.html
typedef enum _SYSTEM_HANDLE_TYPE
  {
  OB_TYPE_UNKNOWN,
  OB_TYPE_TYPE,
  OB_TYPE_DIRECTORY,
  OB_TYPE_SYMBOLIC_LINK,
  OB_TYPE_TOKEN,
  OB_TYPE_PROCESS,
  OB_TYPE_THREAD,
  OB_TYPE_JOB,
  OB_TYPE_DEBUG_OBJECT,
  OB_TYPE_EVENT,
  OB_TYPE_EVENT_PAIR,
  OB_TYPE_MUTANT,
  OB_TYPE_CALLBACK,
  OB_TYPE_SEMAPHORE,
  OB_TYPE_TIMER,
  OB_TYPE_PROFILE,
  OB_TYPE_KEYED_EVENT,
  OB_TYPE_WINDOWS_STATION,
  OB_TYPE_DESKTOP,
  OB_TYPE_SECTION,
  OB_TYPE_KEY,
  OB_TYPE_PORT,
  OB_TYPE_WAITABLE_PORT,
  OB_TYPE_ADAPTER,
  OB_TYPE_CONTROLLER,
  OB_TYPE_DEVICE,
  OB_TYPE_DRIVER,
  OB_TYPE_IOCOMPLETION,
  OB_TYPE_FILE,
  OB_TYPE_WMIGUID
  } SYSTEM_HANDLE_TYPE;

//CPU状态标志寄存器 20120517 1019 。x64 20131206 0927
/*
英特尔64和IA-32架构软件developer’s手册,第1卷、基本架构[PDF]
文件名:25366521.pdf 大小:3170961字节 日期:2006年9月 Vol.1 3-21 & 3-24
- O  one
- Z  zero
- S  Indicates a Status Flag
- C  Indicates a Control Flag
- X  Indicates a System Flag
*/
struct CPU_FLAGS
  {
  TULONG CF:1;   //!< S  Carry Flag
  TULONG O1:1;
  TULONG PF:1;   //!< S  Parity Flag
  TULONG Z3:1;

  TULONG AF:1;   //!< S  Auxiliary Carry Flag
  TULONG Z5:1;
  TULONG ZF:1;   //!< S  Zero Flag
  TULONG SF:1;   //!< S  Sign Flag

  TULONG TF:1;   //!< X  Trap Flag
  TULONG IF:1;   //!< X  Interrupt Enable Flag
  TULONG DF:1;   //!< C  Direction Flag
  TULONG OF:1;   //!< S  Overflow Flag

  TULONG IOPL:2; //!< X  I/O Privilege Level
  TULONG NT:1;   //!< X  Nested Task (NT)
  TULONG Z15:1;

  TULONG RF:1;   //!< X  Resume Flag
  TULONG VM:1;   //!< X  Virtual-8086 Mode
  TULONG AC:1;   //!< X  Alignment Check
  TULONG VIF:1;  //!< X  Virtual Interrupt Flag

  TULONG VIP:1;  //!< X  Virtual Interrupt Pending
  TULONG ID:1;   //!< X  ID Flag
  TULONG Z22:1;
  TULONG Z23:1;

  TULONG Z24:1;
  TULONG Z25:1;
  TULONG Z26:1;
  TULONG Z27:1;

  TULONG Z28:1;
  TULONG Z29:1;
  TULONG Z30:1;
  TULONG Z31:1;

#ifdef _WIN64
  TULONG ZReserved:32;  //!< x64下高32位保留
#endif
  };

//系统模块信息 20130807 1750 。x64 20131211 1600
//http://undocumented.ntinternals.net/UserMode/Structures/SYSTEM_MODULE.html
typedef struct _SYSTEM_MODULE
  {
  TULONG  Reserved[2];
  PVOID   ImageBaseAddress;
  ULONG   ImageSize;
  ULONG   Flags;
  WORD    Index;
  WORD    Rank;
  WORD    LoadCount;
  WORD    NameOffset;
  BYTE    Name[256];
  } SYSTEM_MODULE, *PSYSTEM_MODULE;

//系统模块信息 20130807 1752 。x64 20131211 1600
//http://undocumented.ntinternals.net/UserMode/Structures/SYSTEM_MODULE_INFORMATION.html
#pragma warning(push)

#   ifdef __INTEL_COMPILER
#       pragma warning(disable:94)//the size of an array must be greater than zero
#   else
#       pragma warning(disable:4200)//使用了非标准扩展 : 结构/联合中的零大小数组
#   endif

typedef struct _SYSTEM_MODULE_INFORMATION
  {
  TUINT         NumberOfModules:32; //x64下真实有效值只有32位
  SYSTEM_MODULE Information[0];
  } SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;

#pragma warning(pop)

//句柄系统信息 20100820 1631 。x64 20131211 1600
//http://forum.sysinternals.com/howto-enumerate-handles_topic18892.html
typedef struct _SYSTEM_HANDLE
  {
  ULONG       ProcessId;
  UCHAR       ObjectTypeNumber;
  UCHAR       Flags;
  USHORT      Handle;
  PVOID       Object;
  ACCESS_MASK GrantedAccess;
#ifdef _WIN64
  ACCESS_MASK GrantedAccessEx;
#endif
  } SYSTEM_HANDLE, *PSYSTEM_HANDLE;

//句柄系统信息扩展 20130806 1750 。x64 20131211 1600
//http://forum.sysinternals.com/howto-enumerate-handles_topic18892.html
#pragma warning(push)

#   ifdef __INTEL_COMPILER
#       pragma warning(disable:94)//the size of an array must be greater than zero
#   else
#       pragma warning(disable:4200)//使用了非标准扩展 : 结构/联合中的零大小数组
#   endif

typedef struct _SYSTEM_HANDLE_INFORMATION 
  {
  TUINT         NumberOfHandles:32; //x64下真实有效值只有32位
  SYSTEM_HANDLE Information[0];
  }SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;

#pragma warning(pop)

//系统线程信息 20140218 1203。x64 20140220 1057
//http://undocumented.ntinternals.net/UserMode/Undocumented%20Functions/System%20Information/Structures/SYSTEM_THREAD.html
typedef struct _SYSTEM_THREAD
  {
  LARGE_INTEGER           KernelTime;
  LARGE_INTEGER           UserTime;
  LARGE_INTEGER           CreateTime;
  TULONG                  WaitTime;
  PVOID                   StartAddress;
  CLIENT_ID               ClientId;
  KPRIORITY               Priority;
  LONG                    BasePriority;
  ULONG                   ContextSwitchCount;
  ULONG                   State;
  KWAIT_REASON            WaitReason;
  } SYSTEM_THREAD, *PSYSTEM_THREAD;

//系统进程信息 20140218 1204。x64 20140220 1057
//http://undocumented.ntinternals.net/UserMode/Undocumented%20Functions/System%20Information/Structures/SYSTEM_PROCESS_INFORMATION.html
#pragma warning(push)

#   ifdef __INTEL_COMPILER
#       pragma warning(disable:94)//the size of an array must be greater than zero
#   else
#       pragma warning(disable:4200)//使用了非标准扩展 : 结构/联合中的零大小数组
#   endif

typedef struct _SYSTEM_PROCESS_INFORMATION
  {
  ULONG                   NextEntryOffset;
  ULONG                   NumberOfThreads;
  LARGE_INTEGER           Reserved[3];
  LARGE_INTEGER           CreateTime;
  LARGE_INTEGER           UserTime;
  LARGE_INTEGER           KernelTime;
  UNICODE_STRING          ImageName;
  KPRIORITY               BasePriority;
  HANDLE                  ProcessId;
  HANDLE                  InheritedFromProcessId;
  TULONG                  HandleCount;
  ULONG                   Reserved2[2];
  TULONG                  PrivatePageCount;
  VM_COUNTERS             VirtualMemoryCounters;
  IO_COUNTERS             IoCounters;
  SYSTEM_THREAD           Threads[0];
  } SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

#pragma warning(pop)