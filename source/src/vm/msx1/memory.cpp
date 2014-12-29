/*
	Pioneer PX-7 Emulator 'ePX-7'

	Author : Takeda.Toshiya
	Date   : 2014.01.09-

	modified by umaiboux

	[ memory ]
*/

#include "memory.h"
#include "../ld700.h"
#include "../tms9918a.h"
#include "../../fileio.h"

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 13, eb = (e) >> 13; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x2000 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x2000 * (i - sb); \
		} \
	} \
}

// slot #0

void SLOT_MAIN::initialize()
{
	memset(rom, 0xff, sizeof(rom));
	memset(ram, 0, sizeof(ram));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("BASIC.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
	delete fio;
	
	SET_BANK(0x0000, 0x7fff, wdmy, rom);
	SET_BANK(0x8000, 0xffff, ram, ram);
}

void SLOT_MAIN::write_data8(uint32 addr, uint32 data)
{
	wbank[addr >> 13][addr & 0x1fff] = data;
}

uint32 SLOT_MAIN::read_data8(uint32 addr)
{
	return rbank[addr >> 13][addr & 0x1fff];
}

// slot #2

#define EVENT_CLOCK	0

void SLOT_SUB::initialize()
{
	memset(rom, 0xff, sizeof(rom));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("EXT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
	delete fio;
	
	SET_BANK(0x0000, 0x3fff, wdmy, rdmy);
	SET_BANK(0x4000, 0x5fff, wdmy, rom);
	SET_BANK(0x6000, 0xffff, wdmy, rdmy);
	
	clock = exv = ack = false;
	
	register_event(this, EVENT_CLOCK, 1000000.0 / 7812.5, true, NULL);
}

void SLOT_SUB::reset()
{
	super_impose = false;
	req_intr = false;
	pc4 = false;
	mute_l = mute_r = true;
	
	d_ldp->write_signal(SIG_LD700_MUTE_L, 1, 1);
	d_ldp->write_signal(SIG_LD700_MUTE_R, 1, 1);
	d_vdp->write_signal(SIG_TMS9918A_SUPER_IMPOSE, 0, 0);
}

void SLOT_SUB::write_data8(uint32 addr, uint32 data)
{
	if(addr == 0x7ffe) {
		d_ldp->write_signal(SIG_LD700_REMOTE, data, 1);
	} else if(addr == 0x7fff) {
		// super impose
		bool prev_super_impose = super_impose;
		super_impose = ((data & 1) == 0);
		if(super_impose) {
			if(req_intr && !prev_super_impose) {
				d_cpu->write_signal(SIG_CPU_IRQ, 1, 1);
			}
		} else {
			d_cpu->write_signal(SIG_CPU_IRQ, 0, 0);
		}
		d_vdp->write_signal(SIG_TMS9918A_SUPER_IMPOSE, super_impose ? 1 : 0, 1);
		
		// mute
		bool prev_mute_l = mute_l;
		mute_l = ((data & 0x80) == 0);
		if(!prev_mute_l && mute_l) {
			mute_r = !pc4;
		}
		d_ldp->write_signal(SIG_LD700_MUTE_L, mute_l ? 1 : 0, 1);
		d_ldp->write_signal(SIG_LD700_MUTE_R, mute_r ? 1 : 0, 1);
	} else {
		wbank[addr >> 13][addr & 0x1fff] = data;
	}
}

uint32 SLOT_SUB::read_data8(uint32 addr)
{
	if(addr == 0x7ffe) {
		return (clock ? 0 : 1) | (ack ? 0 : 0x80) | 0x7e;
	} else if(addr == 0x7fff) {
		uint32 data = (req_intr ? 1 : 0) | (exv ? 0 : 0x80) | 0x7e;
		req_intr = false;
		d_cpu->write_signal(SIG_CPU_IRQ, 0, 0);
		return data;
	} else {
		return rbank[addr >> 13][addr & 0x1fff];
	}
}

void SLOT_SUB::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_SLOT2_EXV) {
		bool prev = exv;
		exv = ((data & mask) != 0);
		if(prev && !exv) {
			req_intr = true;
			if(super_impose) {
				d_cpu->write_signal(SIG_CPU_IRQ, 1, 1);
			}
		}
	} else if(id == SIG_SLOT2_ACK) {
		ack = ((data & mask) != 0);
	} else if(id == SIG_SLOT2_MUTE) {
		pc4 = ((data & mask) != 0);
	}
}

void SLOT_SUB::event_callback(int event_id, int err)
{
	if(event_id == EVENT_CLOCK) {
		clock = !clock;
	}
}

// slot #1, #3

void SLOT_CART::initialize()
{
	memset(rom, 0xff, sizeof(rom));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	SET_BANK(0x0000, 0xffff, wdmy, rom);
	
	inserted = false;
}

void SLOT_CART::write_data8(uint32 addr, uint32 data)
{
	wbank[addr >> 13][addr & 0x1fff] = data;
}

uint32 SLOT_CART::read_data8(uint32 addr)
{
	return rbank[addr >> 13][addr & 0x1fff];
}

void SLOT_CART::open_cart(_TCHAR *file_path)
{
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		memset(rom, 0xff, sizeof(rom));
		
		fio->Fseek(0, FILEIO_SEEK_END);
		int file_size = fio->Ftell();
		fio->Fseek(0, FILEIO_SEEK_SET);
		
		if(file_size <= 0x2000) {
			// 8KB: 00000000
			fio->Fread(rom, 0x2000, 1);
			memcpy(rom + 0x2000, rom, 0x2000);
			memcpy(rom + 0x4000, rom, 0x4000);
			memcpy(rom + 0x8000, rom, 0x8000);
		} else if(file_size <= 0x4000) {
			// 16KB: 01010101
			fio->Fread(rom, 0x4000, 1);
			memcpy(rom + 0x4000, rom, 0x4000);
			memcpy(rom + 0x8000, rom, 0x8000);
		} else if(file_size <= 0x8000) {
			// 32KB: 01012323
			fio->Fread(rom + 0x4000, 0x8000, 1);
			memcpy(rom + 0x0000, rom + 0x4000, 0x4000);
			memcpy(rom + 0xc000, rom + 0x8000, 0x4000);
		} else {
			// 64KB: 01234567
			fio->Fread(rom, 0x10000, 1);
		}
		fio->Fclose();
		inserted = true;
	}
	delete fio;
}

void SLOT_CART::close_cart()
{
	memset(rom, 0xff, sizeof(rom));
	inserted = false;
}

// memory bus

void MEMORY::reset()
{
	for(int i = 0; i < 4; i++) {
		d_map[i] = d_slot[i];
	}
}

void MEMORY::write_data8(uint32 addr, uint32 data)
{
	addr &= 0xffff;
	d_map[addr >> 14]->write_data8(addr, data);
}

uint32 MEMORY::read_data8(uint32 addr)
{
	addr &= 0xffff;
	return d_map[addr >> 14]->read_data8(addr);
}

uint32 MEMORY::fetch_op(uint32 addr, int* wait)
{
	*wait = 1;
	return read_data8(addr);
}

void MEMORY::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_MEMORY_SEL) {
		for(int i = 0; i < 4; i++) {
			d_map[i] = d_slot[data & 3];
			data >>= 2;
		}
	}
}
