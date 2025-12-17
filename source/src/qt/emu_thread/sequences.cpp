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
#include "fileio.h"

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
//	std::shared_ptr<USING_FLAGS> p_flags = using_flags;
	#if 0
	qint64 _interval = (qint64)(1.0e6 / 59.94);
	qint64 _nsec;
	__LIKELY_IF(p_emu != nullptr) {
		_nsec = p_emu->get_next_period_nsec();
		__UNLIKELY_IF(_nsec >= (2000 * 1000 * 1000)) { // Maximum 2 Sec.
			_nsec = 2000 * 1000 * 1000;
		} else if(_nsec < (1000 * 100)) { // Minimum 100uS (!)
			_nsec = 1000 * 100; 
		}
		qint64 _usec = (qint64)((((double)_nsec) / 1000.0) + 0.5); // nSec -> uSec .
		_usec <<= 10;
		fps_accum += _usec;
		_interval = fps_accum >> 10;
		fps_accum -= (_interval << 10);
	}
	#else
	int64_t _frame_interval_us = (int64_t)((1.0e6 / 59.94) + 0.5);
	__LIKELY_IF(p_emu != nullptr) {
		_frame_interval_us = p_emu->get_frame_interval();
	}
	if(driven_by_half_of_frame) {
		if(!(half_count)) { // TOP of FRAME
			_frame_interval_us = _frame_interval_us / 2;
		} else {
			// MIDDLE of FRAME
			_frame_interval_us = _frame_interval_us - (_frame_interval_us / 2);
		}
	}
	fps_accum += _frame_interval_us;
	qint64 _interval = (qint64)(fps_accum >> 10);
	fps_accum -= (_interval << 10);
	#endif
	return _interval;
}

void EmuThreadClassBase::reset_emulation_values()
{
	nr_fps = get_emu_frame_rate();
	emit sig_set_draw_fps(nr_fps);
	req_draw = true;

	int64_t _interval = get_interval();
	__UNLIKELY_IF((_interval < 0) || (now_skip)) {
		_interval = 0;
	}
	tick_timer.restart();
	update_fps_time = get_current_tick_usec() + (1000 * 1000);
	current_time = get_current_tick_usec();
	next_time = current_time + _interval;
	
	if(p_emu != NULL) {
		half_count = p_emu->is_half_event();
		driven_by_half_of_frame = p_emu->is_driven_by_half_of_frame();
	}
}

void EmuThreadClassBase::resetEmu()
{
	clear_key_queue();
	if(p_emu == nullptr) return;
	p_emu->reset();
	reset_emulation_values();
}

void EmuThreadClassBase::specialResetEmu(int num)
{
	if(p_emu == nullptr) return;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if(p->is_use_special_reset()) {
		p_emu->special_reset(num);
		reset_emulation_values();
	}
}

void EmuThreadClassBase::loadState()
{
	if(p_emu == nullptr) return;
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;

	if(!(p->is_use_state())) return;

	if(!lStateFile.isEmpty()) {
		if(FILEIO::IsFileExisting(lStateFile.toLocal8Bit().constData())) {
			p_emu->load_state(lStateFile.toLocal8Bit().constData());
			lStateFile.clear();
			reset_emulation_values();
		}
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
