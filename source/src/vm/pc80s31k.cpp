/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2011.12.18-

	[ PC-80S31K ]
*/

#include "pc80s31k.h"
#include "disk.h"
#include "upd765a.h"
#include "../fileio.h"

//#define _DEBUG_PC80S31K

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

void PC80S31K::initialize()
{
	// init memory
	memset(rom, 0xff, sizeof(rom));
	memset(ram, 0, sizeof(ram));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load rom image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("PC88.ROM")), FILEIO_READ_BINARY)) {
		fio->Fseek(0x14000, FILEIO_SEEK_CUR);
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	} else if(fio->Fopen(emu->bios_path(_T("DISK.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	} else {
		// stop cpu
		d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
	}
	delete fio;
	
	// init memory map
	SET_BANK(0x0000, 0x1fff, wdmy, rom);
	SET_BANK(0x2000, 0x3fff, wdmy, rdmy);
	SET_BANK(0x4000, 0x7fff, ram, ram);
	SET_BANK(0x8000, 0xffff, wdmy, rdmy);
}

void PC80S31K::reset()
{
	// FIXME: ready will be pulluped but this causes PC-8801 keeps trying reading empty drive
//	d_fdc->write_signal(SIG_UPD765A_FREADY, 1, 1);	// ???
	d_fdc->set_drive_type(0, DRIVE_TYPE_2D);
	d_fdc->set_drive_type(1, DRIVE_TYPE_2D);
	
	// clear output
	d_pio->write_io8(1, 0);
	d_pio->write_io8(2, 0);
}

uint32 PC80S31K::read_data8(uint32 addr)
{
	addr &= 0xffff;
	return rbank[addr >> 13][addr & 0x1fff];
}

uint32 PC80S31K::fetch_op(uint32 addr, int *wait)
{
	addr &= 0xffff;
	*wait = (addr < 0x2000) ? 1 : 0;
	return rbank[addr >> 13][addr & 0x1fff];
}

void PC80S31K::write_data8(uint32 addr, uint32 data)
{
	addr &= 0xffff;
	if(addr == 0x7f15 && data == 0x1f && d_cpu->get_pc() < 0x2000) {
		// ugly patch to enable both #1 and #2 drives
		data = 0x3f;
	}
	wbank[addr >> 13][addr & 0x1fff] = data;
}

uint32 PC80S31K::read_io8(uint32 addr)
{
	uint32 val;
	
	switch(addr & 0xff) {
	case 0xf8:
		d_fdc->write_signal(SIG_UPD765A_TC, 1, 1);
		break;
	case 0xfa:
	case 0xfb:
		return d_fdc->read_io8(addr & 1);
	case 0xfc:
	case 0xfd:
		val = d_pio->read_io8(addr & 3);
#ifdef _DEBUG_PC80S31K
		emu->out_debug_log("SUB\tIN RECV(%d)=%2x\n", addr & 3, val);
#endif
		return val;
	case 0xfe:
		val = d_pio->read_io8(addr & 3);
#ifdef _DEBUG_PC80S31K
		{
			static uint32 prev = -1;
			if(prev != val){
//				emu->out_debug_log("SUB\tIN DAV=%d,RFD=%d,DAC=%d,ATN=%d\n", val&1, (val>>1)&1, (val>>2)&1, (val>>3)&1);
				prev = val;
			}
		}
#endif
		return val;
	}
	return 0xff;
}

void PC80S31K::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff) {
	case 0xf4:
		for(int drv = 0; drv < 2; drv++) {
			uint32 mode = data >> drv;
			if(mode & 1) {
				d_fdc->set_drive_type(drv, DRIVE_TYPE_2HD);
			} else if(mode & 4) {
				d_fdc->set_drive_type(drv, DRIVE_TYPE_2DD);
			} else {
				d_fdc->set_drive_type(drv, DRIVE_TYPE_2D);
			}
		}
		break;
	case 0xf7:
		// external printer port
		break;
	case 0xf8:
		// TODO: we need to update uPD765A to let the motor of each drive on/off
		break;
	case 0xfb:
		d_fdc->write_io8(addr & 1, data);
		break;
	case 0xfc:
	case 0xfd:
#ifdef _DEBUG_PC80S31K
		emu->out_debug_log("SUB\tOUT SEND(%d)=%2x\n", addr & 3, data);
#endif
		d_pio->write_io8(addr & 3, data);
		break;
	case 0xfe:
//		emu->out_debug_log("SUB\tOUT DAV=%d,RFD=%d,DAC=%d,ATN=%d\n", (data>>4)&1, (data>>5)&1, (data>>6)&1, (data>>7)&1);
		d_pio->write_io8(addr & 3, data);
		break;
	case 0xff:
		if(!(data & 0x80)) {
			int bit = (data >> 1) & 7;
			if(bit == 4){
//				emu->out_debug_log("SUB\tOUT DAV=%d\n", data & 1);
			} else if(bit == 5){
//				emu->out_debug_log("SUB\tOUT RFD=%d\n", data & 1);
			} else if(bit == 6){
//				emu->out_debug_log("SUB\tOUT DAC=%d\n", data & 1);
			} else if(bit == 7){
//				emu->out_debug_log("SUB\tOUT ATN=%d\n", data & 1);
			}
		}
		d_pio->write_io8(addr & 3, data);
		break;
	}
}

uint32 PC80S31K::intr_ack()
{
	return 0;	// NOP
}

#define STATE_VERSION	1

void PC80S31K::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(ram, sizeof(ram), 1);
}

bool PC80S31K::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(ram, sizeof(ram), 1);
	return true;
}

