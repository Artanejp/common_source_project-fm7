/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.15-

	[ YM2203 / YM2608 ]
*/


#include "ym2612.h"
#include "debugger.h"
#include <math.h>

#define EVENT_FM_TIMER	0

#ifdef SUPPORT_MAME_FM_DLL
// thanks PC8801MA改
#include "fmdll/fmdll.h"
static CFMDLL* fmdll = NULL;
static int chip_reference_counter = 0;
static bool dont_create_multiple_chips = false;
#else
#endif

void YM2612::initialize()
{
	DEVICE::initialize();
	// ToDo: Set type via software interface for AY-3-891x.
	if(this_device_name[0] == _T('\0')) {
		set_device_name(_T("YM2612 OPN2"));
	}
	opn2 = new FM::OPN2;

#ifdef SUPPORT_MAME_FM_DLL
	if(!fmdll) {
		fmdll = new CFMDLL(config.mame2608_dll_path);
	}
	dllchip = NULL;
#endif
	register_vline_event(this);
	left_volume = right_volume = 256;
	v_left_volume = v_right_volume = 256;
	mute = false;
	clock_prev = clock_accum = clock_busy = 0;
	ch = 0;
	addr_A1 = false;

	if(d_debugger != NULL) {
		d_debugger->set_device_name(_T("Debugger (YM2612 OPN2)"));
		d_debugger->set_context_mem(this);
		d_debugger->set_context_io(vm->dummy);
	}
}

void YM2612::release()
{
	delete opn2;
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

void YM2612::reset()
{
	touch_sound();
	opn2->Reset();
#ifdef SUPPORT_MAME_FM_DLL
	if(dllchip) {
		fmdll->Reset(dllchip);
	}
#endif
	memset(port_log, 0, sizeof(port_log));
	fnum2 = 0;
	fnum21 = 0;
	// stop timer
	timer_event_id = -1;
	set_reg(0x27, 0);

	port[0].first = port[1].first = true;
	port[0].wreg = port[1].wreg = 0;//0xff;
	mode = 0;
	irq_prev = busy = false;
}


void YM2612::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 3) {
	case 0:
		// write dummy data for prescaler
		ch = data;
		addr_A1 = false;
		#if 1
		if(0x2d <= ch && ch <= 0x2f) {
			update_count();
			set_reg(ch, 0);
			update_interrupt();
			clock_busy = get_current_clock();
			busy = true;
		}
		#endif
		break;
	case 1:
		if(!(addr_A1)) {
			if(d_debugger != NULL && d_debugger->now_device_debugging) {
				d_debugger->write_via_debugger_data8(ch, data);
			} else {
				this->write_via_debugger_data8(ch, data);
			}
		}
		break;
	case 2:
		ch1 = data1 = data;
		addr_A1 = true;
		break;
	case 3:
		if(addr_A1) {
			if(d_debugger != NULL && d_debugger->now_device_debugging) {
				d_debugger->write_via_debugger_data8(0x100 | ch1, data);
			} else {
				this->write_via_debugger_data8(0x100 | ch1, data);
			}
		}
		break;
	default:
		break;
	}
}

uint32_t YM2612::read_io8(uint32_t addr)
{
	switch(addr & 3) {
	case 0:
	case 1:
	case 2:
	case 3:
		{
				/* BUSY : x : x : x : x : x : FLAGB : FLAGA */
			update_count();
			update_interrupt();
			uint32_t status;
			status = opn2->ReadStatus() & ~0x80;
			if(busy) {
				if(get_passed_usec(clock_busy) < 1.0) {
					status |= 0x80;
				} else {
					busy = false;
				}
			}
			return status;
		}
		break;
	default:
		break;
	}
	return 0xff;
}

void YM2612::write_via_debugger_data8(uint32_t addr, uint32_t data)
{
	if(addr < 0x100) {
		// YM2612
		 if(0x2d <= addr && addr <= 0x2f) {
			// don't write again for prescaler
		} else if(0xa4 <= addr && addr <= 0xa6) {
			// XM8 version 1.20
			fnum2 = data;
		} else {
			update_count();
			// XM8 version 1.20
			if(0xa0 <= addr && addr <= 0xa2) {
				set_reg(addr + 4, fnum2);
			}
			this->set_reg(addr, data);
			if(addr == 0x27) {
				update_event();
			}
			update_interrupt();
			clock_busy = get_current_clock();
			busy = true;
		}
	} else if(addr < 0x200) {
		// YM2608
		if(0x1a4 <= addr && addr <= 0x1a6) {
			// XM8 version 1.20
			fnum21 = data;
		} else {
			update_count();
			// XM8 version 1.20
			if(0x1a0 <= addr && addr <= 0x1a2) {
				this->set_reg(addr + 4, fnum21);
			}
			this->set_reg(addr, data);
			data1 = data;
			update_interrupt();
			clock_busy = get_current_clock();
			busy = true;
		}
	}
}

