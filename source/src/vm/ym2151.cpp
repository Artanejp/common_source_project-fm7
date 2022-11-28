/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2009.03.08-

	[ YM2151 ]
*/

#include "ym2151.h"
#ifdef USE_DEBUGGER
#include "debugger.h"
#endif

#define EVENT_FM_TIMER	0

#ifdef SUPPORT_MAME_FM_DLL
// thanks PC8801MA‰ü
#include "fmdll/fmdll.h"
static CFMDLL* fmdll = NULL;
static int chip_reference_counter = 0;
static bool dont_create_multiple_chips = false;
#endif

void YM2151::initialize()
{
	opm = new FM::OPM;
#ifdef SUPPORT_MAME_FM_DLL
	if(!fmdll) {
//		fmdll = new CFMDLL(_T("mame2151.dll"));
		fmdll = new CFMDLL(config.mame2151_dll_path);
	}
	dllchip = NULL;
#endif
	register_vline_event(this);
	mute = false;
	clock_prev = clock_accum = clock_busy = 0;
	
#ifdef USE_DEBUGGER
	if(d_debugger != NULL) {
		d_debugger->set_device_name(_T("Debugger (YM2151 OPM)"));
		d_debugger->set_context_mem(this);
		d_debugger->set_context_io(vm->dummy);
	}
#endif
}

void YM2151::release()
{
	delete opm;
#ifdef SUPPORT_MAME_FM_DLL
	if(dllchip) {
		fmdll->Release(dllchip);
		dllchip = NULL;
		chip_reference_counter--;
	}
	if(fmdll && !chip_reference_counter) {
		delete fmdll;
		fmdll = NULL;
	}
#endif
}

void YM2151::reset()
{
	touch_sound();
	opm->Reset();
#ifdef SUPPORT_MAME_FM_DLL
	if(dllchip) {
		fmdll->Reset(dllchip);
	}
#endif
	memset(port_log, 0, sizeof(port_log));
	timer_event_id = -1;
	irq_prev = busy = false;
}

void YM2151::write_io8(uint32_t addr, uint32_t data)
{
	if(addr & 1) {
#ifdef USE_DEBUGGER
		if(d_debugger != NULL && d_debugger->now_device_debugging) {
			d_debugger->write_via_debugger_data8(ch, data);
		} else
#endif
		this->write_via_debugger_data8(ch, data);
	} else {
		ch = data;
	}
}

uint32_t YM2151::read_io8(uint32_t addr)
{
	if(addr & 1) {
		update_count();
		update_interrupt();
		uint32_t status = opm->ReadStatus() & ~0x80;
		if(busy) {
			// FIXME: we need to investigate the correct busy period
			if(get_passed_usec(clock_busy) < 8) {
				status |= 0x80;
			}
			busy = false;
		}
		return status;
	}
	return 0xff;
}

void YM2151::write_via_debugger_data8(uint32_t addr, uint32_t data)
{
	if(addr < 0x100) {
		update_count();
		this->set_reg(addr, data);
		if(addr == 0x14) {
			update_event();
		}
		update_interrupt();
		clock_busy = get_current_clock();
		busy = true;
	}
}

uint32_t YM2151::read_via_debugger_data8(uint32_t addr)
{
	if(addr < 0x100) {
		return port_log[addr].data;
	}
	return 0;
}

void YM2151::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_YM2151_MUTE) {
		mute = ((data & mask) != 0);
	}
}

void YM2151::event_vline(int v, int clock)
{
	update_count();
	update_interrupt();
}

void YM2151::event_callback(int event_id, int error)
{
	update_count();
	update_interrupt();
	timer_event_id = -1;
	update_event();
}

void YM2151::update_count()
{
	clock_accum += clock_const * get_passed_clock(clock_prev);
	uint32_t count = clock_accum >> 20;
	if(count) {
		opm->Count(count);
		clock_accum -= count << 20;
	}
	clock_prev = get_current_clock();
}

