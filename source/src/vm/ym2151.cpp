/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2009.03.08-

	[ YM2151 ]
*/

#include "ym2151.h"

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
	DEVICE::initialize();
	opm = new FM::OPM;
#ifdef SUPPORT_MAME_FM_DLL
	if(!fmdll) {
//		fmdll = new CFMDLL(_T("mamefm.dll"));
		fmdll = new CFMDLL(config.fmgen_dll_path);
	}
	dllchip = NULL;
#endif
	register_vline_event(this);
	mute = false;
	clock_prev = clock_accum = clock_busy = 0;
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
	memset(port_log, 0, sizeof(port_log));
#endif
	timer_event_id = -1;
	irq_prev = busy = false;
}

void YM2151::write_io8(uint32_t addr, uint32_t data)
{
	if(addr & 1) {
		update_count();
		this->set_reg(ch, data);
		if(ch == 0x14) {
			update_event();
		}
		update_interrupt();
		clock_busy = get_current_clock();
		busy = true;
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
	port_log[addr].written = true;
	port_log[addr].data = data;
#endif
}

void YM2151::update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame)
{
	clock_const = (uint32_t)((double)chip_clock * 1024.0 * 1024.0 / (double)new_clocks + 0.5);
}

#define STATE_VERSION	4

#include "../statesub.h"

void YM2151::decl_state()
{
	enter_decl_state(STATE_VERSION);
#ifdef SUPPORT_MAME_FM_DLL
	for(int i = 0; i < 0x100; i++) {
		DECL_STATE_ENTRY_BOOL_MEMBER((port_log[i].written), i);
		DECL_STATE_ENTRY_UINT8_MEMBER((port_log[i].data), i);
	}
#endif
	DECL_STATE_ENTRY_INT32(chip_clock);
	DECL_STATE_ENTRY_UINT8(ch);
	DECL_STATE_ENTRY_BOOL(irq_prev);
	DECL_STATE_ENTRY_BOOL(mute);
	DECL_STATE_ENTRY_UINT32(clock_prev);
	DECL_STATE_ENTRY_UINT32(clock_accum);
	DECL_STATE_ENTRY_UINT32(clock_const);
	DECL_STATE_ENTRY_UINT32(clock_busy);
	DECL_STATE_ENTRY_INT32(timer_event_id);
	DECL_STATE_ENTRY_BOOL(busy);

	leave_decl_state();

	opm->DeclState();
}
void YM2151::save_state(FILEIO* state_fio)
{
	//state_fio->FputUint32(STATE_VERSION);
	//state_fio->FputInt32(this_device_id);
	
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
	opm->SaveState((void *)state_fio);
//#ifdef SUPPORT_MAME_FM_DLL
//	state_fio->Fwrite(port_log, sizeof(port_log), 1);
//#endif
//	state_fio->FputInt32(chip_clock);
//	state_fio->FputUint8(ch);
//	state_fio->FputBool(irq_prev);
//	state_fio->FputBool(mute);
//	state_fio->FputUint32(clock_prev);
//	state_fio->FputUint32(clock_accum);
//	state_fio->FputUint32(clock_const);
//	state_fio->FputUint32(clock_busy);
//	state_fio->FputInt32(timer_event_id);
//	state_fio->FputBool(busy);
}

bool YM2151::load_state(FILEIO* state_fio)
{
//	if(state_fio->FgetUint32() != STATE_VERSION) {
//		return false;
//	}
//	if(state_fio->FgetInt32() != this_device_id) {
//		return false;
//	}
	bool mb = false;
	if(state_entry != NULL) {
		mb = state_entry->load_state(state_fio);
	}
	if(!mb) return false;

	if(!opm->LoadState((void *)state_fio)) {
		return false;
	}
//#ifdef SUPPORT_MAME_FM_DLL
//	state_fio->Fread(port_log, sizeof(port_log), 1);
//#endif
//	chip_clock = state_fio->FgetInt32();
//	ch = state_fio->FgetUint8();
//	irq_prev = state_fio->FgetBool();
//	mute = state_fio->FgetBool();
//	clock_prev = state_fio->FgetUint32();
//	clock_accum = state_fio->FgetUint32();
//	clock_const = state_fio->FgetUint32();
//	clock_busy = state_fio->FgetUint32();
//	timer_event_id = state_fio->FgetInt32();
//	busy = state_fio->FgetBool();
	
#ifdef SUPPORT_MAME_FM_DLL
	// post process
	if(dllchip) {
		fmdll->Reset(dllchip);
		for(int i = 0; i < 0x100; i++) {
			if(port_log[i].written) {
				fmdll->SetReg(dllchip, i, port_log[i].data);
			}
		}
	}
#endif
	//touch_sound();
	return true;
}

