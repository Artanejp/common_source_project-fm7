/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2009.03.08-

	[ YM2151 ]
*/

#include "ym2151.h"
#include "../fileio.h"

void YM2151::initialize()
{
	opm = new FM::OPM;
	register_vline_event(this);
	mute = false;
	clock_prev = clock_accum = clock_busy = 0;
}

void YM2151::release()
{
	delete opm;
}

void YM2151::reset()
{
	opm->Reset();
	irq_prev = busy = false;
}

void YM2151::write_io8(uint32 addr, uint32 data)
{
	if(addr & 1) {
		update_count();
		opm->SetReg(ch, data);
		update_interrupt();
		clock_busy = current_clock();
		busy = true;
	} else {
		ch = data;
	}
}

uint32 YM2151::read_io8(uint32 addr)
{
	if(addr & 1) {
		update_count();
		update_interrupt();
		uint32 status = opm->ReadStatus() & ~0x80;
		if(busy) {
			// FIXME: we need to investigate the correct busy period
			if(passed_usec(clock_busy) < 8) {
				status |= 0x80;
			}
			busy = false;
		}
		return status;
	}
	return 0xff;
}

void YM2151::write_signal(int id, uint32 data, uint32 mask)
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

void YM2151::update_count()
{
	clock_accum += clock_const * passed_clock(clock_prev);
	uint32 count = clock_accum >> 20;
	if(count) {
		opm->Count(count);
		clock_accum -= count << 20;
	}
	clock_prev = current_clock();
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

void YM2151::mix(int32* buffer, int cnt)
{
	if(cnt > 0 && !mute) {
		opm->Mix(buffer, cnt);
	}
}

void YM2151::init(int rate, int clock, int samples, int vol)
{
	opm->Init(clock, rate, false);
	opm->SetVolume(vol);
	
	chip_clock = clock;
}

void YM2151::SetReg(uint addr, uint data)
{
	opm->SetReg(addr, data);
}

void YM2151::update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame)
{
	clock_const = (uint32)((double)chip_clock * 1024.0 * 1024.0 / (double)new_clocks + 0.5);
}

#define STATE_VERSION	1

void YM2151::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	opm->SaveState((void *)state_fio);
	state_fio->FputInt32(chip_clock);
	state_fio->FputUint8(ch);
	state_fio->FputBool(irq_prev);
	state_fio->FputBool(mute);
	state_fio->FputUint32(clock_prev);
	state_fio->FputUint32(clock_accum);
	state_fio->FputUint32(clock_const);
	state_fio->FputUint32(clock_busy);
	state_fio->FputBool(busy);
}

bool YM2151::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	if(!opm->LoadState((void *)state_fio)) {
		return false;
	}
	chip_clock = state_fio->FgetInt32();
	ch = state_fio->FgetUint8();
	irq_prev = state_fio->FgetBool();
	mute = state_fio->FgetBool();
	clock_prev = state_fio->FgetUint32();
	clock_accum = state_fio->FgetUint32();
	clock_const = state_fio->FgetUint32();
	clock_busy = state_fio->FgetUint32();
	busy = state_fio->FgetBool();
	return true;
}