void YM2151::update_event()
{
	if(timer_event_id != -1) {
		cancel_event(this, timer_event_id);
		timer_event_id = -1;
	}
	
	int count = opm->GetNextEvent();
	if(count > 0) {
		register_event(this, EVENT_FM_TIMER, 1000000.0 / (double)chip_clock * (double)count, false, &timer_event_id);
	}
}

void YM2151::update_interrupt()
{
	bool irq = opm->ReadIRQ();
	if(!irq_prev && irq) {
		write_signals(&outputs_irq, 0xffffffff);
	} else if(irq_prev && !irq) {
		write_signals(&outputs_irq, 0);
	}
	irq_prev = irq;
}

void YM2151::mix(int32_t* buffer, int cnt)
{
	if(cnt > 0 && !mute) {
		opm->Mix(buffer, cnt);
#ifdef SUPPORT_MAME_FM_DLL
		if(dllchip) {
			fmdll->Mix(dllchip, buffer, cnt);
		}
#endif
	}
}

void YM2151::set_volume(int ch, int decibel_l, int decibel_r)
{
	opm->SetVolume(base_decibel + decibel_l, base_decibel + decibel_r);
#ifdef SUPPORT_MAME_FM_DLL
	if(dllchip) {
		fmdll->SetVolumeFM(dllchip, base_decibel + decibel_l);
	}
#endif
}

void YM2151::initialize_sound(int rate, int clock, int samples, int decibel)
{
	opm->Init(clock, rate, false);
	opm->SetVolume(decibel, decibel);
	base_decibel = decibel;
	
#ifdef SUPPORT_MAME_FM_DLL
	if(!dont_create_multiple_chips) {
		fmdll->Create((LPVOID*)&dllchip, clock, rate);
		if(dllchip) {
			chip_reference_counter++;
			
			fmdll->SetVolumeFM(dllchip, decibel);
			
			DWORD mask = 0;
			DWORD dwCaps = fmdll->GetCaps(dllchip);
			if((dwCaps & SUPPORT_MULTIPLE) != SUPPORT_MULTIPLE) {
				dont_create_multiple_chips = true;
			}
			if((dwCaps & SUPPORT_FM_A) == SUPPORT_FM_A) {
				mask = 0x07;
			}
			if((dwCaps & SUPPORT_FM_B) == SUPPORT_FM_B) {
				mask |= 0x38;
			}
			if((dwCaps & SUPPORT_FM_C) == SUPPORT_FM_C) {
				mask |= 0xc0;
			}
			opm->SetChannelMask(mask);
			fmdll->SetChannelMask(dllchip, ~mask);
		}
	}
#endif
	
	chip_clock = clock;
}

void YM2151::set_reg(uint32_t addr, uint32_t data)
{
	touch_sound();
	opm->SetReg(addr, data);
#ifdef SUPPORT_MAME_FM_DLL
	if(dllchip) {
		fmdll->SetReg(dllchip, addr, data);
	}
#endif
	port_log[addr].written = true;
	port_log[addr].data = data;
}

void YM2151::update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame)
{
	clock_const = (uint32_t)((double)chip_clock * 1024.0 * 1024.0 / (double)new_clocks + 0.5);
}

#define STATE_VERSION	4

bool YM2151::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	if(!opm->ProcessState((void *)state_fio, loading)) {
		return false;
	}
	for(int i = 0; i < array_length(port_log); i++) {
		state_fio->StateValue(port_log[i].written);
		state_fio->StateValue(port_log[i].data);
	}
	state_fio->StateValue(chip_clock);
	state_fio->StateValue(ch);
	state_fio->StateValue(irq_prev);
	state_fio->StateValue(mute);
	state_fio->StateValue(clock_prev);
	state_fio->StateValue(clock_accum);
	state_fio->StateValue(clock_const);
	state_fio->StateValue(clock_busy);
	state_fio->StateValue(timer_event_id);
	state_fio->StateValue(busy);
	
#ifdef SUPPORT_MAME_FM_DLL
	// post process
	if(loading && dllchip) {
		fmdll->Reset(dllchip);
		for(int i = 0; i < 0x100; i++) {
			if(port_log[i].written) {
				fmdll->SetReg(dllchip, i, port_log[i].data);
			}
		}
	}
#endif
	return true;
}

