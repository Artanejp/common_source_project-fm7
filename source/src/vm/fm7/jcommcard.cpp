/*
 * Emulation of Fujitsu Japanese Communication Card.
 * (C) 2018 K.Ohta.
 * Note:
 * Based on XM7 L70 , with permittion from Ryu Takegami. 
 */

#include "vm.h"
#include "../../fileio.h"
#include "emu.h"

#include "fm7_common.h"

#include "../mc6809.h"
#include "./jcommcard.h"

FM7_JCOMMCARD::FM7_JCOMMCARD(VM *parent_vm, EMU *parent_emu) : DEVICE(parent_vm, parent_emu)
{
	n_bank = 0;
	rcb_address = 0;
	cpu_ba = cpu_bs = false;
	halted = false;
	kanji_address = 0x00000;
	jis78_emulation = false;
		
	memset(prog_rom, 0xff, sizeof(prog_rom));
	memset(dict_rom, 0xff, sizeof(dict_rom));
	memset(kanji_rom, 0xff, sizeof(kanji_rom));
	memset(backup_ram, 0x00, sizeof(backup_ram));
	cpu = NULL;
	modified = true;
	firmware_ok = false;
}

FM7_JCOMMCARD::~FM7_JCOMMCARD()
{
}

void FM7_JCOMMCARD::initialize(void)
{
	FILEIO *fio = new FILEIO();
		
	if(fio->Fopen(create_local_path(_T("JSUBSYS.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(prog_rom, sizeof(prog_rom), 1);
		fio->Fclose();
		firmware_ok = true;
	}

	/* Patch from XM7/VM/jsubsys.c */
	if(prog_rom[0x000d] == 0x8f) {
		prog_rom[0x000d] = 0x88;
	}
	/* Change: DICT.ROM to JSUBDICT.ROM */
	if(fio->Fopen(create_local_path(_T("JSUBDICT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(dict_rom, sizeof(dict_rom), 1);
		fio->Fclose();
	}
	/* KANJI ROM */
	if(fio->Fopen(create_local_path(_T("JSUBKANJI.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(kanji_rom, sizeof(kanji_rom), 1);
		fio->Fclose();
	} else if(fio->Fopen(create_local_path(_T("KANJI.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(kanji_rom, sizeof(kanji_rom), 1);
		fio->Fclose();
	} else if(fio->Fopen(create_local_path(_T("KANJI1.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(kanji_rom, sizeof(kanji_rom), 1);
		fio->Fclose();
	}

	if(fio->Fopen(create_local_path(_T("JCOMMCARD.bin")), FILEIO_READ_BINARY)) {
		fio->Fread(backup_ram, sizeof(backup_ram), 1);
		fio->Fclose();
		modified = false;
	}
	delete fio;

#if defined(_FM77AV_VARIANTS)
	jis78_emulation = true;
#else
	jis78_emulation = ((config.dipswitch & FM7_DIPSW_JIS78EMU_ON) != 0);
#endif
}

void FM7_JCOMMCARD::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case FM7_JCOMMCARD_BUS_BA:
		cpu_ba = ((data & mask) != 0);
		halted = cpu_ba & cpu_bs;
		break;
	case FM7_JCOMMCARD_BUS_BS:
		cpu_bs = ((data & mask) != 0);
		halted = cpu_ba & cpu_bs;
		break;
	}
}

uint32_t FM7_JCOMMCARD::read_io8(uint32_t address)
{
	uint32_t data = 0xff;
	switch(address & 3) {
	case 0:
		/* HALT STATUS */
		if(halted) {
			data &= 0x7f;
		}
		break;
	case 1:
		/* RCB DATA (Auto increment address) */
		if(halted) {
			data = backup_ram[0x1f00 | rcb_address];
			rcb_address++;
		}
		break;
	case 2:
	case 3:
		/* Kanji Data */
		if((jis78_emulation) && (kanji_address >= 0x3000) && (kanji_address < 0x4000)) {
			/* JIS78 */
			data =  address & 1;
		} else {
			data = kanji_rom[(kanji_address << 1) + (address & 1)];
		}
		break;
	}
	return data;
}

void FM7_JCOMMCARD::write_io8(uint32_t address, uint32_t data)
{
	switch(address & 3) {
	case 0:
		/* Kanji Address High */
		kanji_address = (kanji_address & 0x0000ff) | ((data & 0x000000ff) << 8);
		break;
	case 1:
		/* Kanji Address Low */
		kanji_address = (kanji_address & 0x00ff00) | (data & 0x000000ff);
		break;
	case 2:
		/* REQUEST TO HALT */
		if((data & 0x80) != 0) {
			if(cpu != NULL) cpu->write_signal(SIG_CPU_HALTREQ, 0x00000000, 0xffffffff);
		} else {
			if(cpu != NULL) cpu->write_signal(SIG_CPU_HALTREQ, 0xffffffff, 0xffffffff);
			rcb_address = 0;
		}
		break;
	case 3:
		/* RCB DATA (Auto increment address) */
		if(halted) {
			backup_ram[0x1f00 | rcb_address] = (uint8_t)data;
			rcb_address++;
		}
		break;
	}
}

uint32_t FM7_JCOMMCARD::read_data8(uint32_t address)
{
	/*
	 * $8000-$9FFE : SRAM
	 * $9FFF       : SYNC/BANK REG
	 * $A000-$AFFF : DICT
	 * $C000-$FFFF : SUB SYSTEM
	 */
	if(address < 0x8000) return 0xff; /* NOOP */
	if(address <= 0x9ffe) { /* SRAM */
		return (uint32_t)backup_ram[address & 0x1fff];
	} else if(address == 0x9fff) { /* RCB BANK REGISTER */
		return (uint32_t)n_bank;
	} else if(address < 0xb000) { /* DICT ROM */
		return (uint32_t)(dict_rom[(address & 0x0fff) | (((uint32_t)n_bank) << 12)]);
	} else if(address < 0xc000) {
		return 0xff;
	} else if(address < 0x10000) {
		return (uint32_t)prog_rom[address & 0x3fff];
	}
	return 0xff;
}

void FM7_JCOMMCARD::write_data8(uint32_t address, uint32_t data)
{
	if(address < 0x8000) return;
	if(address >= 0xa000) return;
	if(address == 0x9fff) {
		if(cpu != NULL) cpu->write_signal(SIG_CPU_HALTREQ, ((data & 0x80) != 0) ? 0 : 0xffffffff, 0xffffffff);
		n_bank = (uint8_t)(data & 0x3f); 
	} else if(address < 0x9fff) {
		modified = true;
		backup_ram[address & 0x1fff] = (uint8_t)data;
	}
}

void FM7_JCOMMCARD::release(void)
{
	FILEIO *fio = new FILEIO();
	if(modified) {
		if(fio->Fopen(create_local_path(_T("JCOMMCARD.bin")), FILEIO_WRITE_BINARY)) {
			fio->Fwrite(backup_ram, sizeof(backup_ram), 1);
			fio->Fclose();
			modified = false;
		}
	}
}
	
void FM7_JCOMMCARD::reset(void)
{
	rcb_address = 0x00;
	kanji_address = 0x00000;
	if(cpu != NULL) cpu->write_signal(SIG_CPU_HALTREQ, 0, 0xffffffff);
	cpu_ba = cpu_bs = false;
	halted = false;
#if defined(_FM77AV_VARIANTS)
	jis78_emulation = false;
#else
	jis78_emulation = ((config.dipswitch & FM7_DIPSW_JIS78EMU_ON) != 0);
#endif
}

#define STATE_VERSION 1

void FM7_JCOMMCARD::save_state(FILEIO *state_fio)
{
	state_fio->FputUint32_BE(STATE_VERSION);
	state_fio->FputInt32_BE(this_device_id);
	this->out_debug_log(_T("Save State: JCOMM CARD: id=%d ver=%d\n"), this_device_id, STATE_VERSION);

	state_fio->FputUint8(n_bank & 0x3f);
	state_fio->FputUint8(rcb_address);
	state_fio->FputUint32_BE(kanji_address);

	state_fio->FputBool(cpu_ba);
	state_fio->FputBool(cpu_bs);

	state_fio->Fwrite(prog_rom, sizeof(prog_rom), 1);
	state_fio->Fwrite(dict_rom, sizeof(dict_rom), 1);
	state_fio->Fwrite(kanji_rom, sizeof(kanji_rom), 1);
	state_fio->Fwrite(backup_ram, sizeof(backup_ram), 1);
	state_fio->FputBool(firmware_ok);

#if !defined(_FM77AV_VARIANTS)
	state_fio->FputBool(jis78_emulation);
#endif
}

bool FM7_JCOMMCARD::load_state(FILEIO *state_fio)
{
	uint32_t version;
	version = state_fio->FgetUint32_BE();
	if(this_device_id != state_fio->FgetInt32_BE()) return false;
	this->out_debug_log(_T("Load State: JCOMM CARD: id=%d ver=%d\n"), this_device_id, STATE_VERSION);

	if(version >= 1) {
		n_bank = state_fio->FgetUint8() & 0x3f;
		rcb_address = state_fio->FgetUint8();
		kanji_address = state_fio->FgetUint32_BE();

		cpu_ba = state_fio->FgetBool();
		cpu_bs = state_fio->FgetBool();
		halted = cpu_ba & cpu_bs;

		state_fio->Fread(prog_rom, sizeof(prog_rom), 1);
		state_fio->Fread(dict_rom, sizeof(dict_rom), 1);
		state_fio->Fread(kanji_rom, sizeof(kanji_rom), 1);
		state_fio->Fread(backup_ram, sizeof(backup_ram), 1);
		firmware_ok = state_fio->FgetBool();
		modified = true;

#if !defined(_FM77AV_VARIANTS)
		jis78_emulation = state_fio->FgetBool();
#else
		jis78_emulation = false;
#endif
	}
	return true;
}
