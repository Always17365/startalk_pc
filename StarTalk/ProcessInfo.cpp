
#include "ProcessInfo.h"
#include "MessageManager.h"
#include <QDebug>
#include <QDateTime>
#include <sstream>
// Windows lib import
#ifdef _WINDOWS
#include "qt_windows.h"
#include <DbgHelp.h>
#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#endif

// macOS lib import
#ifdef _MACOS
#include <mach/task.h>
#include <mach/thread_act.h>
#include <mach/vm_map.h>
#include <sys/types.h>
#include <mach/mach_init.h>
#include <mach/mach_port.h>
#include <mach/mach_traps.h>
#include <mach/task_info.h>
#include <mach/thread_info.h>
#include <mach/vm_region.h>
#include <mach/shared_memory_server.h>
#include <zconf.h>
#endif

ProcessInfo::ProcessInfo(QObject* parent)
	: QThread(parent)
{

}

//
ProcessInfo::~ProcessInfo()
{
	_run = false;
	wait();
}

#ifdef _WINDOWS
qint64 fileTimeToMSecs(const FILETIME& fileTime)
{
	return ((qint64(fileTime.dwHighDateTime) << 32) | fileTime.dwLowDateTime) / 10000;
}

static int get_processor_number()
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return (int)info.dwNumberOfProcessors;
}
#endif

qreal getCpuInfo()
{
#ifdef _WINDOWS
    static qint64 last_time = 0;
	static qint64 last_system_time = 0;
	qint64 system_time {};
	static int processor_count_ = -1;
	if (processor_count_ == -1)
		processor_count_ = get_processor_number();

	FILETIME now;
	FILETIME creation_time;
	FILETIME exit_time;
	FILETIME kernel_time;
	FILETIME user_time;

	GetSystemTimeAsFileTime(&now);

	if (!GetProcessTimes(GetCurrentProcess(), &creation_time, &exit_time, &kernel_time, &user_time))
		return -1;

	system_time = (fileTimeToMSecs(kernel_time) + fileTimeToMSecs(user_time)) / processor_count_;
	long long time = fileTimeToMSecs(now);

	if ((last_system_time == 0) || (last_time == 0))
	{
		last_system_time = system_time;
		last_time = time;
		return -2;
	}

	qint64 time_delta = time - last_time;
	if (time_delta == 0) {
		return -1;
	}

	float cpu = (float)(system_time - last_system_time) * 100 / (float)time_delta;
	last_system_time = system_time;
	last_time = time;

	return cpu;
#endif // _WINDOWS
#ifdef _MACOS

    kern_return_t kr;
    task_info_data_t tinfo;
    mach_msg_type_number_t task_info_count;

    task_info_count = TASK_INFO_MAX;
    kr = task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)tinfo, &task_info_count);
    if (kr != KERN_SUCCESS) {
        return -1;
    }

//    task_basic_info_t      basic_info;
    thread_array_t         thread_list;
    mach_msg_type_number_t thread_count;

    thread_info_data_t     thinfo;
    mach_msg_type_number_t thread_info_count;

    thread_basic_info_t basic_info_th;
//    uint32_t stat_thread = 0; // Mach threads

//    basic_info = (task_basic_info_t)tinfo;

    // get threads in the task
    kr = task_threads(mach_task_self(), &thread_list, &thread_count);
    if (kr != KERN_SUCCESS) {
        return -1;
    }
//    if (thread_count > 0)
//        stat_thread += thread_count;

    long tot_sec = 0, tot_usec = 0;
    double tot_cpu = 0;

    for (int j = 0; j < (int)thread_count; j++)
    {
        thread_info_count = THREAD_INFO_MAX;
        kr = thread_info(thread_list[j], THREAD_BASIC_INFO,
                         (thread_info_t)thinfo, &thread_info_count);
        if (kr != KERN_SUCCESS) {
            return -1;
        }

        basic_info_th = (thread_basic_info_t)thinfo;

        if ((basic_info_th->flags & TH_FLAGS_IDLE) == 0) {
            tot_sec = tot_sec + basic_info_th->user_time.seconds + basic_info_th->system_time.seconds;
            tot_usec = tot_usec + basic_info_th->user_time.microseconds + basic_info_th->system_time.microseconds;
            tot_cpu = tot_cpu + basic_info_th->cpu_usage / (float)TH_USAGE_SCALE * 100.0;
        }

    } // for each thread

    kr = vm_deallocate(mach_task_self(), (vm_offset_t)thread_list, thread_count * sizeof(thread_t));
    assert(kr == KERN_SUCCESS);

    return tot_cpu;
#endif // _MACOS
    return 0;
}

#if defined(Q_OS_MAC)
typedef struct { /* dynamic process information */
    size_t rss, vsize;
    double utime, stime;
} RunProcDyn;

