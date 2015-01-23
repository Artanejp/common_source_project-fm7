/*
	EPSON QC-10 Emulator 'eQC-10'

	Author : Takeda.Toshiya
	Date   : 2008.02.15 -

	[ memory ]
*/

#include "memory.h"
#include "../i8253.h"
#include "../pcm1bit.h"
#include "../upd765a.h"
#include "../../fileio.h"

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 11, eb = (e) >> 11; \
	for(int i = sb; i <= eb; i++) { \
		wbank[i] = (w) + 0x800 * (i - sb); \
		rbank[i] = (r) + 0x800 * (i - sb); \
	} \
}

void MEMORY::initialize()
{
	// init memory
	memset(ram, 0, sizeof(ram));
	memset(cmos, 0, sizeof(cmos));
	memset(ipl, 0xff, sizeof(ipl));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load rom images
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("IPL.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ipl, sizeof(ipl), 1);
		fio->Fclose();
	}
	if(fio->Fopen(emu->bios_path(_T("CMOS.BIN")), FILEIO_READ_BINARY)) {
		fio->Fread(cmos, sizeof(cmos), 1);
		fio->Fclose();
	}
	delete fio;
	
	cmos_crc32 = getcrc32(cmos, sizeof(cmos));
}

void MEMORY::release()
{
	if(cmos_crc32 != getcrc32(cmos, sizeof(cmos))) {
		FILEIO* fio = new FILEIO();
		if(fio->Fopen(emu->bios_path(_T("CMOS.BIN")), FILEIO_WRITE_BINARY)) {
			fio->Fwrite(cmos, sizeof(cmos), 1);
			fio->Fclose();
		}
		delete fio;
	}
}

void MEMORY::reset()
{
	// init memory map
	bank = 0x10;
	psel = csel = 0;
	update_map();
	
	// init pcm
	pcm_on = pcm_cont = pcm_pit = false;
	
	// init fdc/fdd status
	fdc_irq = motor = false;
}

void MEMORY::write_data8(uint32 addr, uint32 data)
{
	addr &= 0xffff;
	wbank[addr >> 11][addr & 0x7ff] = data;
}

uint32 MEMORY::read_data8(uint32 addr)
{
	addr &= 0xffff;
	return rbank[addr >> 11][addr & 0x7ff];
}

void MEMORY::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff) {
	case 0x18: case 0x19: case 0x1a: case 0x1b:
		bank = data;
		// timer gate signal
		d_pit->write_signal(SIG_I8253_GATE_0, data, 1);
		d_pit->write_signal(SIG_I8253_GATE_2, data, 2);
		// pcm on
		pcm_cont = ((data & 4) != 0);
		update_pcm();
		break;
	case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		psel = data;
		break;
	case 0x20: case 0x21: case 0x22: case 0x23:
		csel = data;
		break;
	}
	update_map();
}

uint32 MEMORY::read_io8(uint32 addr)
{
	switch(addr & 0xff) {
	case 0x18: case 0x19: case 0x1a: case 0x1b:
		return ~config.dipswitch & 0xff;
	case 0x30: case 0x31: case 0x32: case 0x33:
		return (bank & 0xf0) | (d_fdc->disk_inserted() ? 8 : 0) | (motor ? 0 : 2) | (fdc_irq ? 1 : 0);
	}
	return 0xff;
}

/*
	0000-DFFF	: RAM * 4banks
	E000-FFFF	: RAM
	----
	0000-1FFF	: IPL
	8000-87FF	: CMOS
*/

void MEMORY::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_MEMORY_PCM) {
		// pcm on from pit
		pcm_pit = ((data & mask) != 0);
		update_pcm();
	} else if(id == SIG_MEMORY_FDC_IRQ) {
		fdc_irq = ((data & mask) != 0);
	} else if(id == SIG_MEMORY_MOTOR) {
		motor = ((data & mask) != 0);
	}
}

void MEMORY::update_map()
{
	if(!(psel & 1)) {
		SET_BANK(0x0000, 0x1fff, wdmy, ipl);
		SET_BANK(0x2000, 0xdfff, wdmy, rdmy);
	} else if(csel & 1) {
		if(bank & 0x10) {
			SET_BANK(0x0000, 0x7fff, ram + 0x00000, ram + 0x00000);
		} else if(bank & 0x20) {
			SET_BANK(0x0000, 0x7fff, ram + 0x10000, ram + 0x10000);
		} else if(bank & 0x40) {
			SET_BANK(0x0000, 0x7fff, ram + 0x20000, ram + 0x20000);
		} else if(bank & 0x80) {
			SET_BANK(0x0000, 0x7fff, ram + 0x30000, ram + 0x30000);
		} else {
			SET_BANK(0x0000, 0x7fff, wdmy, rdmy);
		}
		SET_BANK(0x8000, 0x87ff, cmos, cmos);
	} else {
		if(bank & 0x10) {
			SET_BANK(0x0000, 0xdfff, ram + 0x00000, ram + 0x00000);
		} else if(bank & 0x20) {
			SET_BANK(0x0000, 0xdfff, ram + 0x10000, ram + 0x10000);
		} else if(bank & 0x40) {
			SET_BANK(0x0000, 0xdfff, ram + 0x20000, ram + 0x20000);
		} else if(bank & 0x80) {
			SET_BANK(0x0000, 0xdfff, ram + 0x30000, ram + 0x30000);
		} else {
			SET_BANK(0x0000, 0xdfff, wdmy, rdmy);
		}
	}
	SET_BANK(0xe000, 0xffff, ram + 0xe000, ram + 0xe000);
}

void MEMORY::update_pcm()
{
	if(!pcm_on && (pcm_cont || pcm_pit)) {
		d_pcm->write_signal(SIG_PCM1BIT_ON, 1, 1);
		pcm_on = true;
	} else if(pcm_on && !(pcm_cont || pcm_pit)) {
		d_pcm->write_signal(SIG_PCM1BIT_ON, 0, 1);
		pcm_on = false;
	}
}

#define STATE_VERSION	1

void MEMORY::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(ram, sizeof(ram), 1);
	state_fio->Fwrite(cmos, sizeof(cmos), 1);
	state_fio->FputUint32(cmos_crc32);
	state_fio->FputUint8(bank);
	state_fio->FputUint8(psel);
	state_fio->FputUint8(csel);
	state_fio->FputBool(pcm_on);
	state_fio->FputBool(pcm_cont);
	state_fio->FputBool(pcm_pit);
	state_fio->FputBool(fdc_irq);
	state_fio->FputBool(motor);
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
	state_fio->Fread(cmos, sizeof(cmos), 1);
	cmos_crc32 = state_fio->FgetUint32();
	bank = state_fio->FgetUint8();
	psel = state_fio->FgetUint8();
	csel = state_fio->FgetUint8();
	pcm_on = state_fio->FgetBool();
	pcm_cont = state_fio->FgetBool();
	pcm_pit = state_fio->FgetBool();
	fdc_irq = state_fio->FgetBool();
	motor = state_fio->FgetBool();
	
	// post process
	update_map();
	return true;
}

