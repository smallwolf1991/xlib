﻿/*!
  \file  updatelog.h
  \brief updatelog.h的内容是更新日志

  \author     triones
  \date       2014-03-06
*/

#pragma once

/*!
\section xlib_def
- \b 2014-01-07 从mkmem移植入xlib

\section xlib_link
- \b 2013-06-04 去除两个宏的undef，这样外部使用自动链接机制时不再需要重写文件。1.0~2.0
- \b 2013-11-28 新增AUTOLINK_X86_X64与AUTOLINK_VER宏。2.0~3.0
- \b 2014-01-06 稍作改造，移植进xlib

\section xlib_base
- \b 2010-07-05 虽然好像没有太大必要，还是新加operator  new[]
- \b 2010-11-23 转移入LIB，强制默认使用非分页内存。取消ExAllocatePool的宏定义
- \b 2010-11-26 发现遗漏取消ExFreePool的宏定义。
- \b 2011-07-26 发现在Ring0下释放空内存导致C2蓝屏，故在delete内部做处理，以保持外部代码一致性
- \b 2011-11-11 转移入COMMON，加上ring0头文件包含、try宏。2.0
- \b 2012-05-18 加入common_link包含，windows包含，struct包含
- \b 2014-01-06 移植入xlib

\section xlib_nt
- \b 2012-05-18 建立common_nt
- \b 2012-05-23 函数定义由宏实现，使用了宏不定参，没有完美解决法，但还好了已经
- \b 2014-01-07 移植入xlib，改名xlib_nt

\section swap
- \b 2014-01-07 从mkmem移植入xlib，虽然C++0x可以实现模版定义与实现分离，但不适用于LIB，为记

\section xrand
- \b 2013-01-11 新建xrand函数
- \b 2013-11-29 改进xrand函数以适应x64。1.0~2.0
- \b 2016-07-19 优化xrand。2.0~2.1

\section crc
- \b 2013-03-19 新建crc32函数
- \b 2013-03-20 修正溢出Bug及优化
- \b 2013-11-30 新增crc64函数。1.0~2.0
- \b 2014-02-18 新增crc16函数。2.0~3.0

\section xmutex
- \b 2013-03-01 新建xmutex

\section xevent
- \b 2013-03-01 新建xevent

\section Critical
- \b 2012-01-28 建立Critical

\section ws_s
- \b 2011-12-29 新建ws_s专门处理ws&s转换操作，处理ws&s的一个隐藏BUG\n
  重新调整ws&s的设计。1.0~1.1
- \b 2012-05-24 重新设计ws&s，使Ring0与Ring3代码通用。1.1~2.0\n
  同时发现原来对UNICODE_STRING的一个认识错误，修正BUG\n
  as,us.Length表示实际字符串占用的byte，不包含结尾null，但转换时生成null
- \b 2012-06-04 处理流程优化。2.0~2.1
- \b 2012-09-25 发现s2ws处理结尾null上的错误，修正之。2.1~2.2
- \b 2013-09-25 去除转换为标准库的重载。2.2~2.3
- \b 2014-01-11 引入sgistl，修改适应标准库。2.3~3.0
- \b 2014-04-17 细节改善。3.0~3.1
- \b 2014-05-07 细节改进。3.1~3.2
- \b 2014-08-26 改进使转换的完整。3.2~4.0
- \b 2015-05-09 修正输入数据过大导致转换不完全的BUG。4.0~5.0
- \b 2015-06-13 修正5.0版本产生的转换BUG。5.0~5.1
- \b 2016-03-14 修正s2ws处理给定ws缓冲计算错误的BUG。5.1~5.2

\section ws_utf8
- \b 2013-03-07 新增unicode与utf8编码的转换。\n
  ascii与utf8的转换需要自行先从ascii转换为unicode。0.1
- \b 2013-03-08 发现WinXP的ntdll不提供UTF函数，故参考UTF8文档，重新实现。0.1~1.0
- \b 2013-03-09 扩展支持4 byte Unicode
- \b 2013-05-25 处理转换时多处理一个数据的BUG。1.0~1.1
- \b 2014-01-11 引入sgistl，修改适应标准库。1.1~2.0
- \b 2014-04-08 修正utf8_byte2unicode_byte的一个bug；修正一些算法细节。2.0~2.1
- \b 2014-04-17 细节改善。2.1~2.2
- \b 2014-04-18 转变utf8_byte2unicode_byte使不自动跳过非法、不完整字符。2.2~3.0
- \b 2014-05-07 细节改进。3.0~3.1

\section xblk
- \b 2012-09-12 有封装需要，决定封装此操作。1.0

\section xmsg
- \b 2012-06-06 从xlog中分离出xmsg，重新定义为xmsg。1.0\n
  考虑xmsg继承mem_buffer，需要新建不少函数，暂不施行。
- \b 2012-07-19 优化prt
- \b 2012-10-09 新增对int64的支持。其它格式化作了些优化。1.0~1.1
- \b 2012-10-23 使xmsg信息组织后数据包含结尾0。xmsg初始时为"\0"而非空串。\n
  重载end、empty。1.1~2.0
- \b 2013-03-05 重载clear以修复缓冲清空后的小BUG。2.0~2.1
- \b 2014-01-11 引入sgistl，修改适应标准库。2.1~3.0
- \b 2014-04-09 修改一些数据的输出格式。3.0~3.1
- \b 2016-07-20 添加xmsg构造。3.1~3.2

\section xlog
- \b 2011-07-22 新建xlog
- \b 2011-07-30 增加xlog::dynamic_lvl用以控制动态输出等级。\n
  增加xlog_static_lvl宏用以控制静态输出等级\n
  新加endx函数，及相应操作符。\n
  基本实现完全替换dbgout的功能，故从LIB中去除dbgout
- \b 2011-08-20 引入mem_base_base使xlog继承之。0.1~1.0
- \b 2011-09-08 修复prt函数中自动扩展的BUG。
- \b 2011-10-17 将xlog的格式化处理提取形成xlog_base，方便继承用作其它用途。1.0~1.1
- \b 2011-12-12 移植入COMMON，对xlog新增一个静态变量用以类型控制，同时增加一批相关控制宏。\n
  自此引入类型控制输出功能。1.1~2.0
- \b 2012-04-24 对xlog_base新加一个输入函数operator<<(const xlog_base& v)
- \b 2011-06-06 把xlog_base分离出新文件，重命名为xmsg。xlog继承之
- \b 2012-09-25 决定三类输出控制分别独立。2.0~3.0
- \b 2012-10-24 由于xmsg包含结尾0，out函数不再追加

\section hex_str
- \b 2010-12-09 添加结构体hex_character，并对hex2str与str2hex函数进行相应改造，去除强制移位计算
- \b 2010-12-29 hex&str独立剥离，新建文件。3.0
- \b 2011-04-20 hex2str新加默认参数，提升str2hex功能。3.0~3.1
- \b 2011-06-02 新加重载的hex2str、str2hex函数，引入line类的返回。
- \b 2011-08-29 新加一组读取各种进制ASCII字符的转换函数，新加单个hex值转换ASCII函数。\n
  引入hexvalue模板
- \b 2011-12-20 撤销之前添加的一组转换函数，同时撤销hexvalue模板。另设计一个函数以替换之。
- \b 2012-02-27 发现str2hex的一个BUG，已修正
- \b 2012-06-07 删除Hex2Ascii
- \b 2013-03-05 修改返回xstr为返回xmsg。3.1~4.0
- \b 2013-03-20 新增str2hexs函数。4.0~5.0
- \b 2013-10-24 改进hex2show对ascii中文输出的优化。5.0~5.1
- \b 2013-11-13 改进hex2show可以输出utf8。5.1~5.2
- \b 2013-12-10 改进str2hex以适应x64。5.2~5.3
- \b 2014-01-13 引入sgistl，做适应性修改。5.3~6.0
- \b 2014-02-19 修正str2hex的一个小BUG。6.0~6.1
- \b 2014-04-09 修正str2hex的一个小BUG。6.1~6.2
- \b 2014-04-18 改进hex2show的字符集显示算法。6.2~7.0
- \b 2014-05-08 细节改进。7.0~7.1
- \b 2014-07-24 新加hex2show模版。7.1~7.2
- \b 2014-08-13 修正str2hexs的一个严重BUG。7.2~7.3

\section xline
- \b 2010-03-30 新加end()、remain()操作及+=、-=操作;
- \b 2010-04-02 增强setstr()功能
- \b 2010-04-03 增强getstr()功能
- \b 2010-04-12 新加复制构造函数，现在可以应用于容器。另对几个函数加上const以适应一些操作
- \b 2010-05-10 本要改成TCHAR*异常，但考虑特殊性，决定不采用，为记
- \b 2010-06-09 考虑到的确有初始时就需要非网络顺序情况，新加一个构造函数控制，而不去改变默认构造函数(ver 2.1)
- \b 2010-06-09 新增加一个功能line& operator<<(line& (*pfn)(line&)) (ver 2.2)
- \b 2010-10-08 增加一些成员函数以避免类型转换时可能导致困惑，但考虑还是保留原类型转换以保持向下兼容2.2~2.3
- \b 2010-11-06 移植入LIB，除了模板保留在头文件，其它小函数也不再inline，全部转移到CPP。由此做了相应调整2.3~3.0
- \b 2010-11-23 对Ring0支持。Ring0下，异常不抛出，但也暂时不知如何处理，结果无法预料。3.0~3.1\n
  实际改动只是移除异常抛出而已
- \b 2010-11-27 加进一些重载，增加灵活性。另外考虑隐式转换，重新调整函数声明顺序。3.1~3.2
- \b 2010-12-11 发现bswap对bool、结构体类型的支持上的BUG，修正之。3.2~3.3
- \b 2010-12-21 发现isempty函数错误，改正之\n
  发现>>重载与模板作用冲突，模板覆盖重载，暂时没有解决方案。
- \b 2010-12-28 由于setlen函数名称的歧义，改造之并将原始功能定义为lptolen。\n
  删除原内置模板bswap。采用外部bswap模板。3.3~3.4\n
  考虑把>>、<<操作符转移到外部函数，考虑良久，暂不行动。为记
- \b 2011-02-11 改动异常的抛出为TCHAR。
- \b 2011-02-18 提升netline为line类。3.4~4.0\n
  类中嵌入快捷缓冲（闪存），多数情况下能在堆栈直接分配空间，避免经常分配内存\n
  注意这次的改动，数据格式默认为非网络顺序\n
  去除数据类型转换操作，不再支持隐式转换。\n
  一旦申请缓冲，所有数据都将会在缓冲中，不再使用闪存。\n
  这次更新，缓冲只扩展，不再缩减。\n
  优化getbuf、end、getstr、setstr，优化对指针的处理
- \b 2011-04-18 屡次考虑变化计数器为us以缩减类大小，但权衡下保持为ui。为记\n
  决定缩减快捷缓冲大小以减小堆栈压力。
- \b 2011-04-19 重新调整net、fix标志，并增加zeroend、otherhead、headsize标志。4.0~4.1\n
  为新加的标志提供实现功能。\n
  本来考虑设置统一函数管理标志，这样导致代码不清晰及使用困难，作罢，为记\n
  重新调整构造函数，使line在构造时，能影响多个标志，保持了向下兼容的line(true)。
- \b 2011-04-20 修正昨天功能升级后的处理指针流BUG。\n
  顺便改了setmax的一些小细节，优化处理流程以提高运行效率。4.1~4.2\n
  正式移除LIB中的netline类。
- \b 2011-05-26 增加成员函数copystate以复制状态。4.2~4.3
- \b 2011-06-18 修改getstr、setstr使其具有返回值，返回值即处理数据长度。4.3~4.4
- \b 2011-06-22 新加cnull宏，方便处理ASCII时追加结尾0
- \b 2011-07-15 调整line_block，size_head；增加构造初始化对size_head的控制；调整构造函数；\n
  新增hideerr标志；调整needmax、needlen函数；新增initstate函数；调整init函数；\n
  去除lptolen这个危险函数；调整setmax函数；增加snull宏；
- \b 2011-07-16 修改函数命名以贴近标准容器操作，getbuf==begin；getlen==size；isempty==empty；\n
  去除operator=(const unsigned int)；去除operator+=(const unsigned int)；\n
  调整operator=(const line& nline)；调整operator+=(const line& nline)；\n
  调整setstr、getstr函数；添加operator>>(T const&)。4.4~5.0
- \b 2011-07-20 修复setstr转入CPP造成的诡异无法解释的编码错误。
- \b 2011-08-20 调整line类继承mem_base_base类。5.0~6.0\n
  此次封装提取了一些共同函数。同时删除了fix属性。
- \b 2011-09-16 修复setstr前缀处理的BUG。
- \b 2011-10-31 增加函数mkhead。
- \b 2011-12-14 移入COMMON，由于mem_base的变化，删除两个不再需要的重载
- \b 2012-01-30 发现mkhead的一个BUG，已修正
- \b 2012-05-25 状态修改函数添加前缀re
- \b 2012-05-28 针对基类变化做了一些调整。6.0~6.1
- \b 2012-06-06 重新设计line，升级为xline。6.0~7.0\n
  此次升级，所有状态以模版参数设置，不再接受状态临时变化\n
  保留line的定义，同时增加netline的定义
- \b 2012-09-06 修复>>(void*)的隐藏BUG
- \b 2013-03-15 修复<<(netline)的size两次翻转的BUG
- \b 2013-09-30 转移setstr、getstr到xvec成为put、pick。7.0~8.0
- \b 2014-01-13 引入sgistl，作适应性改动。8.0~9.0
- \b 2015-01-22 为数据不足情况补充可选择的异常机制。9.0~9.1

\section md5
- \b 2013-01-04 参考百度百科，重写md5过程
- \b 2013-01-07 测试通过。0.1~1.0
- \b 2013-03-20 改进MD5_VALUE，算法小优化。1.0~1.1
- \b 2016-07-20 优化md5接口。1.1~1.2

\section tean
- \b 2013-01-08 参考旧代码，重写TEAN
- \b 2013-01-11 理解TEAN算法，重新设计之，初步测试通过。0.1~1.0
- \b 2013-03-07 为TeanDecrypt添加合法判定。1.0~1.1
- \b 2013-03-19 导出TeanEncipher、TeanDecipher
- \b 2013-03-20 改进TEAN_DATA、TEAN_KEY。1.0~1.1
- \b 2013-03-21 改进TeanEncrypt及TeanDecrypt的返回值为netline。1.1~1.2
- \b 2014-09-09 修改基本算法为TEA，添加对XTEA的支持。1.2~2.0
- \b 2016-03-11 添加XXTEA算法。2.0~2.1
- \b 2016-07-20 优化接口。2.1~2.2

\section aes
- \b 2013-10-21 新建aes
- \b 2016-07-19 优化aes接口。1.0~1.1

\section syssnap
- \b 2013-08-06 新建sysinfo
- \b 2014-02-19 改变sysinfo为syssnap；融合Snapshot。1.0~2.0

\section pe
- \b 2012-07-20 初步实现Ring3支持
- \b 2014-01-23 完成Ring0支持。0.1~1.0
- \b 2014-05-08 细节改进。1.0~1.1
- \b 2016-07-20 添加合法判定。1.1~1.2
- \b 2016-07-21 添加异常处理。1.2~1.3
- \b 2016-10-13 修正之前1.3引入的循环递归BUG。1.3~1.4

\section hook
- \b 2010-07-05 为了增加对ring0的支持，以及新加一些功能，重新改写这个Crack\n
  由于ring0的局限性，ver 2.0下使用的vector无法使用，重新启用ver 1.0的自建结构\n
  自建结构为单向链表。ver 3.0对自建结构做了一些改进\n
  由于驱动的局限性，不再实现DLL或LIB，直接写成代码，使用包含之
- \b 2010-07-06 初步完成，以后需要详细测试。\n
- \b 2010-07-08 加进对跳转表下HOOK的功能\n
- \b 2010-08-20 新加一个标志:nounloadchk，并作一些相应更改
- \b 2010-08-26 认识到FixJMP XX的覆盖危险性，调整默认为JMPDD。这样也与自动决策一致
- \b 2010-08-27 由于Ring0下的一些需求，如SSDT hook或inline hook等。再次改写这个Crack\n
  不再提供用户选择错误返回。有错误一律在DebugView输出。\n
  Crack函数返回值为Cracknode类。\n
  提供GetLastCrackError()以返回最后一次错误信息，去除IsCrackID宏判断。\n
  增加新功能：原始字节绕过\n
  增加新功能：钩子延迟执行。提供CrackExec()以驱动延迟钩子执行。但会做匹配判断，以免在此期间目标位置数据变化。\n
  调动nounloadchk至全局标志\n
  加入：卸载时，钩子被外力还原，继续释放资源的判断。\n
  如果钩子是延迟加载，则在使用execute()成员函数执行时，错误信息不会被输出，只能自行获取。
- \b 2010-11-24 转移到LIB，没有根本改动
- \b 2011-11-11 转移入COMMON，核心不变，外壳重新设计Hook，新加一些功能，删除一些冗余功能，另外Ring0下的一些特殊操作还未添加。4.0~5.0
- \b 2012-03-23 修改Hookit的Ring3版本，不再调用WriteProcessMemory。5.0~6.0
- \b 2012-08-06 新增ReplaceHook函数以支持对跳转表的Hook。6.0~7.0
- \b 2012-09-25 一直有考虑开放HookNode定义，再次放弃，为记\n
  CheckMemFromList引入xblk判定。shellcode组织调整。ReplaceHook改名Hook重载。\n
  考虑增加相对偏移的Hook，但方案不成熟，暂不施行。7.0~7.1
- \b 2012-09-28 新增重载Hook用于跳转表、偏移的Hook。7.1~7.2
- \b 2012-10-08 优化原始Hook的shellcode。删除原跳转表Hook，由全新Hook代替。7.2~8.0
- \b 2013-04-12 新加Nopit函数
- \b 2014-01-23 重新设计Hook，原打算分解模块，尝试不满意，还原之。内核重新设计。8.0~9.0
- \b 2014-01-25 x64的支持初步完成。但实际操作过程不如意，有待完善。
- \b 2014-02-11 内核再次重新设计，初步完成，通过基本测试。9.0~10.0
- \b 2014-08-26 修正Ring3钩子自动卸载不完全的BUG。10.0~10.1
- \b 2016-06-12 添加Hook2Log模块。10.1~11.0\n
  由于钩子自动卸载机制，无法将Hook2Log独立出来，因为可能会使资源重复回收致使崩溃。

\section caller
- \b 2010-07-29 发现BUG，动用约定寄存器可能影响运行环境，所以保护之。并进一步优化保护
- \b 2010-10-06 去除对首次包含需要定义宏的要求，把函数定义为static。\n
  也去除了对参数容量的动态控制。如果对参数容量有更大要求，需要自行修改程序。\n
  不过，0x10个参数，应该够多了。另外，加入thiscall的包含。1.1~2.0
- \b 2010-11-05 移植入LIB，做相应改造。对常用情况做常用处理
- \b 2010-11-06 套用netline  提升成mkCall，重新恢复对参数容量的动态控制。2.0~3.0
- \b 2010-11-30 源码作一些改动，但不会影响实际效果。
- \b 2010-12-10 改造mkcallflags为mkmkCallFlag
- \b 2011-02-19 改造mkCall内部使用line类（原使用netline类）
- \b 2011-04-22 新增SmartCall类。3.0~4.0
- \b 2011-04-23 新增caller类。4.0~5.0\n
  原SmartCall类内存资源占用较大，但执行效率高。\n
  caller类内存资源占用小，但执行效率较低。（与mkCall基本一致）
- \b 2011-04-23 预备将ASM_Call完全提升为caller，暂时共存。过段时间caller经求大量测试无误后，将完全替代原ASM_Call
- \b 2011-06-25 正式移除ASM_Call、SmartCall。将全文件改名caller
- \b 2011-06-25 调整caller_regs为unsigned char以减少caller大小\n
  调整arg_R32类型为unsigned char
- \b 2011-06-26 新增arg_nul常量。\n
  大幅修改fakevtcall函数，不再要求预先写入this指针。\n
  新增changethis函数，允许临时改变this指针\n
  但是特别注意的是：这次修改需要运行时读取虚成员函数偏移，再次把崩溃风险引入fakevtcall。\n
  这次改动，使寄存器参数不再计入参数个数范围。5.0 ~ 6.0
- \b 2011-09-24 新增[]操作符用以改变this指针。这样使得临时改变this并调用函数变成：xx[this](...);
- \b 2011-09-27 发现升级6.0时产生00编码，今已去除。同时加上00编码结尾，以方便移植计算。\n
  新增mov_vt函数，用以移植。引入offseter与objer类。
- \b 2011-10-22 修正升级6.0时，类初始化未去除强制要求this指针的判定。
- \b 2011-12-14 移入COMMON,去除offseter与objer类。把类static变量转移入CPP
- \b 2012-05-22 小动作，把改变虚表的操作封装成mkvt函数
- \b 2012-10-09 重新设计caller的shellcode，寄存器保护更加严格。\n
  使caller返回值为uint64。6.0~7.0
- \b 2013-03-02 去除去虚函数表的修改。去除默认构造，强制caller构造。\n
  不定参调用不再使用虚函数。新加远程调用的支持。7.0~8.0
- \b 2013-12-09 发现远程调用时，寄存器参数个数没有实现不计，修正之。8.0~8.1
- \b 2014-02-26 完成xlib移植，shellcode作了一些优化。8.1~9.0
- \b 2014-08-26 shellcode修正。9.0~10.0
- \b 2015-07-21 发现x64下回调函数内部可能利用多余参数空间，故修改x64的shellcode以适应。10.0~10.1

\section xWSA
- \b 2012-06-08 把xWSA从xSock中分离出来成为独立封装
- \b 2012-11-13 重新设置xWSA，使WSA只启动一次。可不再被继承使用。1.0~2.0
- \b 2016-07-20 修改IpString的返回类型。2.0~2.1

\section xSock
- \b 2011-05-20 由于netSock多线程通讯的拙劣表现，决定重写之，变为Sock
- \b 2011-05-21 Sock采用简化封装，重载构造函数，合并其它功能。\n
  改造了apt函数为返回Sock指针\n
  不再内置send与recv操作，采用与line类的合作机制，用流方式实现通讯\n
- \b 2011-05-26 为Sock增加收发延时，默认50ms。这样避免了接收数据时，出现不必要阻塞。
- \b 2011-12-21 导入AddrInfo函数组。Sock改为xSock。
- \b 2012-01-14 添加IpString函数，由原来的outip变化而来
- \b 2012-02-02 recv处理时没有对主动断开作判断，已修正
- \b 2012-06-08 分离出基本操作，把xSock改为模板。1.0~2.0
- \b 2012-11-13 xSocket不再继承xWSA，包含之。\n
  新增SockDaulBuffer双缓冲类。\n
  xSock不再继承xSocket，包含之，继承SockDaulBuffer。\n
  新增几个便捷的流操作。2.0~3.0

\section xNetDl
- \b 2013-10-19 新建xNetDl
- \b 2015-01-22 添加可选的异常机制。1.0~1.1
- \b 2015-01-23 修改使可下载不定长数据。1.1~1.2

\section xServer
- \b 2012-01-28 加入xServer进入COMMON
- \b 2012-02-02 给xServer的catchit加入空过滤组判定，避免异常时无限触发
- \b 2012-04-25 对enum回调添加处理，凡在删除队列中的连接，不列入枚举

\section signaturematcher
- \b 2011-06-18 确立1.0版本(binsig)。0.1~1.0
- \b 2012-12-03 完全推翻1.0的机制，采用新技术，初步实现2.0版本，
- \b 2012-12-06 确定2.0版本，现与binsig共存，大量测试无误后，替代之。
- \b 2012-12-26 修正范围匹配达到最大范围时无法返回递进的BUG
- \b 2013-03-20 通过实测，库中移除binsig。
- \b 2014-04-01 改进binarysign为signaturematcher。现共存，确立版本2.0~3.0
- \b 2014-05-01 新加脚本处理。3.0~4.0
- \b 2014-05-05 处理了解析note词法的BUG。4.0~4.1
- \b 2014-06-03 处理了解析##&L词法的BUG。
- \b 2014-06-06 处理了匹配mark_ref的BUG、及代码一些小调整。mark_ref的定义微调。4.1~4.2
- \b 2014-07-29 code review中发现处理范围的一个失误，中间原子匹配时资源未回收，修正之。4.2~4.3
- \b 2016-01-08 修正因使用基类改进，而带进的范围处理始终失败的BUG。4.3~4.4
- \b 2016-03-21 修正match_markref的一个BUG，此BUG导致Ring0下链接失败。4.4~4.5
- \b 2016-03-31 改进防止内存不可读时产生异常。Ring0下异常无法捕获导致重启。4.5~4.6
- \b 2016-04-06 修正x64下偏移计算错误的Bug。4.6~4.7
- \b 2016-10-18 改进模块代码段的起始地址获取，避免取偏移计算造成不必要的偏差。4.7~5.0

\section des
- \b 2015-04-28 确立1.0版本
- \b 2016-07-19 优化des接口。1.0~1.1
*/