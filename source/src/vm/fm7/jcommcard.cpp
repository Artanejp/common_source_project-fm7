/*
 * Emulation of Fujitsu Japanese Communication Card.
 * (C) 2018 K.Ohta.
 * Note:
 * Based on XM7 L70 , with permittion from Ryu Takegami. 
 */

#include "../vm.h"
#include "../../emu.h"
#include "../../fileio.h"

#include "fm7_common.h"

#include "../mc6809.h"
#include "./jcommcard.h"

FM7_JCOMMCARD::FM7_JCOMMCARD(VM_TEMPLATE* parent_vm, EMU *parent_emu) : DEVICE(parent_vm, parent_emu)
{
	n_bank = 0;
	rcb_address = 0;
	halted = false;
	kanji_address.d = 0x00000000;
		
	memset(prog_rom, 0xff, sizeof(prog_rom));
	memset(dict_rom, 0xff, sizeof(dict_rom));
	memset(p_ram, 0x00, sizeof(p_ram));
	cpu = NULL;
	//modified = true;
	firmware_ok = false;
	diag_dictrom_load = false;
}

FM7_JCOMMCARD::~FM7_JCOMMCARD()
{
}

void FM7_JCOMMCARD::initialize(void)
{
	FILEIO *fio = new FILEIO();
	bool b_stat = false;
	bool b_stat_dicrom = false;
	size_t nsize;
	if(fio->Fopen(create_local_path(_T(ROM_JCOMM_FIRMWARE)), FILEIO_READ_BINARY)) { // 20180114
		fio->Fread(prog_rom, sizeof(prog_rom), 1);
		fio->Fclose();
		firmware_ok = true;
	}

	/* Patch from XM7/VM/jsubsys.c */
	if(prog_rom[0x000d] == 0x8f) {
		prog_rom[0x000d] = 0x88;
	}
	/* Change: DICT.ROM to JSUBDICT.ROM */
	if(fio->Fopen(create_local_path(_T(ROM_JCOMM_DICTIONARY)), FILEIO_READ_BINARY)) {
		nsize = fio->Fread(dict_rom, 1, sizeof(dict_rom));
		fio->Fclose();
		if(nsize >= sizeof(dict_rom)) {
			diag_dictrom_load = true;
			this->out_debug_log(_T("FULL SET OF DICTIONARY ROM WITH KANJI LOADED."));
		} else {
			this->out_debug_log(_T("PARTLY SET OF DICTIONARY ROM LOADED."));
		}			
	}
	/* KANJI ROM */
	
	if(!diag_dictrom_load) {
		if(fio->Fopen(create_local_path(_T(ROM_JCOMM_KANJI)), FILEIO_READ_BINARY)) {
			fio->Fread(&(dict_rom[0x40000]), 0x20000, 1);
			fio->Fclose();
			b_stat = true;
		} else if(fio->Fopen(create_local_path(_T(ROM_KANJI_CLASS1)), FILEIO_READ_BINARY)) {
			fio->Fread(&(dict_rom[0x40000]), 0x20000, 1);
			fio->Fclose();
			b_stat = true;
		} else if(fio->Fopen(create_local_path(_T(ROM_KANJI_CLASS1_FALLBACK)), FILEIO_READ_BINARY)) {
			fio->Fread(&(dict_rom[0x40000]), 0x20000, 1);
			fio->Fclose();
			b_stat = true;
		}
		this->out_debug_log(_T("KANJIROM READ %s."), b_stat ? "OK" : "FAILED");
	}
	delete fio;
}


void FM7_JCOMMCARD::write_signal(int id, uint32_t data, uint32_t mask)
{
	bool b = ((data & mask) != 0);
	switch(id) {
	case FM7_JCOMMCARD_BUS_HALT:
		halted = b;
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
			data = p_ram[0x1f00 | rcb_address];
			rcb_address++;
		}
		break;
	case 2:
	case 3:
		/* Kanji Data */
		data = dict_rom[0x40000 + (kanji_address.d << 1) + (address & 1)];
		break;
	}
	return data;
}

