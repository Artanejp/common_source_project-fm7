/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.01.30-

	[ EPSON TF-20 ]
*/

#include "tf20.h"
#include "upd765a.h"

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 11, eb = (e) >> 11; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x800 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x800 * (i - sb); \
		} \
	} \
}

void TF20::initialize()
{
	// init memory
	memset(rom, 0xff, sizeof(rom));
	memset(ram, 0, sizeof(ram));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load rom image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("TF20.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	} else if(fio->Fopen(create_local_path(_T("DISK.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	} else {
		// stop cpu
		d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
	}
	delete fio;
	
	// init memory map
	SET_BANK(0x0000, 0xffff, ram, ram);
}

void TF20::reset()
{
	SET_BANK(0x0000, 0x07ff, ram, rom);
	rom_selected = true;
}

uint32_t TF20::read_data8(uint32_t addr)
{
	addr &= 0xffff;
	return rbank[addr >> 11][addr & 0x7ff];
}

void TF20::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	wbank[addr >> 11][addr & 0x7ff] = data;
}

/*
xxF0H-F3H	R/W	uPD7201 (A=A0 B=A1)
xxF6H		R	Shadow ROM Separation
xxF7H		R	Dip Switch (D0-D3)
xxF8H		R/W	uPD765A TC
xxF8H		W	FDDMotor ON/OFF (DO1=ON)
xxFAH		R	uPD765A Status Register
xxFBH		R/W	uPD765A Data Register

Dip Switch
6-1	DriveA:	OFF.ON.OFF.OFF.OFF.ON
	DriveB:	OFF,ON,OFF,OFF,ON,OFF
*/

uint32_t TF20::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0xf0:
	case 0xf1:
	case 0xf2:
	case 0xf3:
		return d_sio->read_io8(addr);
	case 0xf6:
		SET_BANK(0x0000, 0x07ff, ram, ram);
		rom_selected = false;
		break;
	case 0xf7:
		return 0xf0 | (drive_no & 0x0f);
	case 0xf8:
		d_fdc->write_signal(SIG_UPD765A_TC, 1, 1);
		break;
	case 0xfa:
	case 0xfb:
		return d_fdc->read_io8(addr);
	case 0xfc:
	case 0xfd:
	case 0xfe:
		return d_pio->read_io8(addr);
	}
	return 0xff;
}

void TF20::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xf0:
	case 0xf1:
	case 0xf2:
	case 0xf3:
		d_sio->write_io8(addr, data);
		break;
	case 0xf8:
		d_fdc->write_signal(SIG_UPD765A_TC, 1, 1);
		break;
	case 0xfb:
		d_fdc->write_io8(addr, data);
		break;
	case 0xfc:
	case 0xfd:
	case 0xfe:
	case 0xff:
		d_pio->write_io8(addr, data);
		break;
	}
}

uint32_t TF20::get_intr_ack()
{
	return 0;	// NOP
}

#define STATE_VERSION	1

bool TF20::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(ram, sizeof(ram), 1);
	state_fio->StateValue(rom_selected);
	
	// post process
	if(loading) {
		if(rom_selected) {
			SET_BANK(0x0000, 0x07ff, ram, rom);
		} else {
			SET_BANK(0x0000, 0x07ff, ram, ram);
		}
	}
	return true;
}

