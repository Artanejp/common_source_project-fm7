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
#if defined(Q_OS_LINUX)
	if(thread_id == (Qt::HANDLE)nullptr) {
		queue_fixed_cpu = cpunum;
		return;
	}

	long cpus = sysconf(_SC_NPROCESSORS_ONLN);
	cpu_set_t *mask;
	mask = CPU_ALLOC(cpus);
	CPU_ZERO_S(CPU_ALLOC_SIZE(cpus), mask);
	if((cpunum < 0) || (cpunum >= cpus)) {
		for(int i = 0; i < cpus; i++ ) {
			CPU_SET(i, mask);
		}
	} else {
		CPU_SET(cpunum, mask);
	}
//	sched_setaffinity((pid_t)thread_id, CPU_ALLOC_SIZE(cpus), (const cpu_set_t*)mask);
	pthread_setaffinity_np(*((pthread_t*)thread_id), CPU_ALLOC_SIZE(cpus),(const cpu_set_t *)mask);
	CPU_FREE(mask);
#else
	return;
#endif
	return;
}
