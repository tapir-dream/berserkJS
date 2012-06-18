#include "appinfo.h"

#if defined(Q_OS_WIN32)
#include <w32api.h>
#include <windows.h>
#include <psapi.h>

CRITICAL_SECTION cs;  // 供多线程同步的临界区变量
HANDLE hd;    // 当前进程的句柄
DWORD t1;     // 时间戳
double percent;  // 最近一次计算的CPU占用率

__int64 oldp;

// 时间格式转换
__int64 fileTimeToInt64(const FILETIME& time)
{
    ULARGE_INTEGER tt;
    tt.LowPart = time.dwLowDateTime;
    tt.HighPart = time.dwHighDateTime;
    return(tt.QuadPart);
}

// 得到进程占用的CPU时间
int getTime(__int64& proc)
{
    FILETIME create;
    FILETIME exit;
    FILETIME ker;  // 内核占用时间
    FILETIME user; // 用户占用时间
    if(!GetProcessTimes(hd, &create, &exit, &ker, &user)){
        return -1;
    }

    proc = (fileTimeToInt64(ker) + fileTimeToInt64(user)) / 10000;
    return 0;
}

// 获取内核总数
int getCPUCoresCount()
{
    SYSTEM_INFO siSysInfo;
    // Copy the hardware information to the SYSTEM_INFO structure.
    GetSystemInfo(&siSysInfo);
    return siSysInfo.dwNumberOfProcessors;

    /*
    // Display the contents of the SYSTEM_INFO structure.

    printf("Hardware information: \n");
    printf("  OEM ID: %u\n", siSysInfo.dwOemId);
    printf("  Number of processors: %u\n",
     siSysInfo.dwNumberOfProcessors);
    printf("  Page size: %u\n", siSysInfo.dwPageSize);
    printf("  Processor type: %u\n", siSysInfo.dwProcessorType);
    printf("  Minimum application address: %lx\n",
     siSysInfo.lpMinimumApplicationAddress);
    printf("  Maximum application address: %lx\n",
     siSysInfo.lpMaximumApplicationAddress);
    printf("  Active processor mask: %u\n",
     siSysInfo.dwActiveProcessorMask);s
    */
}

void init()
{
    // 初始化线程临界区变量
    InitializeCriticalSection(&cs);
    // 初始的占用率
    percent = 0;
    // 得到当前进程id
    DWORD pid = GetCurrentProcessId();
    // 通过id得到进程的句柄
    hd = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if(hd == NULL){
        return;
    }
    // 得到初始时刻的值
    getTime(oldp);
    t1 = GetTickCount();
}

void destroy()
{
    if(hd != NULL){
        CloseHandle(hd);
    }
    DeleteCriticalSection(&cs);
}

// 进行换算
double cpu()
{
    int time = 15; // 毫秒数。用一个比较少的时间片作为计算单位，这个值可修改
    if(hd == NULL) {
        return 0;
    }

    EnterCriticalSection(&cs);
    DWORD t2 = GetTickCount();
    DWORD dt = t2 - t1;
    if(dt > time) {
       __int64 proc;
       getTime(proc);

       percent = ((proc - oldp) * 100)/dt;

       t1 = t2;
       oldp = proc;
    }
    LeaveCriticalSection(&cs);
    return int(percent/getCPUCoresCount()*10)/10.0;
}

double memory()
{
    /*
    而内存信息结构的定义如下：
    typedef struct _PROCESS_MEMORY_COUNTERS {
        DWORD cb;
        DWORD PageFaultCount; // 分页错误数目
        SIZE_T PeakWorkingSetSize; // 工作集列 ( 物理内存 ) 的最大值
        SIZE_T WorkingSetSize; // 工作集列 ( 物理内存 ) 的大小
        SIZE_T QuotaPeakPagedPoolUsage; // 分页池的峰值的最大值
        SIZE_T QuotaPagedPoolUsage; // 分页池的峰值大小
        SIZE_T QuotaPeakNonPagedPoolUsage; // 非分页池的峰值的最大值
        SIZE_T QuotaNonPagedPoolUsage; // 非分页池的峰值大小
        SIZE_T PagefileUsage; // 页文件页的大小（虚拟内存）
        SIZE_T PeakPagefileUsage; // 页文件页的最大值
     }
     */

    if(hd == NULL) {
        return 0;
    }
    PROCESS_MEMORY_COUNTERS pmc;
    pmc.cb = sizeof(PROCESS_MEMORY_COUNTERS);
    GetProcessMemoryInfo(hd, &pmc, sizeof(pmc));
    // WorkingSetSize 值通常会比在任务管理器中看到的值要大
    // 因为它是表示程序使用内存的工作总集
    // 而任务管理器中仅仅显示专用内存集
    // 其区别在于后者不包括可共享的内存（如加载的DLL）
    return (pmc.WorkingSetSize/1024);
}

#endif

#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
#include <unistd.h>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <QString>

void exec(const char *cmd, char *result)
{

    FILE* stream;
    char buf[128] = {0};
    stream = popen(cmd, "r" );
    //将FILE* stream的数据流读取到buf中
    fread(buf, sizeof(char), sizeof(buf), stream);
    // buf 复制
    strcpy(result, buf);
    pclose(stream);
}

double cpu()
{
    int pid = int(getpid());
    char result[128];
    std::stringstream newstr;
    newstr<<pid;
    std::string cmd = "ps aux|grep " + newstr.str() + "|awk '{print $3}'|awk 'NR==1'";
    exec(cmd.c_str(), result);
    return QString(result).toDouble();
}

double memory()
{
    int pid = int(getpid());
    char result[128];
    std::stringstream newstr;
    newstr<<pid;
    std::string cmd = "ps aux|grep " + newstr.str() + "|awk '{print $6}'|awk 'NR==1'";
    exec(cmd.c_str(), result);
    return QString(result).toDouble();
}

#endif

AppInfo::AppInfo()
{
    #ifdef Q_OS_WIN32
    init();
    #endif
}

AppInfo::~AppInfo()
{
    #ifdef Q_OS_WIN32
    destroy();
    #endif
}

double AppInfo::getCPU()
{
    return cpu();
}

double AppInfo::getMemory()
{
    return memory();
}