int run_get_dynamic_proc_info(RunProcDyn *rpd)
{
    task_t task;
    kern_return_t error;
    mach_msg_type_number_t count;
    struct task_basic_info ti{};

    pid_t pid = getpid();
    error = task_for_pid(mach_task_self(), pid, &task);
    if (error != KERN_SUCCESS) {
        rpd->rss = rpd->vsize = 0;
        rpd->utime = rpd->stime = 0;
        return 0;
    }
    count = TASK_BASIC_INFO_COUNT;
    error = task_info(task, TASK_BASIC_INFO, (task_info_t)&ti, &count);
    assert(error == KERN_SUCCESS);
    {
        vm_region_basic_info_data_64_t b_info;
        vm_address_t address = GLOBAL_SHARED_TEXT_SEGMENT;
        vm_size_t size;
        mach_port_t object_name;
        count = VM_REGION_BASIC_INFO_COUNT_64;
        error = vm_region_64(task, &address, &size, VM_REGION_BASIC_INFO,
                             (vm_region_info_t)&b_info, &count, &object_name);
        if (error == KERN_SUCCESS) {
            if (b_info.reserved && size == (SHARED_TEXT_REGION_SIZE) &&
                ti.virtual_size > (SHARED_TEXT_REGION_SIZE + SHARED_DATA_REGION_SIZE))
            {
                ti.virtual_size -= (SHARED_TEXT_REGION_SIZE + SHARED_DATA_REGION_SIZE);
            }
        }
        rpd->rss = ti.resident_size;
        rpd->vsize = ti.virtual_size;
    }
//    {
//        unsigned i;
//        rpd->utime = ti.user_time.seconds + ti.user_time.microseconds * 1e-6;
//        rpd->stime = ti.system_time.seconds + ti.system_time.microseconds * 1e-6;
//        error = task_threads(task, &thread_table, &table_size);
//        assert(error == KERN_SUCCESS);
//        thi = &thi_data;
//        for (i = 0; i != table_size; ++i) {
//            count = THREAD_BASIC_INFO_COUNT;
//            error = thread_info(thread_table[i], THREAD_BASIC_INFO, (thread_info_t)thi, &count);
//            assert(error == KERN_SUCCESS);
//            if ((thi->flags & TH_FLAGS_IDLE) == 0) {
//                rpd->utime += thi->user_time.seconds + thi->user_time.microseconds * 1e-6;
//                rpd->stime += thi->system_time.seconds + thi->system_time.microseconds * 1e-6;
//            }
//            if (task != mach_task_self()) {
//                error = mach_port_deallocate(mach_task_self(), thread_table[i]);
//                assert(error == KERN_SUCCESS);
//            }
//        }
//        error = vm_deallocate(mach_task_self(), (vm_offset_t)thread_table, table_size * sizeof(thread_array_t));
//        assert(error == KERN_SUCCESS);
//    }
    mach_port_deallocate(mach_task_self(), task);
    return 0;
}
#elif defined(Q_OS_WIN)
    struct WinMemory {
        unsigned long pSize; // peak size
        unsigned long size;  // resident size
        unsigned long pVSize; // peak virtual size
        unsigned long vSize; // virtual size
    };

    int getMemoryInfo(WinMemory * winSize)
    {
        HANDLE handle=GetCurrentProcess();
        PROCESS_MEMORY_COUNTERS_EX pmc = {0};
        if (!GetProcessMemoryInfo(handle, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc)))
        {
//            ("GetProcessMemoryInfo fail, lastErrorCode:%d", GetLastError());
            return -1;
        }
        winSize->pSize = pmc.PeakWorkingSetSize;
        winSize->size = pmc.WorkingSetSize;
        winSize->pVSize = pmc.PeakPagefileUsage;
        winSize->vSize = pmc.PagefileUsage;

        return 0;
    }
#endif

//
void ProcessInfo::run()
{
	_run = true; 
	//
	while (_run)
	{
		// cpu

        static int exceptCnt = 3;
        bool except = false;
        double cpu = getCpuInfo();
//        qInfo() << "cpu used" << cpu;
#if  defined(Q_OS_MAC)
        except = cpu > 15 * 4; //
#elif defined(Q_OS_WIN)
        except = cpu > 15;
#endif
        // memory
#if defined(Q_OS_MAC)
        RunProcDyn m;
		run_get_dynamic_proc_info(&m);
//        qInfo() << QString("resident memory size %1M").arg((double)m.rss / 1024 / 1024);
#elif defined(Q_OS_WIN)
        WinMemory m;
        getMemoryInfo(&m);
//        qInfo() << QString("resident memory size %1M").arg( (double)m.size / 1024 / 1024);
#endif
        if(except) {
            exceptCnt--;
            if(exceptCnt <= 0) {
                QTalkMsgManager::addExceptCpu(cpu, QDateTime::currentMSecsSinceEpoch(), "");
                exceptCnt = 3;
            }
        }
        // sleep
		QThread::msleep(5000);
	}
}