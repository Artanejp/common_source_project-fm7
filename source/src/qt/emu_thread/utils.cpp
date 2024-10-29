/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
    Port to Qt : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.10
	History: 2023.02.24 Split from emu_thread_tmpl.cpp
	Note: This class must be compiled per VM, must not integrate shared units.
	[ win32 main ] -> [ Qt main ] -> [Emu Thread] -> [Utilities]
*/

#include <QWidget>

#include "config.h"
#include "emu_template.h"
#include "emu_thread_tmpl.h"
#include "mainwidget_base.h"
#include "common.h"
#include "../../osdcall_types.h"

#include "virtualfileslist.h"
#include "menu_metaclass.h"

#include "menu_flags.h"

void EmuThreadClassBase::calc_volume_from_balance(int num, int balance)
{
	int level = volume_avg[num].load();
	int right;
	int left;
	volume_balance[num] = balance;
	right = level + balance;
	left  = level - balance;
	p_config->sound_volume_l[num] = left;
	p_config->sound_volume_r[num] = right;
}

void EmuThreadClassBase::calc_volume_from_level(int num, int level)
{
	int balance = volume_balance[num].load();
	int right,left;
	volume_avg[num] = level;
	right = level + balance;
	left  = level - balance;
	p_config->sound_volume_l[num] = left;
	p_config->sound_volume_r[num] = right;
}


const _TCHAR *EmuThreadClassBase::get_device_name(void)
{
	return (const _TCHAR *)_T("TEST");
}


#if defined(Q_OS_LINUX)
//#define _GNU_SOURCE
#include <unistd.h>
#include <sched.h>
#include <pthread.h>
#endif

void EmuThreadClassBase::do_set_emu_thread_to_fixed_cpu_from_action(void)
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int cpunum = cp->data().value<int>();
	do_set_emu_thread_to_fixed_cpu(cpunum);
}

void EmuThreadClassBase::do_set_emu_thread_to_fixed_cpu(int cpunum)
{
	if(thread_id == (Qt::HANDLE)nullptr) {
		return;
	}
#if defined(Q_OS_LINUX)
	long cpus = sysconf(_SC_NPROCESSORS_ONLN);
	__UNLIKELY_IF(cpus <= 0) {
		return;
	}
	__UNLIKELY_IF(cpus <= cpunum) {
		return;
	}
	cpu_set_t *mask;
	mask = CPU_ALLOC(cpus);
	size_t bytes = CPU_ALLOC_SIZE(cpus);
	CPU_ZERO_S(bytes, mask);
	
	if(cpunum < 0) {
		for(int i = 0; i < cpus; i++) {
			CPU_SET_S(i, bytes , mask);
		}
	} else {
		CPU_SET_S(cpunum, bytes , mask);
	}
	pthread_setaffinity_np(*((pthread_t*)thread_id), bytes, (const cpu_set_t *)mask);
	CPU_FREE(mask);
#endif
}

void EmuThreadClassBase::do_append_cpu_to_emu_thread(unsigned int cpunum)
{
	if(thread_id == (Qt::HANDLE)nullptr) {
		return;
	}
#if defined(Q_OS_LINUX)
	long cpus = sysconf(_SC_NPROCESSORS_ONLN);
	
	__UNLIKELY_IF(cpus <= 0) {
		return;
	}
	__UNLIKELY_IF(cpunum > INT_MAX) {
		return;
	}
	__UNLIKELY_IF(((unsigned int)cpus) <= cpunum) {
		return;
	}
	cpu_set_t *mask;
	mask = CPU_ALLOC(cpus);
	size_t bytes = CPU_ALLOC_SIZE(cpus);
	
	pthread_getaffinity_np(*((pthread_t*)thread_id), bytes, mask);
	if(CPU_ISSET_S(cpunum, bytes, mask) == 0) {
		CPU_SET_S(cpunum, bytes, mask);
		pthread_setaffinity_np(*((pthread_t*)thread_id), bytes, (const cpu_set_t *)mask);
	}
	CPU_FREE(mask);
#endif
}

void EmuThreadClassBase::do_remove_cpu_to_emu_thread(unsigned int cpunum)
{
	if(thread_id == (Qt::HANDLE)nullptr) {
		return;
	}
#if defined(Q_OS_LINUX)
	long cpus = sysconf(_SC_NPROCESSORS_ONLN);
	__UNLIKELY_IF(cpus <= 0) {
		return;
	}
	__UNLIKELY_IF(cpunum > INT_MAX) {
		return;
	}
	__UNLIKELY_IF(((unsigned int)cpus) <= cpunum) {
		return;
	}
	cpu_set_t *mask;
	mask = CPU_ALLOC(cpus);
	size_t bytes = CPU_ALLOC_SIZE(cpus);
	
	pthread_getaffinity_np(*((pthread_t*)thread_id), bytes, mask);
	if(CPU_COUNT(mask) <= 1) {
		// At least one CPU. 
		CPU_FREE(mask);
		return;
	}

	if(CPU_ISSET_S(cpunum, bytes, mask) != 0) {
		CPU_CLR_S(cpunum, bytes, mask);
		pthread_setaffinity_np(*((pthread_t*)thread_id), bytes, (const cpu_set_t *)mask);
	}
	CPU_FREE(mask);
#endif
}

void EmuThreadClassBase::do_apply_cpu_affinities_to_emu_thread()
{
	if(queue_cpu_affinities.empty()) {
		return;
	}
	if(thread_id == (Qt::HANDLE)nullptr) {
		return;
	}
	
#if defined(Q_OS_LINUX)
	long cpus = sysconf(_SC_NPROCESSORS_ONLN);
	if(cpus <= 0) {
		queue_cpu_affinities.clear();
		return;
	}
#endif
	// ToDo: Set one operation.
	for(auto p = queue_cpu_affinities.begin(); p != queue_cpu_affinities.end(); ++p) {
		__UNLIKELY_IF((*p).first == CPU_SET_ALL) {
			do_set_emu_thread_to_fixed_cpu(INT_MIN);
		} else if((*p).first == CPU_SET_BIT) {
			do_append_cpu_to_emu_thread((*p).second);
		} else if((*p).first == CPU_CLEAR_BIT) {
			do_remove_cpu_to_emu_thread((*p).second);
		} else if((*p).first == CPU_SET_FIXED) {
			if((*p).second <= ((unsigned int)INT_MAX)) {
				do_set_emu_thread_to_fixed_cpu((int)((*p).second));
			}
		}
	}
	queue_cpu_affinities.clear();

}
