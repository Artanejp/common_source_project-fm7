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
	qint64 interval = 0;
	__LIKELY_IF(p_emu != nullptr) {
		if(!(half_count)) {
			fps_accum += p_emu->get_frame_interval();
		}
		interval = (fps_accum >> 10) / 2;
		fps_accum -= (interval << 10);
	}
	return interval;
}

void EmuThreadClassBase::reset_emulation_values()
{
	nr_fps = get_emu_frame_rate();
	emit sig_set_draw_fps(nr_fps);
	req_draw = true;
	next_time = 0;
	tick_timer.restart();
	update_fps_time = get_current_tick_usec() + (1000 * 1000);
	current_time = get_current_tick_usec();
	
	if(p_emu != nullptr) {
		half_count = p_emu->is_half_event();
	} else {
		half_count = false;
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