void FM7_JCOMMCARD::write_io8(uint32_t address, uint32_t data)
{
	switch(address & 3) {
	case 0:
		/* Kanji Address High */
		kanji_address.b.h = (uint8_t)(data & 0xff);
		break;
	case 1:
		/* Kanji Address Low */
		kanji_address.b.l = (uint8_t)(data & 0xff);
		break;
	case 2:
		/* REQUEST TO HALT */
		if((data & 0x80) != 0) {
			if(cpu != NULL) cpu->write_signal(SIG_CPU_HALTREQ, 0x00000000, 0xffffffff);
			//haltreq = false;
			//halted = false;
		} else {
			if(cpu != NULL) cpu->write_signal(SIG_CPU_HALTREQ, 0xffffffff, 0xffffffff);
			//haltreq = true;
			//halted = true;
			rcb_address = 0;
		}
		break;
	case 3:
		/* RCB DATA (Auto increment address) */
		if(halted) {
			p_ram[0x1f00 | rcb_address] = (uint8_t)data;
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
		return (uint32_t)p_ram[address & 0x1fff];
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
		if(cpu != NULL) cpu->write_signal(SIG_CPU_HALTREQ, ((data & 0x80) == 0) ? 0xffffffff : 0, 0xffffffff);
		//halted = ((data & 0x80) == 0);
		n_bank = (uint8_t)(data & 0x3f); 
	} else if(address < 0x9fff) {
		//modified = true;
		p_ram[address & 0x1fff] = (uint8_t)data;
	}
}

void FM7_JCOMMCARD::release(void)
{
#if 0
	FILEIO *fio = new FILEIO();
	if(modified) {
		if(fio->Fopen(create_local_path(_T(RAM_JCOMM_BACKUP)), FILEIO_WRITE_BINARY)) {
			fio->Fwrite(p_ram, sizeof(p_ram), 1);
			fio->Fclose();
			modified = false;
		}
	}
	delete fio;
#endif
}
	
void FM7_JCOMMCARD::reset(void)
{
	rcb_address = 0x00;
	kanji_address.d = 0x00000000;
	if(cpu != NULL) {
		cpu->write_signal(SIG_CPU_HALTREQ, 0, 0xffffffff);
		cpu->reset();
	}
}

#define STATE_VERSION 4

bool FM7_JCOMMCARD::decl_state(FILEIO *state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
 
	state_fio->StateValue(n_bank);
	state_fio->StateValue(rcb_address);
	state_fio->StateValue(kanji_address.d);
	state_fio->StateValue(halted);

	state_fio->StateArray(prog_rom, sizeof(prog_rom), 1);
	state_fio->StateArray(dict_rom, sizeof(dict_rom), 1);
	state_fio->StateArray(p_ram, sizeof(p_ram), 1);
	state_fio->StateValue(firmware_ok);

	return true;
}

void FM7_JCOMMCARD::save_state(FILEIO *state_fio)
{
	//state_fio->FputUint32_BE(STATE_VERSION);
	//state_fio->FputInt32_BE(this_device_id);
	this->out_debug_log(_T("Save State: JCOMM CARD: id=%d ver=%d\n"), this_device_id, STATE_VERSION);

	decl_state(state_fio, false);
	//state_fio->FputUint8(n_bank & 0x3f);
	//state_fio->FputUint8(rcb_address);
	//state_fio->FputUint32_BE(kanji_address.d);

	//state_fio->FputBool(halted);

	//state_fio->Fwrite(prog_rom, sizeof(prog_rom), 1);
	//state_fio->Fwrite(dict_rom, sizeof(dict_rom), 1);
	//state_fio->Fwrite(p_ram, sizeof(p_ram), 1);
	//state_fio->FputBool(firmware_ok);

}

bool FM7_JCOMMCARD::load_state(FILEIO *state_fio)
{
	//uint32_t version;
	//version = state_fio->FgetUint32_BE();
	//if(this_device_id != state_fio->FgetInt32_BE()) return false;
	this->out_debug_log(_T("Load State: JCOMM CARD: id=%d ver=%d\n"), this_device_id, STATE_VERSION);
	if(!decl_state(state_fio, true)) {
		return false;
	}
	n_bank &= 0x3f;

	//if(version >= 1) {
	//	n_bank = state_fio->FgetUint8() & 0x3f;
	//	rcb_address = state_fio->FgetUint8();
	//	kanji_address.d = state_fio->FgetUint32_BE();
	//	halted = state_fio->FgetBool();
	//	state_fio->Fread(prog_rom, sizeof(prog_rom), 1);
	//	state_fio->Fread(dict_rom, sizeof(dict_rom), 1);
	//	state_fio->Fread(p_ram, sizeof(p_ram), 1);
	//	firmware_ok = state_fio->FgetBool();
	//modified = true; // Abondoned
	//}
	return true;
}
