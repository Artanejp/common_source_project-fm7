/*
	SANYO PHC-20 Emulator 'ePHC-20'

	Author : Takeda.Toshiya
	Date   : 2010.09.03-

	[ memory ]
*/

#include "memory.h"
#include "../datarec.h"
#include "../../fileio.h"

static const uint8 key_map[9][8] = {
	{0x31, 0x57, 0x53, 0x58, 0x00, 0x28, 0xba, 0xbd},	//	1	W	S	X		DOWN	:	-
	{0x1b, 0x51, 0x41, 0x5a, 0x00, 0x0d, 0xbb, 0xbf},	//	ESC???	Q	A	Z		RET	;	/
	{0x33, 0x52, 0x46, 0x56, 0x00, 0x27, 0xdb, 0x00},	//	3	R	F	V		RIGHT	[	
	{0x32, 0x45, 0x44, 0x43, 0x00, 0x26, 0xdd, 0x20},	//	2	E	D	C		UP	]	SPACE
	{0x35, 0x59, 0x48, 0x4e, 0x00, 0x30, 0x50, 0x00},	//	5	Y	H	N		0	P	
	{0x34, 0x54, 0x47, 0x42, 0x00, 0x25, 0xc0, 0x00},	//	4	T	G	B		LEFT	@	
	{0x36, 0x55, 0x4a, 0x4d, 0x00, 0x39, 0x4f, 0x00},	//	6	U	J	M		9	O	
	{0x37, 0x49, 0x4b, 0xbc, 0x00, 0x38, 0x4c, 0xbe},	//	7	I	K	,		8	L	.
	{0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00},	//	???		SHIFT					
};

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 10, eb = (e) >> 10; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x400 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x400 * (i - sb); \
		} \
	} \
}

void MEMORY::initialize()
{
	memset(rom, 0xff, sizeof(rom));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load rom image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("BASIC.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
	delete fio;
	
	// set memory map
	SET_BANK(0x0000, 0x1fff, wdmy, rom );
	SET_BANK(0x2000, 0x2fff, ram,  ram );
	SET_BANK(0x3000, 0x3fff, wdmy, rdmy);
	SET_BANK(0x4000, 0x43ff, vram, vram);
	SET_BANK(0x4400, 0xffff, wdmy, rdmy);
	
	key_stat = emu->key_buffer();
	
	// register event to update the key status
	register_frame_event(this);
}

void MEMORY::reset()
{
	memset(ram, 0, sizeof(ram));
	memset(vram, 0, sizeof(vram));
	
	memset(status, 0, sizeof(status));
	sysport = 0;
}

void MEMORY::write_data8(uint32 addr, uint32 data)
{
	addr &= 0xffff;
	if((0x3000 <= addr && addr < 0x4000) || 0x4400 <= addr) {
		// memory mapped i/o
		switch(addr) {
		case 0x6000:
			if(data == 0xf9) {
				// datarec out h
				d_drec->write_signal(SIG_DATAREC_OUT, 1, 1);
			} else if(data == 0x0a) {
				// datarec out l
				d_drec->write_signal(SIG_DATAREC_OUT, 0, 1);
			} else {
				// unknown ???
#ifdef _IO_DEBUG_LOG
				emu->out_debug_log("%6x\tWM8\t%4x,%2x\n", get_cpu_pc(0), addr, data);
#endif
			}
			break;
		default:
#ifdef _IO_DEBUG_LOG
			emu->out_debug_log("%6x\tWM8\t%4x,%2x\n", get_cpu_pc(0), addr, data);
#endif
			break;
		}
		return;
	}
	wbank[addr >> 10][addr & 0x3ff] = data;
}

uint32 MEMORY::read_data8(uint32 addr)
{
	addr &= 0xffff;
	if((0x3000 <= addr && addr < 0x4000) || 0x4400 <= addr) {
		// memory mapped i/o
		switch(addr) {
		case 0x3800:
		case 0x3801:
		case 0x3802:
		case 0x3803:
		case 0x3804:
		case 0x3805:
		case 0x3806:
		case 0x3807:
		case 0x3808:
			// note: bit0 of 3808h ... break cload ???
			return ~status[addr & 0x0f];
		case 0x6000:
			// bit0: datarec in
			// bit1: vsync or hsync ???
			return sysport;
		}
#ifdef _IO_DEBUG_LOG
		emu->out_debug_log("%6x\tRM8\t%4x\n", get_cpu_pc(0), addr);
#endif
		return 0xff;
	}
	return rbank[addr >> 10][addr & 0x3ff];
}

void MEMORY::event_frame()
{
	memset(status, 0, sizeof(status));
	
	for(int i = 0; i < 9; i++) {
		uint8 val = 0;
		for(int j = 0; j < 8; j++) {
			val |= key_stat[key_map[i][j]] ? (1 << j) : 0;
		}
		status[i] = val;
	}
}

void MEMORY::write_signal(int id, uint32 data, uint32 mask)
{
	if(data & mask) {
		sysport |= mask;
	} else {
		sysport &= ~mask;
	}
}

#define STATE_VERSION	1

void MEMORY::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(ram, sizeof(ram), 1);
	state_fio->Fwrite(vram, sizeof(vram), 1);
	state_fio->Fwrite(status, sizeof(status), 1);
	state_fio->FputUint8(sysport);
}

bool MEMORY::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(ram, sizeof(ram), 1);
	state_fio->Fread(vram, sizeof(vram), 1);
	state_fio->Fread(status, sizeof(status), 1);
	sysport = state_fio->FgetUint8();
	return true;
}