uint32_t YM2612::read_via_debugger_data8(uint32_t addr)
{
	if(addr < 0x100) {
		// YM2612
		return opn2->GetReg(addr);
	} else if(addr < 0x200) {
		// YM2608
		return port_log[addr].data;
	}
	return 0;
}

void YM2612::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_YM2612_MUTE) {
		mute = ((data & mask) != 0);
	}
}

uint32_t YM2612::read_signal(int id)
{
	return 0x00;
}

void YM2612::event_vline(int v, int clock)
{
	update_count();
	update_interrupt();
}

void YM2612::event_callback(int event_id, int error)
{
	update_count();
	update_interrupt();
	timer_event_id = -1;
	update_event();
}

void YM2612::update_count()
{
	clock_accum += clock_const * get_passed_clock(clock_prev);
	uint32_t count = clock_accum >> 20;
	if(count) {
		opn2->Count(count);
		clock_accum -= count << 20;
	}
	clock_prev = get_current_clock();
}

void YM2612::update_event()
{
	if(timer_event_id != -1) {
		cancel_event(this, timer_event_id);
		timer_event_id = -1;
	}

	int count;
	count = opn2->GetNextEvent();
	if(count > 0) {
		register_event(this, EVENT_FM_TIMER, 1000000.0 / (double)chip_clock * (double)count * 2.0, false, &timer_event_id);
	}
}

void YM2612::update_interrupt()
{
	bool irq;
	irq = opn2->ReadIRQ();
	if(!irq_prev && irq) {
		write_signals(&outputs_irq, 0xffffffff);
	} else if(irq_prev && !irq) {
		write_signals(&outputs_irq, 0);
	}
	irq_prev = irq;
}

#define VCALC(x, y) { \
		x = (x * y) >> 8;						\
	}

#define SATURATION_ADD(x, y) { \
		x = x + y;				 \
		if(x < -0x8000) x = -0x8000;			\
		if(x >  0x7fff) x =  0x7fff;			\
	}

#if 0
static inline __FASTCALL int32_t VCALC(int32_t x, int32_t y)
{
	x = x * y;
	x = x >> 8;
	return x;
}

static inline __FASTCALL int32_t SATURATION_ADD(int32_t x, int32_t y)
{
	x = x + y;
	if(x < -0x8000) x = -0x8000;
	if(x >  0x7fff) x =  0x7fff;
	return x;
}
#endif

void YM2612::mix(int32_t* buffer, int cnt)
{
	if(cnt > 0 && !mute) {
		int32_t *dbuffer = (int32_t *)malloc((cnt * 2 + 2) * sizeof(int32_t));
		memset((void *)dbuffer, 0x00, (cnt * 2 + 2) * sizeof(int32_t));
		opn2->Mix(dbuffer, cnt);
#ifdef SUPPORT_MAME_FM_DLL
		if(dllchip) {
			fmdll->Mix(dllchip, dbuffer, cnt);
		}
#endif
		int32_t *p = dbuffer;
		int32_t *q = buffer;
		__DECL_ALIGNED(32) int32_t tmpp[8];
		__DECL_ALIGNED(32) int32_t tmpq[8];
		__DECL_ALIGNED(32) int32_t tvol[8] =
			{v_left_volume, v_right_volume,
			 v_left_volume, v_right_volume,
			 v_left_volume, v_right_volume,
			 v_left_volume, v_right_volume};

		for(int i = 0; i < cnt / 4; i++) {
__DECL_VECTORIZED_LOOP
			for(int j = 0; j < 8; j++) {
				tmpp[j] = p[j];
				tmpq[j] = q[j];
			}
__DECL_VECTORIZED_LOOP
			for(int j = 0; j < 8; j++) {
				VCALC(tmpp[j], tvol[j]);
			}
__DECL_VECTORIZED_LOOP
			for(int j = 0; j < 8; j++) {
				SATURATION_ADD(tmpq[j], tmpp[j]);
			}
__DECL_VECTORIZED_LOOP
			for(int j = 0; j < 8; j++) {
				q[j] = tmpq[j];
			}
			q += 8;
			p += 8;
		}
		if((cnt & 3) != 0) {
			for(int i = 0; i < (cnt & 3); i++) {
__DECL_VECTORIZED_LOOP
				for(int j = 0; j < 2; j++) {
					tmpp[j] = p[j];
				}
__DECL_VECTORIZED_LOOP
				for(int j = 0; j < 2; j++) {
					VCALC(tmpp[j], tvol[j]);
				}
__DECL_VECTORIZED_LOOP
				for(int j = 0; j < 2; j++) {
					SATURATION_ADD(q[j], tmpp[j]);
				}
				q += 2;
				p += 2;
			}
		}
		free(dbuffer);
	}
}

