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

int EmuThreadClassBase::get_interval(void)
{
	static int64_t accum = 0;
	if(p_emu == nullptr) return 0;
	accum += (p_emu->get_frame_interval() / 2);
	int interval = accum >> 10;
	accum -= interval << 10;
	return interval;
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
