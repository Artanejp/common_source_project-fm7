/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
    Port to Qt : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.10
	History: 2023.02.24 Split from emu_thread_tmpl.cpp
	Note: This class must be compiled per VM, must not integrate shared units.
	[ win32 main ] -> [ Qt main ] -> [Emu Thread] -> [Sequences around emulation]
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

qint64 EmuThreadClassBase::get_interval(void)
{
	#if 0
	if(p_emu == nullptr) return 0;
	double _rate = p_emu->get_frame_rate();
	double nsec = 1.0e6; // 1mS = 1.0e6 nSec.
	__LIKELY_IF(_rate > 0.0) {
		nsec = 1.0e9 / _rate;
	}
	qint64 f_usec = (qint64)llrint(nsec / 1000.0);
	if(half_count) {
		return f_usec / 2;
	} else {
		return (f_usec - (f_usec / 2));
	}
	#else
	if(p_emu == nullptr) return 0;
	double _rate = p_emu->get_frame_rate();
	double nsec = 1.0e6; // 1mS = 1.0e6 nSec.
	__LIKELY_IF(_rate > 0.0) {
		nsec = 1.0e9 / _rate;
	}
	fps_accum += (qint64)(llrint(nsec / 2.0)); // Half emulating.
	qint64 interval = fps_accum / 1000; // By uSec
	fps_accum -= (interval * 1000);
	return interval;
	#endif
}

void EmuThreadClassBase::resetEmu()
{
	clear_key_queue();
	if(p_emu == nullptr) return;
	p_emu->reset();
}

void EmuThreadClassBase::specialResetEmu(int num)
{
	if(p_emu == nullptr) return;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if(p->is_use_special_reset()) {
		p_emu->special_reset(num);
	}
}

void EmuThreadClassBase::loadState()
{
	if(p_emu == nullptr) return;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if(!(p->is_use_state())) return;

	if(!lStateFile.isEmpty()) {
		p_emu->load_state(lStateFile.toLocal8Bit().constData());
		lStateFile.clear();
	}
}

void EmuThreadClassBase::saveState()
{
	if(p_emu == nullptr) return;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if(!(p->is_use_state())) return;

	if(!sStateFile.isEmpty()) {
		p_emu->save_state(sStateFile.toLocal8Bit().constData());
		sStateFile.clear();
	}
}