void YM2612::set_volume(int _ch, int decibel_l, int decibel_r)
{
	__UNLIKELY_IF(decibel_r <= -40) {
		v_right_volume = 0;
	} else {
		v_right_volume = (int)(pow(10.0, (double)decibel_vol / 10.0) * (double)right_volume);
	}
	__UNLIKELY_IF(decibel_l <= -40) {
		v_left_volume = 0;
	} else {
		v_left_volume = (int)(pow(10.0, (double)decibel_vol / 10.0) * (double)left_volume);
	}
	opn2->SetVolumeFM(base_decibel_fm + decibel_l, base_decibel_fm + decibel_r);
#ifdef SUPPORT_MAME_FM_DLL
	if(dllchip) {
		fmdll->SetVolumeFM(dllchip, base_decibel_fm + decibel_l);
	}
#endif
}

void YM2612::initialize_sound(int rate, int clock, int samples, int decibel_fm, int decibel_psg)
{
	// Note: Clock may set to real value, not multiplied by 2.
	opn2->Init(clock, rate, false, get_application_path());
	opn2->SetVolumeFM(decibel_fm, decibel_fm);
	opn2->SetVolumePSG(decibel_psg, decibel_psg);
	base_decibel_fm = decibel_fm;
	base_decibel_psg = decibel_psg;

#ifdef SUPPORT_MAME_FM_DLL
	if(!dont_create_multiple_chips) {
		fmdll->Create((LPVOID*)&dllchip, clock, rate);
		if(dllchip) {
			chip_reference_counter++;

			fmdll->SetVolumeFM(dllchip, decibel_fm);
			fmdll->SetVolumePSG(dllchip, decibel_psg);

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
			if((dwCaps & SUPPORT_PSG) == SUPPORT_PSG) {
				mask |= 0x1c0;
			}
			if((dwCaps & SUPPORT_ADPCM_B) == SUPPORT_ADPCM_B) {
				mask |= 0x200;
			}
			if((dwCaps & SUPPORT_RHYTHM) == SUPPORT_RHYTHM) {
				mask |= 0xfc00;
			}
			opn2->SetChannelMask(mask);
			fmdll->SetChannelMask(dllchip, ~mask);
		}
	}
#endif
	chip_clock = clock;
}

void YM2612::set_reg(uint32_t addr, uint32_t data)
{
	touch_sound();
	if((addr & 0x1f0) <= 0x010) {
		return;
	}
	opn2->SetReg(addr, data);
#ifdef SUPPORT_MAME_FM_DLL
	if(dllchip) {
		fmdll->SetReg(dllchip, addr, data);
	}
	if(0x2d <= addr && addr <= 0x2f) {
		port_log[0x2d].written = port_log[0x2e].written = port_log[0x2f].written = false;
	}
#endif
	port_log[addr].written = true;
	port_log[addr].data = data;
}

void YM2612::update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame)
{
	clock_const = (uint32_t)((double)chip_clock * 1024.0 * 1024.0 / (double)new_clocks + 0.5);
}

