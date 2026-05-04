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
	int64_t _frame_interval_us = (int64_t)((1.0e6 / 59.94) + 0.5);
	__LIKELY_IF(p_emu != nullptr) {
		_frame_interval_us = p_emu->get_frame_interval();
	}
	if(m_driven_by_half_of_frame) {
		if(!(m_half_count)) { // TOP of FRAME
			_frame_interval_us = _frame_interval_us / 2;
		} else {
			// MIDDLE of FRAME
			_frame_interval_us = _frame_interval_us - (_frame_interval_us / 2);
		}
	}
	m_fps_accum += _frame_interval_us;
	int64_t _interval = m_fps_accum >> 10;
	m_fps_accum -= (_interval << 10);
	return _interval;
}

void EmuThreadClassBase::reset_emulation_values()
{
	m_nr_fps = get_emu_frame_rate();
	emit sig_set_draw_fps(m_nr_fps);
	m_req_draw = true;

	// Maybe re-check now_skip : 20260504 K.O
	m_now_skip = (((m_full_speed) || p_emu->is_frame_skippable()) && !(p_emu->is_video_recording())) ? true : false;
	
	m_tick_timer.restart();
	m_current_time = get_current_tick_usec();
	m_update_fps_time = m_current_time + (1000 * 1000); // At least per a second.
	
	if(p_emu != NULL) {
		m_half_count = p_emu->is_half_event();
		m_driven_by_half_of_frame = p_emu->is_driven_by_half_of_frame();
	} else {
		m_half_count = false;
		m_driven_by_half_of_frame = false;
	}
	m_fps_accum = 0; 
	int64_t _interval = get_interval();
	__UNLIKELY_IF((_interval < 0) || (m_now_skip)) {
		_interval = 0;
	}
	m_next_time = m_current_time + _interval;
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

	if(!m_load_state_file_name.isEmpty()) {
		if(FILEIO::IsFileExisting(m_load_state_file_name.toLocal8Bit().constData())) {
			p_emu->load_state(m_load_state_file_name.toLocal8Bit().constData());
			m_load_state_file_name.clear();
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

	if(!m_save_state_file_name.isEmpty()) {
		p_emu->save_state(m_save_state_file_name.toLocal8Bit().constData());
		m_save_state_file_name.clear();
	}
}