bool YM2612::write_debug_reg(const _TCHAR *reg, uint32_t data)
{
	if((reg[0] == 'R') || (reg[0] == 'r')) {
		if(strlen(reg) >= 2) {
			_TCHAR *eptr;
			int regnum = _tcstol(&(reg[1]), &eptr, 16);
			if(regnum < 0x200) {
				set_reg((uint32_t)regnum, data);
			} else {
				return false;
			}
			return true;
		}
		return false;
	} else if(_tcsicmp(reg, _T("CH")) == 0) {
		ch = data;
		return true;
	} else if(_tcsicmp(reg, _T("CH1")) == 0) {
		ch1 = data;
		return true;
	} else if(_tcsicmp(reg, _T("FNUM2")) == 0) {
		fnum2 = data;
		return true;
	} else if(_tcsicmp(reg, _T("DATA1")) == 0) {
		data1 = data;
		return true;
	} else if(_tcsicmp(reg, _T("FNUM21")) == 0) {
		fnum21 = data;
		return true;
	}
	return false;
}

bool YM2612::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	_TCHAR tmps[512] = {0};
	_TCHAR tmps2[32 * 0x200] = {0};
	_TCHAR tmps3[16];
	int rows = 0x200 / 16;
	for(uint32_t i = 0; i < rows; i++) {
		memset(tmps3, 0x00, sizeof(tmps3));
		my_stprintf_s(tmps3, 15, _T("+%02X :"), i * 16);
		_tcsncat(tmps2, tmps3, sizeof(tmps2) - 1);
		for(uint32_t j = 0; j < 16; j++) {
			memset(tmps3, 0x00, sizeof(tmps3));
			if(i == 0) {
				my_stprintf_s(tmps3, 7, _T(" %02X"), opn2->GetReg(j));
			} else {
				if((i * 16 + j) == 0xff) {
					my_stprintf_s(tmps3, 7, _T(" %02X"), opn2->GetReg(i * 16 + j));
				} else {
					my_stprintf_s(tmps3, 7, _T(" %02X"), port_log[i * 16 + j].data);
				}
			}
			_tcsncat(tmps2, tmps3, sizeof(tmps2) - 1);
		}
		_tcsncat(tmps2, "\n", sizeof(tmps2) - 1);
	}
	bool irqflag;
	irqflag = opn2->ReadIRQ();
	my_stprintf_s(buffer, buffer_len - 1, _T("%sCH=%02X  FNUM2=%02X CH1=%02X DATA1=%02X FNUM21=%02X\nPRESCALER=%d IRQ=%s BUSY=%s CHIP_CLOCK=%d\nREG : +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F\n%s"),
				  tmps, ch, fnum2, ch1, data1, fnum21,
				  opn2->GetPrescaler(), (irqflag) ? _T("Y") : _T("N"), (busy) ? _T("Y") : _T("N"), chip_clock, tmps2);
	return true;
}

#define STATE_VERSION	1

bool YM2612::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}

	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
 	}
	if(!opn2->ProcessState((void *)state_fio, loading)) {
		return false;
	}
	for(int i = 0; i < array_length(port_log); i++) {
		state_fio->StateValue(port_log[i].written);
		state_fio->StateValue(port_log[i].data);
	}
	state_fio->StateValue(ch);
	state_fio->StateValue(fnum2);
	state_fio->StateValue(ch1);
	state_fio->StateValue(data1);
	state_fio->StateValue(fnum21);
 	for(int i = 0; i < 2; i++) {
		state_fio->StateValue(port[i].wreg);
		state_fio->StateValue(port[i].rreg);
		state_fio->StateValue(port[i].first);
 	}
	state_fio->StateValue(mode);
	state_fio->StateValue(chip_clock);
	state_fio->StateValue(irq_prev);
	state_fio->StateValue(mute);
	state_fio->StateValue(clock_prev);
	state_fio->StateValue(clock_accum);
	state_fio->StateValue(clock_const);
	state_fio->StateValue(clock_busy);
	state_fio->StateValue(timer_event_id);
	state_fio->StateValue(busy);
	state_fio->StateValue(addr_A1);

#ifdef SUPPORT_MAME_FM_DLL
 	// post process
	if(loading && dllchip) {
		fmdll->Reset(dllchip);
		for(int i = 0; i < 0x200; i++) {
			// write fnum2 before fnum1
			int _ch = ((i >= 0xa0 && i <= 0xaf) || (i >= 0x1a0 && i <= 0x1a7)) ? (i ^ 4) : i;
			if(port_log[ch].written) {
				fmdll->SetReg(dllchip, _ch, port_log[ch].data);
			}
		}
	}
#endif
	return true;
}
